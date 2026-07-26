// /*
//  * TEST: Rotate the motor shaft slowly and ESP32 Encoder will print actual Tick from it's original position
//     +ve Value: Forward, -ve Value = Reverse
//  * ESP32 Quadrature Encoder State Machine
//  * Johnson Quad Encoder
//  */

// #define ENCODER_A 34
// #define ENCODER_B 35

// volatile long encoderCount = 0;

// volatile uint8_t previousState = 0;

// void IRAM_ATTR encoderISR()
// {
//     uint8_t A = digitalRead(ENCODER_A);
//     uint8_t B = digitalRead(ENCODER_B);

//     uint8_t currentState = (A << 1) | B;

//     switch (previousState)
//     {
//         case 0: // 00
//             if (currentState == 2)      // 00 -> 10
//                 encoderCount++;
//             else if (currentState == 1) // 00 -> 01
//                 encoderCount--;
//             break;

//         case 1: // 01
//             if (currentState == 0)      // 01 -> 00
//                 encoderCount++;
//             else if (currentState == 3) // 01 -> 11
//                 encoderCount--;
//             break;

//         case 3: // 11
//             if (currentState == 1)      // 11 -> 01
//                 encoderCount++;
//             else if (currentState == 2) // 11 -> 10
//                 encoderCount--;
//             break;

//         case 2: // 10
//             if (currentState == 3)      // 10 -> 11
//                 encoderCount++;
//             else if (currentState == 0) // 10 -> 00
//                 encoderCount--;
//             break;
//     }

//     previousState = currentState;
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(ENCODER_A, INPUT_PULLUP);
//     pinMode(ENCODER_B, INPUT_PULLUP);

//     uint8_t A = digitalRead(ENCODER_A);
//     uint8_t B = digitalRead(ENCODER_B);

//     previousState = (A << 1) | B;

//     attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoderISR, CHANGE);

//     Serial.println("Quadrature Encoder Test");
// }

// void loop()
// {
//     static long lastCount = 0;

//     if (encoderCount != lastCount)
//     {
//         Serial.print("Count : ");
//         Serial.println(encoderCount);

//         lastCount = encoderCount;
//     }
// }