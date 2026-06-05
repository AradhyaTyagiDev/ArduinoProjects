#include <Arduino.h>
#include "Config.h"
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "RobotController.h"

// -----------------------------------------------------------
// 1. Instantiate Hardware Drivers
// -----------------------------------------------------------
// No pin parameters needed, they are read directly from Config.h
MotorController motor;
UltrasonicSensor sensor(PIN_TRIG, PIN_ECHO); // Sensor still needs pins

// -----------------------------------------------------------
// 2. Instantiate the Orchestrator
// -----------------------------------------------------------
RobotController robot(motor, sensor);

// -----------------------------------------------------------
// Arduino Setup
// -----------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.println(F("Robot Initializing..."));

    robot.begin();

    Serial.println(F("Robot Ready. Autonomous mode active!"));
}

// -----------------------------------------------------------
// Arduino Loop
// -----------------------------------------------------------
void loop() {
    // 100% non-blocking. Runs thousands of times per second.
    robot.update();
}