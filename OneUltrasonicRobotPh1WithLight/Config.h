#pragma once

#include <Arduino.h>

// ======================== Motor Pins ========================
constexpr uint8_t PIN_AIN1 = 17;
constexpr uint8_t PIN_AIN2 = 16;
constexpr uint8_t PIN_PWMA = 4;

constexpr uint8_t PIN_BIN1 = 18;
constexpr uint8_t PIN_BIN2 = 19;
constexpr uint8_t PIN_PWMB = 21;

constexpr uint8_t PIN_STBY = 5;

// ===================== Ultrasonic Pins HC-SR04 ======================
constexpr uint8_t PIN_TRIG = 25;
constexpr uint8_t PIN_ECHO = 32;


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

