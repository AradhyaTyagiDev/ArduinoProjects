#include "UltrasonicSensor.h"

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
        case PingState::IDLE:
            if (nowMs - mLastPingTimeMs >= PING_INTERVAL_MS) {
                // SAFETY CHECK: Only trigger if the Echo pin is currently LOW.
                // If it's stuck HIGH from interference, skip this ping cycle.
                if (digitalRead(mEchoPin) == LOW) {
                    mState = PingState::TRIGGER_PULSE;
                    digitalWrite(mTrigPin, LOW);
                    delayMicroseconds(2);
                    digitalWrite(mTrigPin, HIGH);
                    mTriggerTimeUs = nowUs; 
                }
            }
            break;

        case PingState::TRIGGER_PULSE:
            if (nowUs - mTriggerTimeUs >= 12) { 
                digitalWrite(mTrigPin, LOW);
                mState = PingState::WAIT_ECHO_START;
            }
            break;

        case PingState::WAIT_ECHO_START:
            if (digitalRead(mEchoPin) == HIGH) {
                mEchoStartTimeUs = nowUs;
                mState = PingState::WAIT_ECHO_END;
            } 
            // BUG FIX: Increased timeout from 1ms to 50ms (50000us).
            // Sound takes ~23ms to travel to 400cm and back. 
            // 1ms was causing it to instantly timeout and return 400cm!
            else if (nowUs - mTriggerTimeUs > 50000) { 
                processRawDistance(MAX_DISTANCE_CM);
                mState = PingState::IDLE;
                mLastPingTimeMs = nowMs;
            }
            break;

        case PingState::WAIT_ECHO_END:
            if (digitalRead(mEchoPin) == LOW) {
                uint32_t duration = nowUs - mEchoStartTimeUs;
                float distance = duration * 0.0343f / 2.0f;
                
                if (distance < MIN_DISTANCE_CM) distance = MIN_DISTANCE_CM;
                if (distance > MAX_DISTANCE_CM) distance = MAX_DISTANCE_CM;

                processRawDistance(distance);
                
                mState = PingState::IDLE;
                mLastPingTimeMs = nowMs;
            } 
            // Timeout check (object is too far or echo lost)
            else if (nowUs - mEchoStartTimeUs > MAX_ECHO_TIME_US) {
                processRawDistance(MAX_DISTANCE_CM);
                mState = PingState::IDLE;
                mLastPingTimeMs = nowMs;
            }
            break;
    }
}

void UltrasonicSensor::processRawDistance(float distance) {
    mMedianBuffer[mBufferIndex] = distance;
    mBufferIndex = (mBufferIndex + 1) % MEDIAN_SIZE;

    float median = calculateRollingMedian();

    mSmoothedDistance = (ALPHA * median) + ((1.0f - ALPHA) * mSmoothedDistance);
}

float UltrasonicSensor::calculateRollingMedian() {
    float temp[MEDIAN_SIZE];
    memcpy(temp, mMedianBuffer, sizeof(temp));

    for (size_t i = 1; i < MEDIAN_SIZE; i++) {
        float key = temp[i];
        int j = i - 1;
        while (j >= 0 && temp[j] > key) {
            temp[j + 1] = temp[j];
            j--;
        }
        temp[j + 1] = key;
    }

    return temp[MEDIAN_SIZE / 2];
}

