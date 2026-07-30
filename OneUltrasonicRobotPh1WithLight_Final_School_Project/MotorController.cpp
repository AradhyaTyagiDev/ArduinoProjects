
#include "MotorController.h"

void MotorController::begin() {
    pinMode(PIN_AIN1, OUTPUT); 
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT); 
    pinMode(PIN_BIN2, OUTPUT);
    
    pinMode(PIN_STBY, OUTPUT);
    digitalWrite(PIN_STBY, HIGH);

    // Modern ESP32 Arduino Core v3.x syntax
    ledcAttach(PIN_PWMA, 20000, 8);
    ledcAttach(PIN_PWMB, 20000, 8);

    // Reset
    stop();
    mBusy = false;
}

void MotorController::forward(uint8_t speed) {
    digitalWrite(PIN_AIN1, HIGH); 
    digitalWrite(PIN_AIN2, LOW);
    digitalWrite(PIN_BIN1, HIGH); 
    digitalWrite(PIN_BIN2, LOW);

    ledcWrite(PIN_PWMA, speed); 
    ledcWrite(PIN_PWMB, speed);

    mBusy = false;  // forward is continuous, not timed
}

void MotorController::reverse(uint8_t speed) {
    digitalWrite(PIN_AIN1, LOW); 
    digitalWrite(PIN_AIN2, HIGH);
    digitalWrite(PIN_BIN1, LOW); 
    digitalWrite(PIN_BIN2, HIGH); // Fixed typo
    
    ledcWrite(PIN_PWMA, speed); 
    ledcWrite(PIN_PWMB, speed);
    mBusy = false;
}

void MotorController::rotateLeft(uint16_t angleDeg, uint8_t speed) {
    uint32_t duration = getTurnDuration(angleDeg, speed);
    
    digitalWrite(PIN_AIN1, LOW);  
    digitalWrite(PIN_AIN2, HIGH); // Left wheel reverse
    digitalWrite(PIN_BIN1, HIGH); 
    digitalWrite(PIN_BIN2, LOW);  // Right wheel forward

    ledcWrite(PIN_PWMA, speed); 
    ledcWrite(PIN_PWMB, speed);

    mActionEndTime = millis() + duration;
    mBusy = true;
}

void MotorController::rotateRight(uint16_t angleDeg, uint8_t speed) {
    uint32_t duration = getTurnDuration(angleDeg, speed);
    
    digitalWrite(PIN_AIN1, HIGH); 
    digitalWrite(PIN_AIN2, LOW);  // Left wheel forward
    digitalWrite(PIN_BIN1, LOW);  
    digitalWrite(PIN_BIN2, HIGH); // Right wheel reverse

    ledcWrite(PIN_PWMA, speed); 
    ledcWrite(PIN_PWMB, speed);

    mActionEndTime = millis() + duration;
    mBusy = true;
}

void MotorController::stop() {
    ledcWrite(PIN_PWMA, 0); 
    ledcWrite(PIN_PWMB, 0);
    mBusy = false;
}

void MotorController::activeBrake() {
    // Short Brake: Both directions HIGH fights the motor's inertia
    digitalWrite(PIN_AIN1, HIGH); 
    digitalWrite(PIN_AIN2, HIGH);
    digitalWrite(PIN_BIN1, HIGH); 
    digitalWrite(PIN_BIN2, HIGH);

    ledcWrite(PIN_PWMA, 0); 
    ledcWrite(PIN_PWMB, 0);
    
    mBusy = false;
}

// Merged the duplicate update functions into one clean version
void MotorController::update() {
    if (mBusy && millis() >= mActionEndTime) {
        stop();
    }
}

uint32_t MotorController::getTurnDuration(
    uint16_t angle,
    uint8_t speed) const
{
    // --------------------------------------------------
    // Zero-Input Check
    // --------------------------------------------------
    if (angle == 0 || speed == 0) {
        return 0;
    }

    // --------------------------------------------------
    // Clamp Inputs to Table Boundaries
    // --------------------------------------------------
    angle = std::clamp(angle, CAL_ANGLES[0], CAL_ANGLES[NUM_ANGLES - 1]);
    speed = std::clamp(speed, CAL_SPEEDS[0], CAL_SPEEDS[NUM_SPEEDS - 1]);

    // --------------------------------------------------
    // Find Angle Bounds
    // --------------------------------------------------
    size_t angleLow = 0;
    size_t angleHigh = 0;

    for(size_t i = 0; i < NUM_ANGLES - 1; ++i) {
        if(angle >= CAL_ANGLES[i] && angle <= CAL_ANGLES[i + 1]) {
            angleLow = i;
            angleHigh = i + 1;
            break;
        }
    }

    // --------------------------------------------------
    // Find Speed Bounds
    // --------------------------------------------------
    size_t speedLow = 0;
    size_t speedHigh = 0;

    for(size_t i = 0; i < NUM_SPEEDS - 1; ++i) {
        if(speed >= CAL_SPEEDS[i] && speed <= CAL_SPEEDS[i + 1]) {
            speedLow = i;
            speedHigh = i + 1;
            break;
        }
    }

    // --------------------------------------------------
    // Interpolation Ratios
    // --------------------------------------------------
    const float angleRatio =
        (angleHigh == angleLow)
        ? 0.0f
        : static_cast<float>(angle - CAL_ANGLES[angleLow]) /
          static_cast<float>(CAL_ANGLES[angleHigh] - CAL_ANGLES[angleLow]);

    const float speedRatio =
        (speedHigh == speedLow)
        ? 0.0f
        : static_cast<float>(speed - CAL_SPEEDS[speedLow]) /
          static_cast<float>(CAL_SPEEDS[speedHigh] - CAL_SPEEDS[speedLow]);

    // --------------------------------------------------
    // Table Corners (Explicit cast to float)
    // --------------------------------------------------
    const float d00 = static_cast<float>(CAL_DURATIONS[angleLow][speedLow]);
    const float d01 = static_cast<float>(CAL_DURATIONS[angleLow][speedHigh]);
    const float d10 = static_cast<float>(CAL_DURATIONS[angleHigh][speedLow]);
    const float d11 = static_cast<float>(CAL_DURATIONS[angleHigh][speedHigh]);

    // --------------------------------------------------
    // Bilinear Interpolation
    // --------------------------------------------------
    const float lowerSpeedDuration = d00 + angleRatio * (d10 - d00);
    const float upperSpeedDuration = d01 + angleRatio * (d11 - d01);

    float duration = lowerSpeedDuration + speedRatio * (upperSpeedDuration - lowerSpeedDuration);

    // --------------------------------------------------
    // Stiction Compensation
    // --------------------------------------------------
    constexpr uint16_t SMALL_ANGLE_THRESHOLD = 15;
    constexpr uint8_t  LOW_SPEED_THRESHOLD   = 100;
    constexpr uint16_t STARTUP_OVERHEAD_MS   = 30;

    if(angle <= SMALL_ANGLE_THRESHOLD && speed <= LOW_SPEED_THRESHOLD) {
        duration += STARTUP_OVERHEAD_MS;
    }

    return static_cast<uint32_t>(duration);
}