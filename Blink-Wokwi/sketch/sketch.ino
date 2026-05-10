



// #define LDR_SENSOR_PIN 34
// #define LED_PIN 23
// #define BUZZER_PIN 19
// #define LASER_PIN 18

// int threshold = 2400;

// // PWM settings
// const int buzzerChannel = 0;
// const int frequency = 2000;
// const int resolution = 8;

// void setup() {

//   Serial.begin(115200);

//   pinMode(LED_PIN, OUTPUT);

//   pinMode(LASER_PIN, OUTPUT);
//   digitalWrite(LASER_PIN, HIGH);

//   // Setup PWM
//   ledcSetup(buzzerChannel, frequency, resolution);

//   // Attach buzzer pin
//   ledcAttachPin(BUZZER_PIN, buzzerChannel);
// }

// void loop() {

//   int sensorValue = analogRead(LDR_SENSOR_PIN);

//   Serial.print("Sensor Value: ");
//   Serial.println(sensorValue);

//   // LIGHT REMOVED / LASER BLOCKED
//   if(sensorValue < threshold) {

//     digitalWrite(LED_PIN, HIGH);

//     // Start buzzer
//     ledcWriteTone(buzzerChannel, 2000);

//   }
//   else {

//     digitalWrite(LED_PIN, LOW);

//     // Stop buzzer
//     ledcWriteTone(buzzerChannel, 0);
//   }

//   delay(10);
// }

// #define LDR_SENSOR_PIN 34
// #define LED_PIN 23
// #define BUZZER_PIN 19
// #define LASER_PIN 18

// int threshold = 2800;

// // PWM settings
// const int buzzerChannel = 0;
// const int frequency = 2000;
// const int resolution = 8;

// void setup() {

//   Serial.begin(115200);

//   pinMode(LED_PIN, OUTPUT);

//   pinMode(LASER_PIN, OUTPUT);
//   digitalWrite(LASER_PIN, HIGH);

//   // Setup PWM
//   ledcSetup(buzzerChannel, frequency, resolution);

//   // Attach buzzer pin
//   ledcAttachPin(BUZZER_PIN, buzzerChannel);
// }

// void loop() {

//   int sensorValue = analogRead(LDR_SENSOR_PIN);

//   Serial.print("Sensor Value: ");
//   Serial.println(sensorValue);

//   // Laser detected
//   if(sensorValue > threshold) {

//     digitalWrite(LED_PIN, HIGH);

//     // Play buzzer sound
//     ledcWriteTone(buzzerChannel, 2000);

//   }
//   else {

//     digitalWrite(LED_PIN, LOW);

//     // Stop buzzer
//     ledcWriteTone(buzzerChannel, 0);
//   }

//   delay(50);
// }




// #define LASER_PIN 21

// void setup() {
//   pinMode(LASER_PIN, OUTPUT);
// }

// void loop() {

//   // Laser ON
//   digitalWrite(LASER_PIN, HIGH);
//   delay(1000);

//   // Laser OFF
//   digitalWrite(LASER_PIN, LOW);
//   delay(1000);
// }


// #include <FastLED.h>

// #define LED_PIN     5
// #define NUM_LEDS    12
// #define BRIGHTNESS  80

// #define SPEAKER_PIN 21

// CRGB leds[NUM_LEDS];

// // Notes (frequency in Hz)
// #define C4  261
// #define D4  294
// #define E4  329
// #define F4  349
// #define G4  392
// #define A4  440

// // Twinkle Twinkle melody
// int melody[] = {
//   C4, C4, G4, G4, A4, A4, G4,
//   F4, F4, E4, E4, D4, D4, C4
// };

// int duration[] = {
//   500, 500, 500, 500, 500, 500, 1000,
//   500, 500, 500, 500, 500, 500, 1000
// };

// void setup() {
//   FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
//   FastLED.setBrightness(BRIGHTNESS);

//   ledcSetup(0, 2000, 8);
//   ledcAttachPin(SPEAKER_PIN, 0);
// }

// void loop() {
//   for (int i = 0; i < 14; i++) {

//     // 🎵 Play note
//     ledcWriteTone(0, melody[i]);

//     // 🌈 Twinkling LED effect
//     for (int j = 0; j < NUM_LEDS; j++) {
//       leds[j] = CHSV(random(0,255), 200, 255); // random sparkle
//     }

//     FastLED.show();
//     delay(duration[i]);

//     // turn off between notes
//     ledcWriteTone(0, 0);
//     delay(100);
//   }

//   delay(2000);
// }