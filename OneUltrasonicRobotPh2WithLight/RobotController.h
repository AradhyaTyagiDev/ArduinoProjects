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
constexpr uint8_t PWM_FULL   = 255; 
constexpr uint8_t PWM_MEDIUM = 200; 
constexpr uint8_t PWM_SLOW   = 140; 
constexpr uint8_t PWM_CRAWL  = 90;  

// =========================================================
// Search & Recovery Config
// =========================================================
constexpr uint16_t SEARCH_ANGLES[] = {5, 10, 15, 25, 40, 55};
constexpr uint8_t MAX_SEARCH_ATTEMPTS = 6; 
constexpr uint16_t MAX_SCAN_ANGLE = 90;    

constexpr uint16_t REVERSE_SHORT_MS = 200;  
constexpr uint16_t REVERSE_STUCK_MS  = 500; 

constexpr float MIN_PATH_CLEAR_CM = 40.0f; 
constexpr uint16_t SENSOR_SETTLE_MS = 150; 

// =========================================================
// State Machines
// =========================================================
enum class RobotState {
    Initializing,
    MovingForward,
    Flinching,           // NEW: Hard brake on hand swipe
    AssessingFlinch,     // NEW: 300ms pause
    Slipping,            // NEW: Tight 0.8f arc to dodge
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
    Recentering,         
    StuckRecovery
};

enum class RecoveryStep {
    Reversing,           
    Turning180,          
    Committing
};

class RobotController {
public:
    RobotController(MotorController& motor, UltrasonicSensor& sensor);

    void begin();
    void update(); 

private:

    MotorController& mMotor;
    UltrasonicSensor& mSensor;

    RobotState mState = RobotState::Initializing;
    RecoveryStep mRecoveryStep = RecoveryStep::Reversing;

    uint8_t mSearchAttempt = 0;
    float mLeftScanDistance = 0.0f;
    float mRightScanDistance = 0.0f;
    uint32_t mCommitStartTime = 0;

    // --- Immortal Non-Blocking Action Scheduler ---
    uint32_t mActionStartTime = 0;
    uint32_t mActionDuration = 0;

    void startAction(uint32_t durationMs);
    bool isActionBusy() const;
    bool isSystemBusy() const; 

    // --- Physics & Dynamic Braking ---
    float mPreviousDistance = 400.0f;
    uint32_t mPreviousTime = 0;
    uint8_t mCurrentSpeedPWM = 0; 
    float mClosingRate = 0.0f; 
    float mLastValidDistance = 150.0f;  

    // --- Anti-Wall-Hugging & Frustration Memory ---
    enum class TurnDir : int8_t { NONE = 0, LEFT = -1, RIGHT = 1 };

    TurnDir mLastTurnDirection = TurnDir::RIGHT; 
    TurnDir mScanStartDirection = TurnDir::NONE; 

    float mFrustrationLevel = 0.0f; 
    uint32_t mLastFrustrationDecay = 0; 
    uint32_t mLastRampTime = 0; 

    // --- Lane Keeping & Reflex Variables ---
    uint32_t mStraightDriveTime = 0;     
    bool mIsReactiveSteering = false;    
    float mRecenterCurvature = 0.0f;     
    uint32_t mReactiveSteerStartTime = 0; 

    // --- Helpers ---
    uint16_t getScanAngle();
    float getSafeDistance(); 
};