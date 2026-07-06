#pragma once

#include <Arduino.h>
#include <algorithm> // Required for std::clamp

#include "MotorController.h"
#include "Config.h"

class MotorController {
public:
    void begin();
    void forward(uint8_t speed);
    void reverse(uint8_t speed);
    void stop();
    void activeBrake();
    
    void rotateLeft(uint16_t angleDeg, uint8_t speed);
    void rotateRight(uint16_t angleDeg, uint8_t speed);

    void driveDifferential(int16_t leftPWM, int16_t rightPWM);
    void turn(float curvature, uint8_t baseSpeed); // -1.0 (Left) to 1.0 (Right)
    
    bool isBusy() const { return mBusy; }
    void update();

private:
    uint32_t getTurnDuration(uint16_t angle, uint8_t speed) const;
    
    uint32_t mActionEndTime = 0;
    bool mBusy = false;

    
    // --------------------------------------------------
    // 2D Calibration Axes
    // --------------------------------------------------
    
    // Angles specifically match your searchAttempt escalation logic
    static constexpr uint16_t CAL_ANGLES[] = { 5, 10, 15, 25, 40, 55, 90, 180 };
    
    // Speeds specifically match your State Machine's PWM outputs (40%, 60%, 80%, 100%)
    static constexpr uint8_t CAL_SPEEDS[] = { 102, 153, 204, 255 };
    
    static constexpr size_t NUM_ANGLES = sizeof(CAL_ANGLES) / sizeof(CAL_ANGLES[0]);
    static constexpr size_t NUM_SPEEDS = sizeof(CAL_SPEEDS) / sizeof(CAL_SPEEDS[0]);

    // --------------------------------------------------
    // 2D Duration Table [angle_index][speed_index]
    // --------------------------------------------------
    static constexpr uint16_t CAL_DURATIONS[NUM_ANGLES][NUM_SPEEDS] = {
        // Speeds:   102     153     204     255  
        /* 5°   */ {  130,    70,     45,     35  },
        /* 10°  */ {  250,   140,     85,     65  },
        /* 15°  */ {  360,   200,    120,     90  },
        /* 25°  */ {  580,   330,    200,    155  },
        /* 40°  */ {  870,   500,    310,    240  },
        /* 55°  */ { 1150,   660,    410,    320  },
        /* 90°  */ { 1300,   780,    490,    370  }, // <- Core Anchors
        /* 180° */ { 2600,  1560,    980,    740  }
    };
};