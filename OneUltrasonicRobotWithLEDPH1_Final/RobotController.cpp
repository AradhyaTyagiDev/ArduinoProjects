#include "RobotController.h"
#include "LedRingController.h"

RobotController::RobotController(MotorController& motor, UltrasonicSensor& sensor)
  : mMotor(motor), mSensor(sensor) {}

void RobotController::begin() {
  mMotor.begin();
  mSensor.begin();

  // Initialize timers to prevent immediate triggering on startup
  mLastFrustrationDecay = millis();
  mLastRampTime = millis();
  
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

    case RobotState::MovingForward:
      {
        float currentDistance = getSafeDistance();
        uint32_t currentTime = millis();

        //Update LED
        updateLedStatus(currentDistance);

        // ---------------------------------------------------------
        // 1. Physics Engine: Closing Rate
        // ---------------------------------------------------------
        if (fabs(currentDistance - mPreviousDistance) > 0.5f) {
          float deltaTimeSec = (currentTime - mPreviousTime) / 1000.0f;
          if (deltaTimeSec > 0.020f) {
            float rawRate = (mPreviousDistance - currentDistance) / deltaTimeSec;
            mClosingRate = (mClosingRate * 0.6f) + (rawRate * 0.4f);
          }
          mPreviousDistance = currentDistance;
          mPreviousTime = currentTime;
        }

        // ---------------------------------------------------------
        // 2. Non-Linear Speed Curve
        // ---------------------------------------------------------
        float normalized = constrain((currentDistance - 40.0f) / 100.0f, 0.0f, 1.0f);
        float curve = normalized * normalized;
        uint8_t targetSpeedPWM = 50 + (curve * 205.0f);

        // ---------------------------------------------------------
        // 3. Dynamic Braking Zone
        // ---------------------------------------------------------
        float stoppingDistance = 20.0f + (mCurrentSpeedPWM * 0.35f);
        if (mClosingRate > 30.0f) {
          stoppingDistance += (mClosingRate * 0.25f);
        }

        // ---------------------------------------------------------
        // 4. Decision Time: Brake or Drive?
        // ---------------------------------------------------------
        if (currentDistance <= stoppingDistance || currentDistance <= 15.0f || mClosingRate > 150.0f) {
          mMotor.activeBrake();
          mCurrentSpeedPWM = 0;
          mClosingRate = 0.0f;

          if (currentDistance <= 15.0f) mSearchAttempt = 5;
          else mSearchAttempt = 0;

          mState = RobotState::ObstacleDetected;
        } else {
          // ---------------------------------------------------------
          // 5. Acceleration & Deceleration Ramping
          // ---------------------------------------------------------
          // We only adjust motor speed every 10 milliseconds.
          // This gives a snappy, lively start (slight front lift)
          // but prevents violent, uncontrolled wheelies.
          uint32_t rampElapsed = currentTime - mLastRampTime;

          if (rampElapsed >= 10) {
            int16_t speedDiff = targetSpeedPWM - mCurrentSpeedPWM;

            if (speedDiff > 0) {
              // ACCELERATE: Add 4 PWM per 10ms (400 PWM per second).
              // This takes ~0.45 seconds to reach full speed.
              // It's fast enough to give a cool, confident "pop",
              // but slow enough to keep the chassis stable.
              //Want MORE pop/lift? Change mCurrentSpeedPWM += 4; to mCurrentSpeedPWM += 5; or 6;
              //Want it smoother/less lift? Change mCurrentSpeedPWM += 4; to mCurrentSpeedPWM += 3; or 2;.
              //Want faster engine braking? Change mCurrentSpeedPWM -= 8; to -= 10;.
              mCurrentSpeedPWM += 4;
              if (mCurrentSpeedPWM > targetSpeedPWM) mCurrentSpeedPWM = targetSpeedPWM;
            } else if (speedDiff < 0) {
              // DECELERATE: Subtract 8 PWM per 10ms (800 PWM per second).
              // Brakes twice as fast as it accelerates for a natural "engine braking" feel.
              mCurrentSpeedPWM -= 8;
              if (mCurrentSpeedPWM < targetSpeedPWM) mCurrentSpeedPWM = targetSpeedPWM;
            }

            mLastRampTime = currentTime;  // Reset the 10ms timer
          }

          // ---------------------------------------------------------
          // 6. FRUSTRATION DECAY & ORGANIC PEEL DRIFT
          // ---------------------------------------------------------
          // Decay Frustration: If cruising in open space (>100cm), calm down over time
          if (currentDistance > 100.0f && (currentTime - mLastFrustrationDecay > 1000)) {
            mFrustrationLevel -= 0.2f;
            if (mFrustrationLevel < 0.0f) mFrustrationLevel = 0.0f;
            mLastFrustrationDecay = currentTime;  // Update the class member
          }

          // Base Wanderlust Sine Wave
          float wanderlustCurvature = sin(millis() / 2500.0f) * 0.30f;

          // Peel Bias: Steer away from the wall we've been hugging!
          // If last turn was LEFT (-1), and frustration is 2.0: bias = -(-1) * (2.0 * 0.15) = +0.3 (Drift Right)
          float peelBias = -static_cast<float>(mLastTurnDirection) * (mFrustrationLevel * 0.15f);

          float totalCurvature = wanderlustCurvature + peelBias;
          totalCurvature = constrain(totalCurvature, -1.0f, 1.0f);  // Keep within driveable limits

          // CRUISE: Apply speed with the dynamically biased drift
          mMotor.turn(totalCurvature, mCurrentSpeedPWM);

          if (currentDistance > CLEAR_DISTANCE_MAX) {
            mSearchAttempt = 0;
          }
        }
        break;
      }

    case RobotState::ObstacleDetected:
      {
        // ---------------------------------------------------------
        // FRUSTRATION INCREMENT: Getting stressed out!
        // ---------------------------------------------------------
        mFrustrationLevel += 1.0f;
        if (mFrustrationLevel > 3.0f) mFrustrationLevel = 3.0f;  // Cap at max panic

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

        // ---------------------------------------------------------
        // SCAN START BIAS: Prefer opposite of last turn
        // ---------------------------------------------------------
        TurnDir preferredStart = TurnDir::LEFT;  // Default

        if (mLastTurnDirection == TurnDir::LEFT) preferredStart = TurnDir::RIGHT;
        else if (mLastTurnDirection == TurnDir::RIGHT) preferredStart = TurnDir::LEFT;

        // If highly frustrated, force wider search angles because small ones are failing
        if (mFrustrationLevel >= 2.0f && mSearchAttempt < 3) {
          mSearchAttempt = 3;
        }

        if (preferredStart == TurnDir::RIGHT) {
          mScanStartDirection = TurnDir::RIGHT;
          mMotor.rotateRight(angle, PWM_CRAWL);
          mState = RobotState::ScanningRight;
        } else {
          mScanStartDirection = TurnDir::LEFT;
          mMotor.rotateLeft(angle, PWM_CRAWL);
          mState = RobotState::ScanningLeft;
        }
      }
      break;

      // --- LEFT SCAN PIPELINE ---

    case RobotState::ScanningLeft:
      if (!isSystemBusy()) {  // Turn finished
        startAction(SENSOR_SETTLE_MS);
        mState = RobotState::SettlingLeft;
      }
      break;

    case RobotState::SettlingLeft:
      if (!isActionBusy()) {  // Settle finished
        mState = RobotState::MeasuringLeft;
      }
      break;

    case RobotState::MeasuringLeft:
      {
        mLeftScanDistance = getSafeDistance();

        // ---------------------------------------------------------
        // SOFT SCAN BIAS: Penalize the direction we just came from
        // ---------------------------------------------------------
        // If last turn was LEFT, require Left path to be 15cm MORE clear to commit
        float leftThreshold = MIN_PATH_CLEAR_CM + (mLastTurnDirection == TurnDir::LEFT ? 15.0f : 0.0f);

        if (mLeftScanDistance > leftThreshold) {
          mCommitStartTime = millis();
          mLastTurnDirection = TurnDir::LEFT;
          mState = RobotState::CommitLeftMove;
        } else {
          uint16_t angle = getScanAngle();
          mMotor.rotateRight(angle * 2, PWM_CRAWL); //Turn RIGHT to look at the right side
          mState = RobotState::ScanningRight;
        }
        break;
      }

      // --- RIGHT SCAN PIPELINE ---

    case RobotState::ScanningRight:
      if (!isSystemBusy()) {  // Turn finished
        startAction(SENSOR_SETTLE_MS);
        mState = RobotState::SettlingRight;
      }
      break;

    case RobotState::SettlingRight:
      if (!isActionBusy()) {  // Settle finished
        mState = RobotState::MeasuringRight;
      }
      break;

    case RobotState::MeasuringRight:
      {
        mRightScanDistance = getSafeDistance();

        // ---------------------------------------------------------
        // SOFT SCAN BIAS: Penalize the direction we just came from
        // ---------------------------------------------------------
        // If last turn was RIGHT, require Right path to be 15cm MORE clear to commit
        float rightThreshold = MIN_PATH_CLEAR_CM + (mLastTurnDirection == TurnDir::RIGHT ? 15.0f : 0.0f);

        if (mRightScanDistance > rightThreshold) {
          mCommitStartTime = millis();
          mLastTurnDirection = TurnDir::RIGHT;
          mState = RobotState::CommitRightMove;
        } else {
          uint16_t currentAngle = getScanAngle();
          mSearchAttempt++;

          if (mSearchAttempt >= MAX_SEARCH_ATTEMPTS) {
            mState = RobotState::StuckRecovery;
            mRecoveryStep = RecoveryStep::Reversing;
          } else {
            if (mScanStartDirection == TurnDir::LEFT) {
              mMotor.rotateLeft(currentAngle, PWM_CRAWL);
            } else {
              mMotor.rotateRight(currentAngle, PWM_CRAWL);
            }
            mState = RobotState::ReturnToCenter;
          }
        }
        break;
      }

      // --- RETURN TO CENTER PIPELINE ---

    case RobotState::ReturnToCenter:
      if (!isSystemBusy()) {  // Turn finished
        startAction(SENSOR_SETTLE_MS);
        mState = RobotState::SettlingCenter;
      }
      break;

    case RobotState::SettlingCenter:
      if (!isActionBusy()) {  // Settle finished
        mState = RobotState::ObstacleDetected;
      }
      break;

    // --- COMMIT MOVE PIPELINE ---
    case RobotState::CommitLeftMove:
      {
        // ARC LEFT: Curvature -0.5f means the right wheel is 50% faster than the left.
        // This creates a smooth, car-like arc around the obstacle.
        mMotor.turn(-0.5f, PWM_CRAWL);

        float dist = getSafeDistance();
        uint32_t elapsed = millis() - mCommitStartTime;

        // EXIT CONDITIONS:
        // 1. The path is wide open again (> 90cm), meaning we successfully cleared the obstacle.
        // 2. TIMEOUT (1.5 seconds): Prevents infinite arcing if trapped in a U-shaped corner.
        if (dist > CLEAR_DISTANCE_MIN || elapsed > 1500) {
          mSearchAttempt = 0;
          mState = RobotState::MovingForward;  // Resume normal driving (with drift)
        }
        // EMERGENCY FALLBACK: If we get too close while arcing (e.g., side wall)
        else if (dist <= 25.0f) {
          mMotor.activeBrake();
          mState = RobotState::ObstacleDetected;
        }
        break;
      }

    case RobotState::CommitRightMove:
      {
        // ARC RIGHT: Curvature +0.5f means the left wheel is 50% faster than the right.
        mMotor.turn(0.5f, PWM_CRAWL);

        float dist = getSafeDistance();
        uint32_t elapsed = millis() - mCommitStartTime;

        if (dist > CLEAR_DISTANCE_MIN || elapsed > 1500) {
          mSearchAttempt = 0;
          mState = RobotState::MovingForward;
        } else if (dist <= 25.0f) {
          mMotor.activeBrake();
          mState = RobotState::ObstacleDetected;
        }
        break;
      }

      // --- STUCK RECOVERY PIPELINE ---

    case RobotState::StuckRecovery:
      {
        switch (mRecoveryStep) {
          case RecoveryStep::Reversing:
            mMotor.reverse(PWM_SLOW);
            startAction(REVERSE_STUCK_MS);
            mRecoveryStep = RecoveryStep::TurningLeft90;
            break;

          case RecoveryStep::TurningLeft90:
            if (!isSystemBusy()) {  // Reverse finished
              mMotor.stop();
              mMotor.rotateLeft(90, PWM_SLOW);
              mRecoveryStep = RecoveryStep::SettlingLeft90;
            }
            break;

          case RecoveryStep::SettlingLeft90:
            if (!isSystemBusy()) {  // Turn finished
              startAction(SENSOR_SETTLE_MS);
              mRecoveryStep = RecoveryStep::MeasuringLeft90;
            }
            break;

          case RecoveryStep::MeasuringLeft90:
            if (!isActionBusy()) {  // Settle finished
              mLeftScanDistance = getSafeDistance();
              mMotor.rotateRight(180, PWM_SLOW);
              mRecoveryStep = RecoveryStep::SettlingRight180;
            }
            break;

          case RecoveryStep::SettlingRight180:
            if (!isSystemBusy()) {  // Turn finished
              startAction(SENSOR_SETTLE_MS);
              mRecoveryStep = RecoveryStep::MeasuringRight180;
            }
            break;

          case RecoveryStep::MeasuringRight180:
            if (!isActionBusy()) {  // Settle finished
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