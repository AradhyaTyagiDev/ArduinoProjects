#include <Arduino.h>

void setup() {
  pinMode(4, OUTPUT);   // set pin 2 as output
  Serial.begin(115200);
Serial.println("Running...");
}

void loop() {
  digitalWrite(4, HIGH); // turn LED ON
  delay(1000);           // wait 1 second

  digitalWrite(4, LOW);  // turn LED OFF
  delay(1000);           // wait 1 second
}