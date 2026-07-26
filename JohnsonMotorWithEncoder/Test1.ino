// /*
//  * ESP32 + TB6612 + Johnson Encoder Motor Test
//  * Prints:
//  *  Encoder Count
//  *  Direction
//  *  Pulses/sec
//  *  RPM
//  */

// #define PWMB 25
// #define BIN1 26
// #define BIN2 27
// #define STBY 33

// #define ENCODER_A 34
// #define ENCODER_B 35

// volatile long encoderCount = 0;

// volatile bool lastA = LOW;
// volatile bool lastB = LOW;

// unsigned long previousMillis = 0;
// long previousCount = 0;

// // CHANGE THIS if your encoder resolution differs.
// const float PULSES_PER_REV = 9360.0;

// void IRAM_ATTR encoderISR()
// {
//     bool A = digitalRead(ENCODER_A);
//     bool B = digitalRead(ENCODER_B);

//     if (A != lastA)
//     {
//         if (A == B)
//             encoderCount++;
//         else
//             encoderCount--;
//     }

//     lastA = A;
//     lastB = B;
// }

// void motorForward(uint8_t speedPWM)
// {
//     digitalWrite(BIN1, HIGH);
//     digitalWrite(BIN2, LOW);
//     analogWrite(PWMB, speedPWM);
// }

// void motorReverse(uint8_t speedPWM)
// {
//     digitalWrite(BIN1, LOW);
//     digitalWrite(BIN2, HIGH);
//     analogWrite(PWMB, speedPWM);
// }

// void motorStop()
// {
//     analogWrite(PWMB, 0);
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(PWMB, OUTPUT);
//     pinMode(BIN1, OUTPUT);
//     pinMode(BIN2, OUTPUT);
//     pinMode(STBY, OUTPUT);

//     digitalWrite(STBY, HIGH);

//     pinMode(ENCODER_A, INPUT_PULLUP);
//     pinMode(ENCODER_B, INPUT_PULLUP);

//     lastA = digitalRead(ENCODER_A);
//     lastB = digitalRead(ENCODER_B);

//     attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);

//     Serial.println();
//     Serial.println("Johnson Encoder Motor Test");
// }

// void loop()
// {
//     static int state = 0;
//     static unsigned long stateStart = millis();

//     switch(state)
//     {
//         case 0:
//             Serial.println("\nFORWARD");
//             motorForward(180);
//             state = 1;
//             stateStart = millis();
//             break;

//         case 1:
//             if(millis()-stateStart > 5000)
//             {
//                 motorStop();
//                 state = 2;
//                 stateStart = millis();
//             }
//             break;

//         case 2:
//             if(millis()-stateStart > 2000)
//             {
//                 Serial.println("\nREVERSE");
//                 motorReverse(180);
//                 state = 3;
//                 stateStart = millis();
//             }
//             break;

//         case 3:
//             if(millis()-stateStart > 5000)
//             {
//                 motorStop();
//                 state = 4;
//                 stateStart = millis();
//             }
//             break;

//         case 4:
//             if(millis()-stateStart > 2000)
//             {
//                 state = 0;
//             }
//             break;
//     }

//     if(millis() - previousMillis >= 1000)
//     {
//         long count = encoderCount;
//         long delta = count - previousCount;

//         float rpm = (delta * 60.0) / PULSES_PER_REV;

//         Serial.print("Count: ");
//         Serial.print(count);

//         Serial.print("   Delta: ");
//         Serial.print(delta);

//         Serial.print("   PPS: ");
//         Serial.print(delta);

//         Serial.print("   RPM: ");
//         Serial.println(rpm,3);

//         previousCount = count;
//         previousMillis = millis();
//     }
// }