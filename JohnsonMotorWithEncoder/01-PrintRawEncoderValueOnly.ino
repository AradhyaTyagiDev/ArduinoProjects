// /*
//  * TEST: Rotate the motor shaft slowly and ESP32 Encoder will print raw values
//  *
//  * Yellow -> GPIO34 (Encoder A)
//  * White  -> GPIO35 (Encoder B)
//  * Blue   -> +5V
//  * Green  -> GND
//  */

// #define ENCODER_A 34
// #define ENCODER_B 35

// int lastA = -1;
// int lastB = -1;

// void setup() {
//   Serial.begin(115200);

//   pinMode(ENCODER_A, INPUT_PULLUP);
//   pinMode(ENCODER_B, INPUT_PULLUP);

//   Serial.println();
//   Serial.println("========== Encoder Raw Test ==========");
//   Serial.println("Rotate the motor shaft slowly.");
//   Serial.println("A\tB");
// }

// void loop() {

//   int A = digitalRead(ENCODER_A);
//   int B = digitalRead(ENCODER_B);

//   // Print only when either signal changes
//   if (A != lastA || B != lastB) {

//     Serial.print(A);
//     Serial.print("\t");
//     Serial.println(B);

//     lastA = A;
//     lastB = B;
//   }
// }