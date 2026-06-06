#include "RobotController.h"

RobotController::RobotController(MotorController& motor, UltrasonicSensor& sensor)
    : mMotor(motor), mSensor(sensor) {}

void RobotController::begin() {
    mMotor.begin();
    mSensor.begin();
    mState = RobotState::Initializing;
}

// =========================================================
// Immortal Non-Blocking Action Scheduler
// =========================================================

void RobotController::startAction(uint32_t durationMs) {
    mActionStartTime = millis();
    mActionDuration = durationMs;
}

bool RobotController::isActionBusy() const {
    // Rollover safe: (current - start) will always be correct even after 50 days
    return (millis() - mActionStartTime) < mActionDuration;
}

bool RobotController::isSystemBusy() const {
    return mMotor.isBusy() || isActionBusy();
}

// =========================================================
// Helpers
// =========================================================
float RobotController::getSafeDistance() {
    float d = mSensor.getDistance();
    
    // 1. NAN Immunity: If invalid, use the last known good distance
    if (isnan(d)) {
        return mLastValidDistance; 
    }
    
    // 2. Dropout Immunity: HC-SR04 physically cannot read < 2cm. 
    // If it reports 0 or < 2, it's a temporary glitch. Ignore it.
    if (d < 2.0f) {
        return mLastValidDistance;
    }
    
    // 3. Update memory for future dropouts
    mLastValidDistance = d;
    return d;
}

uint16_t RobotController::getScanAngle() {
    if (mSearchAttempt >= MAX_SEARCH_ATTEMPTS) {
        mSearchAttempt = MAX_SEARCH_ATTEMPTS - 1;
    }
    uint16_t angle = SEARCH_ANGLES[mSearchAttempt];
    if (angle > MAX_SCAN_ANGLE) angle = MAX_SCAN_ANGLE;
    return angle;
}

// =========================================================
// Main Update Loop
// =========================================================

