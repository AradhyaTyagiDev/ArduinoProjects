#pragma once

#include <Arduino.h>

constexpr uint8_t TRIG_PIN = 25;
constexpr uint8_t ECHO_PIN = 32;

class UltrasonicSensor {
public:
    UltrasonicSensor(uint8_t trigPin, uint8_t echoPin);

    void begin();
    
    // MUST be called every loop() iteration. Non-blocking state machine.
    void update();

    // Returns the latest smoothed distance. Call this anytime.
    float getDistance() const { return mSmoothedDistance; }

private:
    uint8_t mTrigPin = TRIG_PIN;
    uint8_t mEchoPin = ECHO_PIN;

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

UltrasonicSensor::UltrasonicSensor(uint8_t trigPin, uint8_t echoPin)
    : mTrigPin(trigPin), mEchoPin(echoPin) {}

void UltrasonicSensor::begin() {
    pinMode(mTrigPin, OUTPUT);
    pinMode(mEchoPin, INPUT);
    digitalWrite(mTrigPin, LOW);
    mSmoothedDistance = MAX_DISTANCE_CM;
}

void UltrasonicSensor::update() {
    uint32_t nowMs = millis();
    uint32_t nowUs = micros();

    switch (mState) {
        // -------------------------------------------------
        // IDLE: Wait for the 30Hz timer to expire
        // -------------------------------------------------
        case PingState::IDLE:
            if (nowMs - mLastPingTimeMs >= PING_INTERVAL_MS) {
                mState = PingState::TRIGGER_PULSE;
                
                // Start Trigger Pulse
                digitalWrite(mTrigPin, LOW);
                delayMicroseconds(2);
                digitalWrite(mTrigPin, HIGH);
                mTriggerTimeUs = nowUs; // Mark time to time the 10us pulse non-blocking
            }
            break;

        // -------------------------------------------------
        // TRIGGER_PULSE: Hold Trig HIGH for ~12us non-blocking
        // -------------------------------------------------
        case PingState::TRIGGER_PULSE:
            if (nowUs - mTriggerTimeUs >= 12) { 
                digitalWrite(mTrigPin, LOW);
                mState = PingState::WAIT_ECHO_START;
            }
            break;

        // -------------------------------------------------
        // WAIT_ECHO_START: Wait for Echo to go HIGH
        // -------------------------------------------------
        case PingState::WAIT_ECHO_START:
            if (digitalRead(mEchoPin) == HIGH) {
                mEchoStartTimeUs = nowUs;
                mState = PingState::WAIT_ECHO_END;
            } 
            // Timeout: If no echo starts within 1ms, sensor is clear or errored
            else if (nowUs - mTriggerTimeUs > 1000) { 
                processRawDistance(MAX_DISTANCE_CM);
                mState = PingState::IDLE;
                mLastPingTimeMs = nowMs;
            }
            break;

        // -------------------------------------------------
        // WAIT_ECHO_END: Measure the Echo pulse width
        // -------------------------------------------------
        case PingState::WAIT_ECHO_END:
            if (digitalRead(mEchoPin) == LOW) {
                uint32_t duration = nowUs - mEchoStartTimeUs;
                float distance = duration * 0.0343f / 2.0f;
                
                // Clamp to sensor's realistic boundaries (NO NAN!)
                if (distance < MIN_DISTANCE_CM) distance = MIN_DISTANCE_CM;
                if (distance > MAX_DISTANCE_CM) distance = MAX_DISTANCE_CM;

                processRawDistance(distance);
                
                mState = PingState::IDLE;
                mLastPingTimeMs = nowMs;
            } 
            // Timeout: Object is beyond 400cm or echo lost
            else if (nowUs - mEchoStartTimeUs > MAX_ECHO_TIME_US) {
                processRawDistance(MAX_DISTANCE_CM);
                mState = PingState::IDLE;
                mLastPingTimeMs = nowMs;
            }
            break;
    }
}

// -------------------------------------------------
// Filter Pipeline: Raw -> Rolling Median -> EMA
// -------------------------------------------------
void UltrasonicSensor::processRawDistance(float distance) {
    // 1. Insert new reading into the rolling time-window buffer
    mMedianBuffer[mBufferIndex] = distance;
    mBufferIndex = (mBufferIndex + 1) % MEDIAN_SIZE;

    // 2. Calculate Median (Spike Immunity)
    float median = calculateRollingMedian();

    // 3. Exponential Smoothing (Smooth Cornering / Physical alignment)
    // NO Jump Rejection! If the median jumps from 20cm to 200cm because the robot turned, 
    // we accept it immediately and smooth the output for the motor controller.
    mSmoothedDistance = (ALPHA * median) + ((1.0f - ALPHA) * mSmoothedDistance);
}

float UltrasonicSensor::calculateRollingMedian() {
    // Copy buffer to temporary array for sorting
    float temp[MEDIAN_SIZE];
    memcpy(temp, mMedianBuffer, sizeof(temp));

    // Insertion sort (fastest algorithm for tiny arrays of 5 elements)
    for (size_t i = 1; i < MEDIAN_SIZE; i++) {
        float key = temp[i];
        int j = i - 1;
        while (j >= 0 && temp[j] > key) {
            temp[j + 1] = temp[j];
            j--;
        }
        temp[j + 1] = key;
    }

    // Return the middle element
    return temp[MEDIAN_SIZE / 2];
}


// //===========================================
// // Outer Interface Method
// //===========================================

// float getFilteredDistance()
// {
//     float samples[5];

//     for(int i=0;i<5;i++)
//     {
//         samples[i] = readDistanceCM();
//         delay(3);
//     }

//     float median =
//         median5(samples);

//     float jumpRejected =
//         rejectJump(median);

//     float smoothed =
//         exponentialFilter(jumpRejected);

//     return smoothed;
// }

// //===========================================
// // Get Distance
// //===========================================
// float readDistanceCM()
// {
//     digitalWrite(TRIG_PIN, LOW);
//     delayMicroseconds(2);

//     digitalWrite(TRIG_PIN, HIGH);
//     delayMicroseconds(10);
//     digitalWrite(TRIG_PIN, LOW);

//     unsigned long duration =
//         pulseIn(
//             ECHO_PIN,
//             HIGH,
//             30000);

//     if(duration == 0)
//     {
//         return NAN;
//     }

//     float distance =
//         duration * 0.0343f / 2.0f;

//     if(distance < 2.0f)
//         return NAN;

//     if(distance > 400.0f)
//         return NAN;

//     return distance;
// }

// //===========================================
// // Filter
// //===========================================
// float median5(float values[5])
// {
//     for(int i=0;i<4;i++)
//     {
//         for(int j=i+1;j<5;j++)
//         {
//             if(values[j] < values[i])
//             {
//                 float tmp = values[i];
//                 values[i] = values[j];
//                 values[j] = tmp;
//             }
//         }
//     }

//     return values[2];
// }

// float rejectJump(float value)
// {
//     static float previous = 100.0f;

//     constexpr float MAX_JUMP_CM = 50.0f;

//     if(fabs(value - previous) > MAX_JUMP_CM)
//     {
//         return previous;
//     }

//     previous = value;

//     return value;
// }

// float exponentialFilter(float value)
// {
//     static float filtered = 100.0f;

//     constexpr float ALPHA = 0.25f;

//     filtered =
//         (ALPHA * value)
//         +
//         ((1.0f - ALPHA) * filtered);

//     return filtered;
}