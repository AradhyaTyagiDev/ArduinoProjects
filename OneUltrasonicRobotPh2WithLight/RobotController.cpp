#include "RobotController.h"

RobotController::RobotController(MotorController& motor, UltrasonicSensor& sensor)
  : mMotor(motor), mSensor(sensor) {}

void RobotController::begin() {
  mMotor.begin();
  mSensor.begin();
  mLastFrustrationDecay = millis();
  mLastRampTime = millis();
  mStraightDriveTime = 0; 
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
  if (isnan(d)) return mLastValidDistance;
  if (d < 2.0f) return mLastValidDistance;
  
  // CRITICAL GLITCH FILTER: Ignore sudden drops in open spaces
  if (d < 30.0f && mLastValidDistance > 70.0f) {
    return mLastValidDistance; 
  }
  
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
      // 1. Physics Engine: Closing Rate (Low-pass filtered!)
      // ---------------------------------------------------------
      if (fabs(currentDistance - mPreviousDistance) > 0.5f) {
        float deltaTimeSec = (currentTime - mPreviousTime) / 1000.0f;
        if (deltaTimeSec > 0.020f) {
          float rawRate = (mPreviousDistance - currentDistance) / deltaTimeSec;
          mClosingRate = (mClosingRate * 0.6f) + (rawRate * 0.4f); // Filter smooths noise
        }
        mPreviousDistance = currentDistance;
        mPreviousTime = currentTime;
      }

      // ---------------------------------------------------------
      // 2. Non-Linear Speed Curve
      // ---------------------------------------------------------
      float normalized = constrain((currentDistance - 30.0f) / 120.0f, 0.0f, 1.0f);
      float curve = normalized * normalized;
      uint8_t targetSpeedPWM = 80 + (curve * 175.0f); 

      // ---------------------------------------------------------
      // 3. Dynamic Braking Zone
      // ---------------------------------------------------------
      float stoppingDistance = 25.0f + (mCurrentSpeedPWM * 0.38f);
      if (mClosingRate > 30.0f) {
        stoppingDistance += (mClosingRate * 0.25f);
      }

      // ---------------------------------------------------------
      // 4. Decision Time: Flinch, Hard Brake, Smooth Bypass, or Cruise?
      // ---------------------------------------------------------
      
      // A. FLINCH REFLEX: Hand swiped (Trap B logic applied here)
      if (mClosingRate > 150.0f && currentDistance > 20.0f) {
        mMotor.activeBrake(); 
        mCurrentSpeedPWM = 0; 
        mClosingRate = 0.0f;
        mIsReactiveSteering = false;
        mState = RobotState::Flinching; // Trigger Flinch pipeline
      }
      // B. HARD BRAKE: Too close physically (Failed Swerve Fallback)
      else if (currentDistance <= stoppingDistance || currentDistance <= 20.0f) {
        mMotor.activeBrake(); 
        mCurrentSpeedPWM = 0; 
        mClosingRate = 0.0f;
        mIsReactiveSteering = false;

        if (currentDistance <= 15.0f) mSearchAttempt = 5; 
        else mSearchAttempt = 2; 

        mState = RobotState::ObstacleDetected;
      }
      // C. SMOOTH BYPASS ZONE (<= 70cm). Slip around without stopping.
      else if (currentDistance <= 70.0f) {
        
        if (!mIsReactiveSteering) {
            mIsReactiveSteering = true;
            mReactiveSteerStartTime = currentTime;
            mStraightDriveTime = 0; 
        }

        uint32_t rampElapsed = currentTime - mLastRampTime;
        if (rampElapsed >= 10) {
          int16_t speedDiff = targetSpeedPWM - mCurrentSpeedPWM;
          if (speedDiff > 0) mCurrentSpeedPWM += 6; 
          else if (speedDiff < 0) mCurrentSpeedPWM -= 10; 
          mCurrentSpeedPWM = constrain(mCurrentSpeedPWM, 0, targetSpeedPWM);
          mLastRampTime = currentTime; 
        }

        // FIX 1: Default to RIGHT (1.0f) if no memory exists yet to prevent 0.0f curvature!
        float dirMultiplier = (mLastTurnDirection == TurnDir::LEFT) ? -1.0f : 1.0f;
        float reactiveCurve = dirMultiplier * 0.5f;
        
        mMotor.turn(reactiveCurve, mCurrentSpeedPWM);
      }
      // D. CRUISE ZONE: Path is clear! (> 70cm).
      else {
        uint32_t rampElapsed = currentTime - mLastRampTime;
        if (rampElapsed >= 10) {
          int16_t speedDiff = targetSpeedPWM - mCurrentSpeedPWM;
          if (speedDiff > 0) mCurrentSpeedPWM += 6; 
          else if (speedDiff < 0) mCurrentSpeedPWM -= 10; 
          mCurrentSpeedPWM = constrain(mCurrentSpeedPWM, 0, targetSpeedPWM);
          mLastRampTime = currentTime; 
        }

        // ---------------------------------------------------------
        // 5. LANE RECOVERY TRIGGER (Trap A logic applied here)
        // ---------------------------------------------------------
        if (mIsReactiveSteering) {
          // FIX 2: Removed "&& swerveDuration > 1500". 
          // We must recenter the MOMENT the sensor clears the box (>90cm).
          if (currentDistance > 90.0f) { 
            mIsReactiveSteering = false;
            
            // TRAP A FIX: Strictly time-limited (700ms) and gentle curvature (0.25f)
            float dirMultiplier = (mLastTurnDirection == TurnDir::LEFT) ? -1.0f : 1.0f;
            mRecenterCurvature = -dirMultiplier * 0.25f;
            startAction(700); // Exactly 700ms limit as requested
            mState = RobotState::Recentering;
          } else {
            float dirMultiplier = (mLastTurnDirection == TurnDir::LEFT) ? -1.0f : 1.0f;
            float reactiveCurve = dirMultiplier * 0.5f;
            mMotor.turn(reactiveCurve, mCurrentSpeedPWM);
          }
        } else {
          // --- Normal Stubborn Cruising Logic ---
          float totalCurvature = 0.0f; 

          if (currentTime - mLastFrustrationDecay > 1000) {
            mFrustrationLevel -= 0.2f;
            if (mFrustrationLevel < 0.0f) mFrustrationLevel = 0.0f;
            mLastFrustrationDecay = currentTime; 
          }

          if (currentDistance > 80.0f) {
            if (mStraightDriveTime == 0) {
              mStraightDriveTime = currentTime; 
            }
            if (currentTime - mStraightDriveTime > 10000) {
              totalCurvature += sin(millis() / 3000.0f) * 0.05f; 
            }
          } else {
            mStraightDriveTime = 0; 
          }

          if (mFrustrationLevel > 1.0f) {
            float dirMultiplier = (mLastTurnDirection == TurnDir::LEFT) ? -1.0f : 1.0f;
            float peelBias = -dirMultiplier * (mFrustrationLevel * 0.05f);
            totalCurvature += peelBias;
          }

          totalCurvature = constrain(totalCurvature, -0.15f, 0.15f);

          mMotor.turn(totalCurvature, mCurrentSpeedPWM);

          if (currentDistance > CLEAR_DISTANCE_MAX) {
            mSearchAttempt = 0;
          }
        }
      }
      break;
    }

    // =========================================================
    // --- FLINCH REFLEX PIPELINE ---
    // =========================================================
    case RobotState::Flinching:
      mMotor.activeBrake();
      startAction(300); // 300ms pause to assess
      mState = RobotState::AssessingFlinch;
      break;

    case RobotState::AssessingFlinch:
      if (!isActionBusy()) {
        // FIX 1 APPLIED HERE TOO: Default to RIGHT if no memory exists
        float dirMultiplier = (mLastTurnDirection == TurnDir::LEFT) ? -1.0f : 1.0f;
        float slipCurvature = dirMultiplier * 0.8f;
        
        mMotor.turn(slipCurvature, PWM_CRAWL);
        startAction(800); // 800ms tight slip
        mState = RobotState::Slipping;
      }
      break;

    case RobotState::Slipping:
      if (!isActionBusy()) {
        // Successfully slipped past!
        float dirMultiplier = (mLastTurnDirection == TurnDir::LEFT) ? -1.0f : 1.0f;
        mRecenterCurvature = -dirMultiplier * 0.25f;
        startAction(700);
        mState = RobotState::Recentering;
      } else if (getSafeDistance() <= 25.0f) {
        // Still blocked after slipping! Decisive Retreat immediately.
        mMotor.activeBrake();
        mState = RobotState::StuckRecovery;
        mRecoveryStep = RecoveryStep::Reversing;
      }
      break;

    // =========================================================
    // --- STANDARD OBSTACLE PIPELINE ---
    // =========================================================
    case RobotState::ObstacleDetected: {
      mFrustrationLevel += 1.0f;
      if (mFrustrationLevel > 3.0f) mFrustrationLevel = 3.0f;
      
      if (mFrustrationLevel >= 3.0f) {
        mState = RobotState::StuckRecovery;
        mRecoveryStep = RecoveryStep::Reversing;
        break;
      }
      
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
        TurnDir preferredStart = (mLastTurnDirection == TurnDir::LEFT) ? TurnDir::RIGHT : TurnDir::LEFT;
        
        if (mFrustrationLevel >= 2.0f && mSearchAttempt < 3) mSearchAttempt = 3;
        
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
      if (!isSystemBusy()) { startAction(SENSOR_SETTLE_MS); mState = RobotState::SettlingLeft; }
      break;

    case RobotState::SettlingLeft:
      if (!isActionBusy()) { mState = RobotState::MeasuringLeft; }
      break;

    case RobotState::MeasuringLeft: {
      mLeftScanDistance = getSafeDistance();
      float leftThreshold = MIN_PATH_CLEAR_CM + (mLastTurnDirection == TurnDir::LEFT ? 15.0f : 0.0f);
      
      if (mLeftScanDistance > leftThreshold) {
        mCommitStartTime = millis();
        mLastTurnDirection = TurnDir::LEFT;
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
      if (!isSystemBusy()) { startAction(SENSOR_SETTLE_MS); mState = RobotState::SettlingRight; }
      break;

    case RobotState::SettlingRight:
      if (!isActionBusy()) { mState = RobotState::MeasuringRight; }
      break;

    case RobotState::MeasuringRight: {
      mRightScanDistance = getSafeDistance();
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
          if (mScanStartDirection == TurnDir::LEFT) mMotor.rotateLeft(currentAngle, PWM_CRAWL);
          else mMotor.rotateRight(currentAngle, PWM_CRAWL);
          mState = RobotState::ReturnToCenter;
        }
      }
      break;
    }

    // --- RETURN TO CENTER PIPELINE ---
    case RobotState::ReturnToCenter:
      if (!isSystemBusy()) { startAction(SENSOR_SETTLE_MS); mState = RobotState::SettlingCenter; }
      break;

    case RobotState::SettlingCenter:
      if (!isActionBusy()) { mState = RobotState::ObstacleDetected; }
      break;

    // --- COMMIT MOVE PIPELINE ---
    case RobotState::CommitLeftMove: {
      mMotor.turn(-0.5f, PWM_MEDIUM); // Fast bypass arc
      float dist = getSafeDistance();
      uint32_t elapsed = millis() - mCommitStartTime;
      
      if (dist > CLEAR_DISTANCE_MIN || elapsed > 1500) {
        mSearchAttempt = 0;
        // TRAP A FIX: Low speed, fixed 700ms, gentle 0.25f
        mRecenterCurvature = 0.25f;
        startAction(700);
        mState = RobotState::Recentering;
      } else if (dist <= 25.0f) {
        mMotor.activeBrake();
        mState = RobotState::ObstacleDetected;
      }
      break;
    }

    case RobotState::CommitRightMove: {
      mMotor.turn(0.5f, PWM_MEDIUM); 
      float dist = getSafeDistance();
      uint32_t elapsed = millis() - mCommitStartTime;
      
      if (dist > CLEAR_DISTANCE_MIN || elapsed > 1500) {
        mSearchAttempt = 0;
        // TRAP A FIX: Low speed, fixed 700ms, gentle 0.25f
        mRecenterCurvature = -0.25f;
        startAction(700);
        mState = RobotState::Recentering;
      } else if (dist <= 25.0f) {
        mMotor.activeBrake();
        mState = RobotState::ObstacleDetected;
      }
      break;
    }

    // --- LANE RECOVERY (TRAP A ENFORCED) ---
    case RobotState::Recentering: {
      // TRAP A FIX: Use PWM_CRAWL strictly! 
      mMotor.turn(mRecenterCurvature, PWM_CRAWL); 
      
      if (getSafeDistance() <= 60.0f) {
        mMotor.activeBrake();
        mState = RobotState::ObstacleDetected;
      }
      else if (!isActionBusy()) {
        mState = RobotState::MovingForward; 
      }
      break;
    }

    // --- DECISIVE RETREAT ---
    case RobotState::StuckRecovery: {
      switch (mRecoveryStep) {
        case RecoveryStep::Reversing:
          mMotor.reverse(PWM_SLOW);
          startAction(400);
          mRecoveryStep = RecoveryStep::Turning180;
          break;
          
        case RecoveryStep::Turning180:
          if (!isSystemBusy()) {
            mMotor.stop();
            if (mLastTurnDirection == TurnDir::LEFT) {
              mMotor.rotateRight(180, PWM_SLOW);
            } else {
              mMotor.rotateLeft(180, PWM_SLOW);
            }
            mRecoveryStep = RecoveryStep::Committing;
          }
          break;
          
        case RecoveryStep::Committing:
          if (!isSystemBusy()) {
            mSearchAttempt = 0;
            mFrustrationLevel = 0.0f;
            mRecenterCurvature = 0.0f;
            mState = RobotState::MovingForward;
          }
          break;
      }
      break;
    }
  }
}