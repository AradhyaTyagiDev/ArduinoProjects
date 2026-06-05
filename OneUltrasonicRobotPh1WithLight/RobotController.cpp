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
    if (isnan(d)) return 0.0f; // NAN Immunity
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
            float distance = getSafeDistance();
            if (distance <= EMERGENCY_DISTANCE) {
                mMotor.stop();
                mSearchAttempt = 5; 
                mState = RobotState::ObstacleDetected;
            } else if (distance <= BLOCKED_DISTANCE_MIN) {
                mMotor.stop();
                if (mSearchAttempt < 4) mSearchAttempt = 4;
                mState = RobotState::ObstacleDetected;
            } else if (distance <= BLOCKED_DISTANCE_MAX) {
                mMotor.stop();
                mState = RobotState::ObstacleDetected;
            } else if (distance <= SLOW_DISTANCE_MIN) {
                mMotor.forward(PWM_CRAWL);
            } else if (distance <= CAUTION_DISTANCE_MIN) {
                mMotor.forward(PWM_SLOW);
            } else if (distance <= CLEAR_DISTANCE_MIN) {
                mMotor.forward(PWM_MEDIUM);
            } else {
                mMotor.forward(PWM_FULL);
                if (distance > CLEAR_DISTANCE_MAX) mSearchAttempt = 0;
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