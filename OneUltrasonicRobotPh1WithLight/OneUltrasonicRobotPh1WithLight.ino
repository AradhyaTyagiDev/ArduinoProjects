#include "LedRingController.h"
#include "UltrasonicSensor.h"
#include <Arduino.h>

//==================================
// Setup
//==================================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  ring.begin();
  ring.clear();
  ring.show();

  setLedIntensity(currentLEDBrightness);

  Serial.println("LED Threat Animation Test");
}

//====================================================
// TEST MODE
//====================================================
void loop() {

  float distance = readFilteredDistance();

  Serial.printf(
        "Distance: %.1f cm\n",
        distance
    );

   updateLedStatus(distance);

  static uint32_t lastPrint = 0;
}


