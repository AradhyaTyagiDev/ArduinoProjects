// /*
//  * Computes the average RPM over the full 2-minute test 
//  * Measures every 100 ms and calculates Ticks/Second
//  This program:
//     Runs the motor continuously at PWM = 255
//     Measures encoder counts every 100 ms
//     Converts that value to ticks/second
//     Prints:
//       Ticks in the last 100 ms
//       Calculated ticks/second
//       Total encoder count

//       100ms: 1425	Ticks/sec: 14250	Total: 22977
//       100ms: 1427	Ticks/sec: 14270	Total: 24404
//       100ms: 1426	Ticks/sec: 14260	Total: 25830

//       RPM = (Ticks/sec × 60) / CountsPerRevolution

//       RPM = (14450 × 60) / 9360 ≈ 92.6 RPM (supply voltage may not be exactly 12 V)

//     ======================================
//         TEST COMPLETE
//     ======================================
//     Run Time              : 120.00 sec
//     PWM                   : 255
//     Encoder Counts/Rev    : 9360
//     Total Encoder Ticks   : 1734360
//     Average Ticks/Second  : 14453.00
//     Average RPM           : 92.65
//     ======================================
//  */

// /*
//  * ESP32 Motor Speed Characterization
//  * Runs motor for 2 minutes and calculates average RPM
//  */

// #define PWMB       25
// #define BIN1       26
// #define BIN2       27
// #define STBY       33

// #define ENCODER_A  34
// #define ENCODER_B  35

// //------------------------------------------------------------
// // Change this if your measured encoder CPR is different
// //------------------------------------------------------------
// const float COUNTS_PER_REV = 9360.0;

// //------------------------------------------------------------

// volatile long encoderCount = 0;
// volatile uint8_t previousState = 0;

// const int PWM = 255;

// const unsigned long SAMPLE_INTERVAL = 100;      // ms
// const unsigned long TEST_DURATION   = 120000;   // 2 minutes

// //------------------------------------------------------------
// // Encoder ISR
// //------------------------------------------------------------
// void IRAM_ATTR encoderISR()
// {
//     uint8_t A = digitalRead(ENCODER_A);
//     uint8_t B = digitalRead(ENCODER_B);

//     uint8_t currentState = (A << 1) | B;

//     switch (previousState)
//     {
//         case 0:
//             if(currentState == 2) encoderCount++;
//             else if(currentState == 1) encoderCount--;
//             break;

//         case 1:
//             if(currentState == 0) encoderCount++;
//             else if(currentState == 3) encoderCount--;
//             break;

//         case 3:
//             if(currentState == 1) encoderCount++;
//             else if(currentState == 2) encoderCount--;
//             break;

//         case 2:
//             if(currentState == 3) encoderCount++;
//             else if(currentState == 0) encoderCount--;
//             break;
//     }

//     previousState = currentState;
// }

// //------------------------------------------------------------

// void motorForward()
// {
//     digitalWrite(BIN1, HIGH);
//     digitalWrite(BIN2, LOW);
//     analogWrite(PWMB, PWM);
// }

// void motorStop()
// {
//     analogWrite(PWMB, 0);
// }

// //------------------------------------------------------------

// unsigned long startTime;
// unsigned long lastSampleTime;

// long lastCount = 0;

// bool testFinished = false;

// //------------------------------------------------------------

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(BIN1, OUTPUT);
//     pinMode(BIN2, OUTPUT);
//     pinMode(PWMB, OUTPUT);
//     pinMode(STBY, OUTPUT);

//     digitalWrite(STBY, HIGH);

//     pinMode(ENCODER_A, INPUT_PULLUP);
//     pinMode(ENCODER_B, INPUT_PULLUP);

//     previousState =
//         (digitalRead(ENCODER_A) << 1) |
//          digitalRead(ENCODER_B);

//     attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoderISR, CHANGE);

//     encoderCount = 0;

//     Serial.println("--------------------------------------");
//     Serial.println("2 Minute Motor Speed Test");
//     Serial.println("--------------------------------------");

//     motorForward();

//     startTime = millis();
//     lastSampleTime = startTime;
// }

// //------------------------------------------------------------

// void loop()
// {
//     if (testFinished)
//         return;

//     unsigned long now = millis();

//     //--------------------------------------------------------
//     // Print every 100 ms
//     //--------------------------------------------------------
//     if (now - lastSampleTime >= SAMPLE_INTERVAL)
//     {
//         long currentCount = encoderCount;

//         long ticks100ms = currentCount - lastCount;
//         long ticksPerSecond = ticks100ms * 10;

//         Serial.print("Interval ");
//         Serial.print((now - startTime) / 100);
//         Serial.print(" (100ms)");

//         Serial.print("\tTicks: ");
//         Serial.print(ticks100ms);

//         Serial.print("\tTicks/sec: ");
//         Serial.print(ticksPerSecond);

//         Serial.print("\tTotal: ");
//         Serial.println(currentCount);

//         lastCount = currentCount;
//         lastSampleTime += SAMPLE_INTERVAL;
//     }

//     //--------------------------------------------------------
//     // End after 2 minutes
//     //--------------------------------------------------------
//     if (now - startTime >= TEST_DURATION)
//     {
//         motorStop();

//         testFinished = true;

//         float totalTimeSeconds = (now - startTime) / 1000.0;

//         long totalTicks = encoderCount;

//         float averageTicksPerSecond =
//             totalTicks / totalTimeSeconds;

//         float rpm =
//             (averageTicksPerSecond * 60.0) / COUNTS_PER_REV;

//         Serial.println();
//         Serial.println("======================================");
//         Serial.println("          TEST COMPLETE");
//         Serial.println("======================================");

//         Serial.print("Run Time              : ");
//         Serial.print(totalTimeSeconds, 2);
//         Serial.println(" sec");

//         Serial.print("PWM                   : ");
//         Serial.println(PWM);

//         Serial.print("Encoder Counts/Rev    : ");
//         Serial.println(COUNTS_PER_REV, 0);

//         Serial.print("Total Encoder Ticks   : ");
//         Serial.println(totalTicks);

//         Serial.print("Average Ticks/Second  : ");
//         Serial.println(averageTicksPerSecond, 2);

//         Serial.print("Average RPM           : ");
//         Serial.println(rpm, 2);

//         Serial.println("======================================");
//     }
// }