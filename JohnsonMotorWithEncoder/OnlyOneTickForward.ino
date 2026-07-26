// #define PWMB 25
// #define BIN1 26
// #define BIN2 27
// #define STBY 33

// #define ENCODER_A 34
// #define ENCODER_B 35

// volatile long encoderCount = 0;
// volatile long startCount = 0;

// void IRAM_ATTR encoderISR() {
//   bool A = digitalRead(ENCODER_A);
//   bool B = digitalRead(ENCODER_B);

//   if (A == B)
//     encoderCount++;
//   else
//     encoderCount--;
// }

// void setup() {
//   Serial.begin(115200);

//   pinMode(PWMB, OUTPUT);
//   pinMode(BIN1, OUTPUT);
//   pinMode(BIN2, OUTPUT);
//   pinMode(STBY, OUTPUT);

//   pinMode(ENCODER_A, INPUT_PULLUP);
//   pinMode(ENCODER_B, INPUT_PULLUP);

//   attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);

//   digitalWrite(STBY, HIGH);

//   Serial.println("Starting...");
//   delay(1000);

//   startCount = encoderCount;
// }

// void loop() {

//   if (fabsf(encoderCount - startCount) >= 1) {
//     //Serial.print("-----------------------Final Start Count : ");
//     // Wait for one encoder count
//     analogWrite(PWMB, 0);

//     // Serial.print("Final Start Count : ");
//     // Serial.println(startCount);

//     // Serial.print("Final Count : ");
//     // Serial.println(encoderCount);

//     // Serial.print("Final Moved Counts: ");
//     // Serial.println(encoderCount - startCount);

//     // Serial.println("Done");
//   } else {
//     // Forward direction
//     digitalWrite(BIN1, HIGH);
//     digitalWrite(BIN2, LOW);

//     analogWrite(PWMB, 80);  // Low speed

//     Serial.print("Start Count : ");
//     Serial.println(startCount);

//     Serial.print("Final Count : ");
//     Serial.println(encoderCount);
//   }
// }