

// #pragma once

// #include <Arduino.h>
// #include <algorithm> // Required for std::clamp
// #include "Config.h"  // Include Config.h directly

// // PWM Channels for ESP32 Core v2.x compatibility
// constexpr uint8_t PWMA_CHANNEL = 0;
// constexpr uint8_t PWMB_CHANNEL = 1;

// class MotorController {
// public:
//     MotorController() = default; 

//     void begin();
//     void forward(uint8_t speed);
//     void reverse(uint8_t speed);
//     void stop();
    
//     void rotateLeft(uint16_t angleDeg, uint8_t speed);
//     void rotateRight(uint16_t angleDeg, uint8_t speed);
//     void forwardForDuration(uint8_t speed, uint16_t durationMs);
//     void reverseForDuration(uint8_t speed, uint16_t durationMs);
    
//     bool isBusy() const { return mBusy; }
//     void update();

// private:
//     // FIX: Added 'const' here to match the .cpp implementation
//     uint32_t getTurnDuration(uint16_t angle, uint8_t speed) const;
    
//     uint32_t mActionEndTime = 0;
//     bool mBusy = false;

//     // --------------------------------------------------
//     // 2D Calibration Axes
//     // --------------------------------------------------
//     static constexpr uint16_t CAL_ANGLES[] = { 5, 10, 15, 25, 40, 55, 90, 180 };
//     static constexpr uint8_t CAL_SPEEDS[] = { 102, 153, 204, 255 };
    
//     static constexpr size_t NUM_ANGLES = sizeof(CAL_ANGLES) / sizeof(CAL_ANGLES[0]);
//     static constexpr size_t NUM_SPEEDS = sizeof(CAL_SPEEDS) / sizeof(CAL_SPEEDS[0]);

//     // --------------------------------------------------
//     // 2D Duration Table [angle_index][speed_index]
//     // --------------------------------------------------
//     static constexpr uint16_t CAL_DURATIONS[NUM_ANGLES][NUM_SPEEDS] = {
//         /* Speeds:   102     153     204     255  */
//         /* 5°   */ {  130,    70,     45,     35  },
//         /* 10°  */ {  250,   140,     85,     65  },
//         /* 15°  */ {  360,   200,    120,     90  },
//         /* 25°  */ {  580,   330,    200,    155  },
//         /* 40°  */ {  870,   500,    310,    240  },
//         /* 55°  */ { 1150,   660,    410,    320  },
//         /* 90°  */ { 1300,   780,    490,    370  },
//         /* 180° */ { 2600,  1560,    980,    740  }
//     };
// };

// #include "MotorController.h"

// void MotorController::begin() {
//     pinMode(PIN_AIN1, OUTPUT); 
//     pinMode(PIN_AIN2, OUTPUT);
//     pinMode(PIN_BIN1, OUTPUT); 
//     pinMode(PIN_BIN2, OUTPUT);
    
//     pinMode(PIN_STBY, OUTPUT);
//     digitalWrite(PIN_STBY, HIGH);

//     // FIX: ESP32 Core v2.x API (Compatible with all ESP32 boards)
//     ledcSetup(PWMA_CHANNEL, 20000, 8);
//     ledcSetup(PWMB_CHANNEL, 20000, 8);
//     ledcAttachPin(PIN_PWMA, PWMA_CHANNEL);
//     ledcAttachPin(PIN_PWMB, PWMB_CHANNEL);

//     stop();
//     mBusy = false;
// }

// void MotorController::forward(uint8_t speed) {
//     digitalWrite(PIN_AIN1, HIGH); 
//     digitalWrite(PIN_AIN2, LOW);
//     digitalWrite(PIN_BIN1, HIGH); 
//     digitalWrite(PIN_BIN2, LOW);

//     ledcWrite(PWMA_CHANNEL, speed); 
//     ledcWrite(PWMB_CHANNEL, speed);

//     mBusy = false;  // Continuous action
// }

// void MotorController::reverse(uint8_t speed) {
//     digitalWrite(PIN_AIN1, LOW); 
//     digitalWrite(PIN_AIN2, HIGH);
//     digitalWrite(PIN_BIN1, LOW); 
//     digitalWrite(PIN_BIN2, HIGH);
    
//     ledcWrite(PWMA_CHANNEL, speed); 
//     ledcWrite(PWMB_CHANNEL, speed);
//     mBusy = false; // Continuous action
// }

// void MotorController::forwardForDuration(uint8_t speed, uint16_t durationMs) {
//     digitalWrite(PIN_AIN1, HIGH); 
//     digitalWrite(PIN_AIN2, LOW);
//     digitalWrite(PIN_BIN1, HIGH); 
//     digitalWrite(PIN_BIN2, LOW);
    
//     ledcWrite(PWMA_CHANNEL, speed); 
//     ledcWrite(PWMB_CHANNEL, speed);
    
