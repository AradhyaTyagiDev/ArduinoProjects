#pragma once

#include <Arduino.h>

// =========================== Pin Definitions ==========================
// --- Hardware Pins (ESP32) --- 
// Motor A
constexpr int PIN_AIN1 = 17;
constexpr int PIN_AIN2 = 16;
constexpr int PIN_PWMA = 4;

// Motor B
constexpr int PIN_BIN1 = 18;
constexpr int PIN_BIN2 = 19;
constexpr int PIN_PWMB = 21;

constexpr int PIN_STBY = 5;

// Ultrasonic sensor (HS04)
constexpr int PIN_TRIG = 25; // Assign your HC-SR04 Trigger pin
constexpr int PIN_ECHO = 32; // Assign your HC-SR04 Echo pin

// --- Distance Thresholds (cm) ---
constexpr float DIST_FULL_SPEED_MIN = 150.0f;
constexpr float DIST_CLEAR_MAX = 149.0f;
constexpr float DIST_CLEAR_MIN = 90.0f;
constexpr float DIST_CAUTION_MAX = 89.0f;
constexpr float DIST_CAUTION_MIN = 50.0f;
constexpr float DIST_SLOW_MAX = 49.0f;
constexpr float DIST_SLOW_MIN = 25.0f;
constexpr float DIST_BLOCKED_MAX = 24.0f;
constexpr float DIST_BLOCKED_MIN = 18.0f;
constexpr float DIST_EMERGENCY = 10.0f;

// --- Speed Profiles (0-255 PWM) ---
constexpr uint8_t SPEED_FAST = 255;    // 100%
constexpr uint8_t SPEED_MEDIUM = 204;  // 80%
constexpr uint8_t SPEED_SLOW = 153;    // 60%
constexpr uint8_t SPEED_CRAWL = 102;   // 40%
constexpr uint8_t SPEED_VERY_SLOW = 50; // ~20% for obstacle negotiation

// Sensor update rate
constexpr unsigned long SENSOR_INTERVAL_MS = 33;  // ~30 Hz

// --- Robot States ---
enum class RobotState {
    Initializing,
    MovingForward,
    ObstacleDetected,
    SearchingPath,
    VerifyPath,
    DecidePath,
    TurningLeftMove,
    TurningRightMove,
    Reversing,
    StuckRecovery
};