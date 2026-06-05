#pragma once

#include "RobotState.h"
#include "UltrasonicSensor.h"
#include "DistanceFilter.h"
#include "MotorController.h"

#include "RobotController.h"
#include "Config.h"

class RobotController
{
public:

    void begin();

    void update();

private:

    RobotState mState = RobotState::Initializing;

    MotorController mMotor;

    uint8_t mSearchAttempt = 0;

    float mDistance = 100;

    uint32_t mStateStartTime = 0;

    void transition(RobotState newState);

    ScheduledAction mAction;
};

void RobotController::begin()
{
    mMotor.begin();
    transition(RobotState::MovingForward);
}

void RobotController::transition(RobotState newState)
{
    mState = newState;
    mStateStartTime = millis();
}

void RobotController::update()
{
    mDistance =
        mFilter.update(
            mSensor.readDistanceCM());

    switch(mState)
    {
        case RobotState::MovingForward:
        {
            if(mDistance > 150)
            {
                mMotor.forward(255);
            }
            else if(mDistance > 90)
            {
                mMotor.forward(200);
            }
            else if(mDistance > 50)
            {
                mMotor.forward(150);
            }
            else if(mDistance > 25)
            {
                mMotor.forward(100);
            }
            else
            {
                transition(
                    RobotState::ObstacleDetected);
            }

            break;
        }

        case RobotState::ObstacleDetected:
        {
            mMotor.stop();

            transition(
                RobotState::SearchingPath);

            break;
        }

        case RobotState::SearchingPath:
        {
            static const uint16_t searchTime[] =
            {
                80,
                120,
                180,
                250,
                350,
                500
            };

            if(mSearchAttempt >= 6)
            {
                transition(
                    RobotState::StuckRecovery);

                break;
            }

            mMotor.rotateLeft(120);

            if(millis() - mStateStartTime >
                searchTime[mSearchAttempt])
            {
                mMotor.stop();

                if(mDistance > 40)
                {
                    mSearchAttempt = 0;

                    transition(
                        RobotState::MovingForward);
                }
                else
                {
                    mSearchAttempt++;

                    transition(
                        RobotState::SearchingPath);
                }
            }

            break;
        }

        case RobotState::StuckRecovery:
        {
            if(millis() - mStateStartTime < 800)
            {
                mMotor.reverse(150);
            }
            else if(millis() - mStateStartTime < 1800)
            {
                mMotor.rotateLeft(180);
            }
            else
            {
                mSearchAttempt = 0;

                transition(
                    RobotState::MovingForward);
            }

            break;
        }

        default:
            break;
    }
}


 // -------------------------------------------------- // -------------------------------------------------- // --------------------------------------------------
 ///How to use new New non-blocking Ultrasonic sensor
void loop() {
    // 1. Update sensor (Takes <1 microsecond, never blocks)
    sensor.update();

    // 2. Update motor timers (Takes <1 microsecond, never blocks)
    motor.update();

    // 3. If the motor is executing a turn/reverse, wait for it to finish
    if (motor.isBusy()) {
        return; 
    }

    // 4. Read the instant, pre-filtered distance (Takes nanoseconds)
    float distance = sensor.getDistance();

    // 5. State machine logic
    if (distance <= EMERGENCY_DISTANCE) {
        motor.rotateRight(45, 153);
    } else if (distance <= CLEAR_DISTANCE_MIN) {
        motor.forward(153);
    } else {
        motor.forward(255);
    }
}