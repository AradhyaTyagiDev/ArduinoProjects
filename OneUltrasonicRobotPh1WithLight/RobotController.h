#pragma once

#include <Arduino.h>
#include "MotorController.h"
#include "UltrasonicSensor.h"

// =========================================================
// Distance Zones (cm)
// =========================================================
constexpr float FULL_SPEED_CLEAR_HIGHWAY = 150.0f;

constexpr float CLEAR_DISTANCE_MAX = 149.0f;
constexpr float CLEAR_DISTANCE_MIN = 90.0f;

constexpr float CAUTION_DISTANCE_MAX = 89.0f;
constexpr float CAUTION_DISTANCE_MIN = 50.0f;

constexpr float SLOW_DISTANCE_MAX = 49.0f;
constexpr float SLOW_DISTANCE_MIN = 25.0f;

constexpr float BLOCKED_DISTANCE_MAX = 24.0f;
constexpr float BLOCKED_DISTANCE_MIN = 18.0f;

constexpr float EMERGENCY_DISTANCE = 10.0f;

// =========================================================
// Motion Profiles (PWM)
// =========================================================
// constexpr uint8_t PWM_FULL   = 255; // 100%
// constexpr uint8_t PWM_MEDIUM = 204; // 80%
// constexpr uint8_t PWM_SLOW   = 153; // 60%
// constexpr uint8_t PWM_CRAWL  = 102; // 40%

constexpr uint8_t PWM_FULL   = 160; // Was 255. Capped for testing!
constexpr uint8_t PWM_MEDIUM = 130; // Was 204
constexpr uint8_t PWM_SLOW   = 110; // Was 153
constexpr uint8_t PWM_CRAWL  = 90;  // Was 102

// =========================================================
// Search & Recovery Config
// =========================================================
constexpr uint16_t SEARCH_ANGLES[] = {5, 10, 15, 25, 40, 55};
constexpr uint8_t MAX_SEARCH_ATTEMPTS = 6; // 0 to 5
constexpr uint16_t MAX_SCAN_ANGLE = 90;    // Cap to prevent >180 overflow

constexpr uint16_t REVERSE_SHORT_MS = 200;  // Reduced from 400. Just a tiny bump back.
constexpr uint16_t REVERSE_STUCK_MS  = 500; // Reduced from 800.

// How much clearance does the robot need to commit to a path?
// Lowering this from 89cm to 40cm stops the spinning in cluttered rooms.
constexpr float MIN_PATH_CLEAR_CM = 40.0f; 

// Time for 30Hz sensor to flush its 5-sample median buffer
constexpr uint16_t SENSOR_SETTLE_MS = 150; 

// =========================================================
// State Machines
// =========================================================
enum class RobotState {
    Initializing,
    MovingForward,
    ObstacleDetected,
    Reversing,
    ScanningLeft,
    SettlingLeft,
    MeasuringLeft,
    ScanningRight,
    SettlingRight,
    MeasuringRight,
    ReturnToCenter,
    SettlingCenter,
    CommitLeftMove,
    CommitRightMove,
    StuckRecovery
};

enum class RecoveryStep {
    Reversing,
    TurningLeft90,
    SettlingLeft90,
    MeasuringLeft90,
    TurningRight180,
    SettlingRight180,
    MeasuringRight180,
    TurningToBest,
    Committing
};

class RobotController {
public:
    RobotController(MotorController& motor, UltrasonicSensor& sensor);

    void begin();
    void update(); // Main non-blocking loop

private:

    MotorController& mMotor;
    UltrasonicSensor& mSensor;

    RobotState mState = RobotState::Initializing;
    RecoveryStep mRecoveryStep = RecoveryStep::Reversing;

    uint8_t mSearchAttempt = 0;
    float mLeftScanDistance = 0.0f;
    float mRightScanDistance = 0.0f;

    // Arcing & Commit Timers: Timer to prevent the robot from getting stuck in an infinite arc loop if it's trapped in a U-shaped corner.
    uint32_t mCommitStartTime = 0;

    // --------------------------------------------------
    // Immortal Non-Blocking Action Scheduler
    // --------------------------------------------------
    uint32_t mActionStartTime = 0;
    uint32_t mActionDuration = 0;

    void startAction(uint32_t durationMs);
    bool isActionBusy() const;
    bool isSystemBusy() const; // Checks both Motor turns and Scheduled actions

    // Physics & Dynamic Braking Variables
    float mPreviousDistance = 400.0f;
    uint32_t mPreviousTime = 0;
    uint8_t mCurrentSpeedPWM = 0; 
    float mClosingRate = 0.0f; // Low-pass filtered approach speed (cm/s)

    // Sensor Dropout Memory
    float mLastValidDistance = 150.0f;  // Initialize to a large, safe distance

    // --------------------------------------------------
    // Anti-Wall-Hugging & Frustration Memory
    // --------------------------------------------------
    // CRITICAL: LEFT must be -1 and RIGHT must be 1 for the peel math to work!
    enum class TurnDir : int8_t {
        NONE = 0,
        LEFT = -1,  // CRITICAL: Must be -1 for the peel math to work!
        RIGHT = 1   // CRITICAL: Must be 1 for the peel math to work!
    };

    // Anti-Wall-Hugging & Frustration Engine
    TurnDir mLastTurnDirection = TurnDir::NONE;
    TurnDir mScanStartDirection = TurnDir::NONE; 

    float mFrustrationLevel = 0.0f; // Unified stress level (0.0 Calm to 3.0 Panicked)
    uint32_t mLastFrustrationDecay = 0; // Use this instead of static variable

    uint32_t mLastRampTime = 0; // Tracks time for smooth, controlled acceleration

    // --------------------------------------------------
    // Helpers
    // --------------------------------------------------
    uint16_t getScanAngle();
    float getSafeDistance(); // NAN Immunity
};