void RobotController::update() {
    mSensor.update();
    mMotor.update();

    switch (mState) {
        case RobotState::Initializing:
            if (getSafeDistance() > 0) mState = RobotState::MovingForward;
            break;

        case RobotState::MovingForward: {
            float currentDistance = getSafeDistance();
            uint32_t currentTime = millis();

            // ---------------------------------------------------------
            // 1. Physics Engine: Closing Rate (Low-Pass Filtered)
            // ---------------------------------------------------------
            // CRITICAL: Only calculate when the sensor actually reports a NEW value!
            if (fabs(currentDistance - mPreviousDistance) > 0.5f) {
                float deltaTimeSec = (currentTime - mPreviousTime) / 1000.0f;
                
                // Must be at least 20ms to be a valid sensor update
                if (deltaTimeSec > 0.020f) { 
                    float rawRate = (mPreviousDistance - currentDistance) / deltaTimeSec;
                    
                    // Low-pass filter to smooth out ultrasonic noise
                    mClosingRate = (mClosingRate * 0.6f) + (rawRate * 0.4f);
                }
                
                mPreviousDistance = currentDistance;
                mPreviousTime = currentTime;
            }

            // ---------------------------------------------------------
            // 2. Non-Linear Speed Curve (Quadratic Easing)
            // ---------------------------------------------------------
            // Normalize distance: 40cm = 0.0 (Crawl), 160cm = 1.0 (Max Speed)
            float normalized = constrain((currentDistance - 40.0f) / 120.0f, 0.0f, 1.0f);
            
            // Square it. The robot stays incredibly slow near 40cm, 
            // but rapidly accelerates as the path opens up past 100cm.
            float curve = normalized * normalized;
            
            // What the robot WANTS to be doing based on distance
            uint8_t targetSpeedPWM = 40 + (curve * 120.0f);

            // ---------------------------------------------------------
            // 3. Dynamic Braking Zone (Based on ACTUAL speed)
            // ---------------------------------------------------------
            // 🛑 THE MAGIC FIX: Use mCurrentSpeedPWM (Actual physical speed) 
            // NOT targetSpeedPWM. Momentum depends on what the motors are ACTUALLY doing.
            float stoppingDistance = 20.0f + (mCurrentSpeedPWM * 0.35f);

            // Closing Rate Buffer: If approaching fast, extend the safety net!
            if (mClosingRate > 30.0f) {
                stoppingDistance += (mClosingRate * 0.25f); 
            }

            // ---------------------------------------------------------
            // 4. Decision Time: Brake or Drive?
            // ---------------------------------------------------------
            // PANIC STOP CONDITIONS:
            // 1. We have entered the dynamic stopping zone.
            // 2. Absolute emergency fallback (< 15cm).
            // 3. An object is approaching us faster than 150 cm/s.
            if (currentDistance <= stoppingDistance || currentDistance <= 15.0f || mClosingRate > 150.0f) {
                mMotor.activeBrake(); // STOP ON A DIME!
                
                mCurrentSpeedPWM = 0; // Reset actual speed to 0
                
                // Escalate search attempt if we got dangerously close
                if (currentDistance <= 15.0f) {
                    mSearchAttempt = 5; 
                } else {
                    mSearchAttempt = 0; 
                }
                
                mState = RobotState::ObstacleDetected;
            } 
            else {
                // CRUISE: Apply the smooth, non-linear speed
                mCurrentSpeedPWM = targetSpeedPWM; // Update actual physical speed
                mMotor.forward(mCurrentSpeedPWM);

                if (currentDistance > CLEAR_DISTANCE_MAX) {
                    mSearchAttempt = 0;
                }
            }
            break;
        }

        case RobotState::ObstacleDetected: {
            uint16_t reverseTime = (getSafeDistance() <= EMERGENCY_DISTANCE) ? REVERSE_STUCK_MS : REVERSE_SHORT_MS;
            mMotor.reverse(PWM_CRAWL);
            startAction(reverseTime);
            mState = RobotState::Reversing;
            break;
        }

        case RobotState::Reversing:
            if (!isSystemBusy()) {
                mMotor.stop();
                uint16_t angle = getScanAngle();
                mMotor.rotateLeft(angle, PWM_CRAWL);
                mState = RobotState::ScanningLeft;
            }
            break;

        // --- LEFT SCAN PIPELINE ---

        case RobotState::ScanningLeft:
            if (!isSystemBusy()) { // Turn finished
                startAction(SENSOR_SETTLE_MS);
                mState = RobotState::SettlingLeft;
            }
            break;

        case RobotState::SettlingLeft:
            if (!isActionBusy()) { // Settle finished
                mState = RobotState::MeasuringLeft;
            }
            break;

        case RobotState::MeasuringLeft: {
            mLeftScanDistance = getSafeDistance(); // Read flushed data
            if (mLeftScanDistance > MIN_PATH_CLEAR_CM) { // Changed from CAUTION_DISTANCE_MAX
                mState = RobotState::CommitLeftMove;
            } else {
                uint16_t angle = getScanAngle();
                mMotor.rotateRight(angle * 2, PWM_CRAWL);
                mState = RobotState::ScanningRight;
            }
            break;
        }

        // --- RIGHT SCAN PIPELINE ---

        case RobotState::ScanningRight:
            if (!isSystemBusy()) { // Turn finished
                startAction(SENSOR_SETTLE_MS);
                mState = RobotState::SettlingRight;
            }
            break;

        case RobotState::SettlingRight:
            if (!isActionBusy()) { // Settle finished
                mState = RobotState::MeasuringRight;
            }
            break;

        case RobotState::MeasuringRight: {
            mRightScanDistance = getSafeDistance(); // Read flushed data
            if (mRightScanDistance > MIN_PATH_CLEAR_CM) { // Changed from CAUTION_DISTANCE_MAX
                mState = RobotState::CommitRightMove;
            } else {
                uint16_t currentAngle = getScanAngle();
                mSearchAttempt++; 
                
                if (mSearchAttempt >= MAX_SEARCH_ATTEMPTS) {
                    mState = RobotState::StuckRecovery;
                    mRecoveryStep = RecoveryStep::Reversing;
                } else {
                    mMotor.rotateLeft(currentAngle, PWM_CRAWL); 
                    mState = RobotState::ReturnToCenter;
                }
            }
            break;
        }

        // --- RETURN TO CENTER PIPELINE ---

        case RobotState::ReturnToCenter:
            if (!isSystemBusy()) { // Turn finished
                startAction(SENSOR_SETTLE_MS);
                mState = RobotState::SettlingCenter;
            }
            break;

        case RobotState::SettlingCenter:
            if (!isActionBusy()) { // Settle finished
                mState = RobotState::ObstacleDetected;
            }
            break;

        // --- COMMIT MOVE PIPELINE ---

        case RobotState::CommitLeftMove: {
            mMotor.forward(PWM_CRAWL);
            float dist = getSafeDistance();
            if (dist > CLEAR_DISTANCE_MIN) {
                mSearchAttempt = 0;
                mState = RobotState::MovingForward;
            } else if (dist <= BLOCKED_DISTANCE_MAX) {
                mMotor.stop();
                mState = RobotState::ObstacleDetected;
            }
            break;
        }

        case RobotState::CommitRightMove: {
            mMotor.forward(PWM_CRAWL);
            float dist = getSafeDistance();
            if (dist > CLEAR_DISTANCE_MIN) {
                mSearchAttempt = 0;
                mState = RobotState::MovingForward;
            } else if (dist <= BLOCKED_DISTANCE_MAX) {
                mMotor.stop();
                mState = RobotState::ObstacleDetected;
            }
            break;
        }

        // --- STUCK RECOVERY PIPELINE ---

        case RobotState::StuckRecovery: {
            switch (mRecoveryStep) {
                case RecoveryStep::Reversing:
                    mMotor.reverse(PWM_SLOW);
                    startAction(REVERSE_STUCK_MS);
                    mRecoveryStep = RecoveryStep::TurningLeft90;
                    break;

                case RecoveryStep::TurningLeft90:
                    if (!isSystemBusy()) { // Reverse finished
                        mMotor.stop();
                        mMotor.rotateLeft(90, PWM_SLOW);
                        mRecoveryStep = RecoveryStep::SettlingLeft90;
                    }
                    break;

                case RecoveryStep::SettlingLeft90:
                    if (!isSystemBusy()) { // Turn finished
                        startAction(SENSOR_SETTLE_MS);
                        mRecoveryStep = RecoveryStep::MeasuringLeft90;
                    }
                    break;

                case RecoveryStep::MeasuringLeft90:
                    if (!isActionBusy()) { // Settle finished
                        mLeftScanDistance = getSafeDistance();
                        mMotor.rotateRight(180, PWM_SLOW);
                        mRecoveryStep = RecoveryStep::SettlingRight180;
                    }
                    break;

                case RecoveryStep::SettlingRight180:
                    if (!isSystemBusy()) { // Turn finished
                        startAction(SENSOR_SETTLE_MS);
                        mRecoveryStep = RecoveryStep::MeasuringRight180;
                    }
                    break;

                case RecoveryStep::MeasuringRight180:
                    if (!isActionBusy()) { // Settle finished
                        mRightScanDistance = getSafeDistance();
                        mRecoveryStep = RecoveryStep::TurningToBest;
                    }
                    break;

                case RecoveryStep::TurningToBest:
                    // Choose whichever side gave us at least 40cm
                    if (mLeftScanDistance > MIN_PATH_CLEAR_CM && mLeftScanDistance > mRightScanDistance) {
                        mMotor.rotateLeft(180, PWM_SLOW); 
                    } else if (mRightScanDistance > MIN_PATH_CLEAR_CM) {
                        // Right is better, already facing it. Do nothing.
                    } else {
                        // Both are blocked even after 180 scans. Just turn left and hope for the best.
                        mMotor.rotateLeft(180, PWM_SLOW);
                    }
                    mRecoveryStep = RecoveryStep::Committing;
                    break;

                case RecoveryStep::Committing:
                    if (!isSystemBusy()) {
                        mSearchAttempt = 0;
                        mState = RobotState::MovingForward;
                    }
                    break;
            }
            break;
        }
    }
}