#pragma once

#include <Arduino.h>
#include "Config.h"

class UltrasonicSensor {
public:
    UltrasonicSensor(uint8_t trigPin, uint8_t echoPin);

    void begin();
    
    // MUST be called every loop() iteration. Non-blocking state machine.
    void update();

    // Returns the latest smoothed distance. Call this anytime.
    float getDistance() const { return mSmoothedDistance; }

private:
    uint8_t mTrigPin = PIN_TRIG;
    uint8_t mEchoPin = PIN_ECHO;

    // --- Non-blocking State Machine ---
    enum class PingState : uint8_t {
        IDLE,               // Waiting for 33ms interval
        TRIGGER_PULSE,      // Sending 10us pulse
        WAIT_ECHO_START,    // Waiting for Echo pin to go HIGH
        WAIT_ECHO_END       // Measuring the pulse width
    };
    
    PingState mState = PingState::IDLE;
    uint32_t mLastPingTimeMs = 0;
    uint32_t mTriggerTimeUs = 0;
    uint32_t mEchoStartTimeUs = 0;

    // --- Rolling Median Filter (Across Time) ---
    // A window of the last 5 readings taken 33ms apart.
    static constexpr size_t MEDIAN_SIZE = 5;
    float mMedianBuffer[MEDIAN_SIZE] = {400.0f, 400.0f, 400.0f, 400.0f, 400.0f};
    size_t mBufferIndex = 0;

    // --- Exponential Smoothing ---
    float mSmoothedDistance = 400.0f;
    static constexpr float ALPHA = 0.3f; // 30% new data, 70% old data

    // --- Timing & Constraints ---
    static constexpr uint16_t PING_INTERVAL_MS = 33;   // ~30 Hz (Also respects >60ms acoustic decay for new unique paths)
    static constexpr uint16_t MAX_DISTANCE_CM = 400;
    static constexpr uint16_t MIN_DISTANCE_CM = 2;
    static constexpr uint32_t MAX_ECHO_TIME_US = MAX_DISTANCE_CM * 58; // ~23200 us

    // Internal methods
    void processRawDistance(float distance);
    float calculateRollingMedian();
};