//     mActionEndTime = millis() + durationMs;
//     mBusy = true; // Timed action
// }

// void MotorController::reverseForDuration(uint8_t speed, uint16_t durationMs) {
//     digitalWrite(PIN_AIN1, LOW); 
//     digitalWrite(PIN_AIN2, HIGH);
//     digitalWrite(PIN_BIN1, LOW); 
//     digitalWrite(PIN_BIN2, HIGH);
    
//     ledcWrite(PWMA_CHANNEL, speed); 
//     ledcWrite(PWMB_CHANNEL, speed);
    
//     mActionEndTime = millis() + durationMs;
//     mBusy = true; // Timed action
// }

// void MotorController::rotateLeft(uint16_t angleDeg, uint8_t speed) {
//     uint32_t duration = getTurnDuration(angleDeg, speed);
    
//     digitalWrite(PIN_AIN1, LOW);  
//     digitalWrite(PIN_AIN2, HIGH); // Left wheel reverse
//     digitalWrite(PIN_BIN1, HIGH); 
//     digitalWrite(PIN_BIN2, LOW);  // Right wheel forward

//     ledcWrite(PWMA_CHANNEL, speed); 
//     ledcWrite(PWMB_CHANNEL, speed);

//     mActionEndTime = millis() + duration;
//     mBusy = true;
// }

// void MotorController::rotateRight(uint16_t angleDeg, uint8_t speed) {
//     uint32_t duration = getTurnDuration(angleDeg, speed);
    
//     digitalWrite(PIN_AIN1, HIGH); 
//     digitalWrite(PIN_AIN2, LOW);  // Left wheel forward
//     digitalWrite(PIN_BIN1, LOW);  
//     digitalWrite(PIN_BIN2, HIGH); // Right wheel reverse

//     ledcWrite(PWMA_CHANNEL, speed); 
//     ledcWrite(PWMB_CHANNEL, speed);

//     mActionEndTime = millis() + duration;
//     mBusy = true;
// }

// void MotorController::stop() {
//     ledcWrite(PWMA_CHANNEL, 0); 
//     ledcWrite(PWMB_CHANNEL, 0);
//     mBusy = false;
// }

// void MotorController::update() {
//     if (mBusy && millis() >= mActionEndTime) {
//         stop();
//     }
// }

// uint32_t MotorController::getTurnDuration(uint16_t angle, uint8_t speed) const {
//     if (angle == 0 || speed == 0) return 0;

//     angle = std::clamp(angle, CAL_ANGLES[0], CAL_ANGLES[NUM_ANGLES - 1]);
//     speed = std::clamp(speed, CAL_SPEEDS[0], CAL_SPEEDS[NUM_SPEEDS - 1]);

//     size_t angleLow = 0, angleHigh = 0;
//     for(size_t i = 0; i < NUM_ANGLES - 1; ++i) {
//         if(angle >= CAL_ANGLES[i] && angle <= CAL_ANGLES[i + 1]) {
//             angleLow = i; angleHigh = i + 1; break;
//         }
//     }

//     size_t speedLow = 0, speedHigh = 0;
//     for(size_t i = 0; i < NUM_SPEEDS - 1; ++i) {
//         if(speed >= CAL_SPEEDS[i] && speed <= CAL_SPEEDS[i + 1]) {
//             speedLow = i; speedHigh = i + 1; break;
//         }
//     }

//     const float angleRatio = (angleHigh == angleLow) ? 0.0f : 
//         static_cast<float>(angle - CAL_ANGLES[angleLow]) / static_cast<float>(CAL_ANGLES[angleHigh] - CAL_ANGLES[angleLow]);
    
//     const float speedRatio = (speedHigh == speedLow) ? 0.0f : 
//         static_cast<float>(speed - CAL_SPEEDS[speedLow]) / static_cast<float>(CAL_SPEEDS[speedHigh] - CAL_SPEEDS[speedLow]);

//     const float d00 = static_cast<float>(CAL_DURATIONS[angleLow][speedLow]);
//     const float d01 = static_cast<float>(CAL_DURATIONS[angleLow][speedHigh]);
//     const float d10 = static_cast<float>(CAL_DURATIONS[angleHigh][speedLow]);
//     const float d11 = static_cast<float>(CAL_DURATIONS[angleHigh][speedHigh]);

//     const float lowerSpeedDuration = d00 + angleRatio * (d10 - d00);
//     const float upperSpeedDuration = d01 + angleRatio * (d11 - d01);

//     float duration = lowerSpeedDuration + speedRatio * (upperSpeedDuration - lowerSpeedDuration);

//     // Stiction Compensation
//     if(angle <= 15 && speed <= 100) duration += 30.0f;

//     return static_cast<uint32_t>(duration);
// }