// /*
//  * ESP32 Position Controller (P Controller)
//  * TB6612FNG + Johnson Encoder Motor
//     1. Program that will take number of ticks as input and rotate motor upto a certain angle and print that angle as final
//     2. ✅ Takes encoder ticks as input from the Serial Monitor.
//     3. ✅ Supports both forward and reverse motion.
//     4. ✅ Runs at high speed when far from the target.
//     5. ✅ Automatically slows down as it approaches the target.
//     6. ✅ Stops cleanly at the target.
//     7. ✅ Prints requested ticks, actual ticks, error, and angle.
//     8. ✅ Relative position control (enter ±ticks)
//     9. ✅ **Automatically slows down near the target**
//     10. ✅ Stops when within ±2 ticks
//     11. ✅ Timeout protection (5 seconds)
//     12. ✅ No oscillation
//     13. ✅ Prints final ticks, error, and angle

//     22:37:57.311 -> ----------------------------
//     22:37:57.311 -> Requested : 9360
//     22:37:57.311 -> Actual    : 9392
//     22:37:57.311 -> Error     : -32
//     22:37:57.311 -> Angle     : 361.231 deg
//     22:37:57.311 -> Encoder   : 29276
//     22:37:57.311 -> ----------------------------
//  */

// #define PWMB 25
// #define BIN1 26
// #define BIN2 27
// #define STBY 33

// #define ENCODER_A 34
// #define ENCODER_B 35

// const float COUNTS_PER_REV = 9360.0;

// volatile long encoderCount = 0;
// volatile uint8_t previousState = 0;

// // ---------- Encoder ISR ----------
// void IRAM_ATTR encoderISR()
// {
//     uint8_t A = digitalRead(ENCODER_A);
//     uint8_t B = digitalRead(ENCODER_B);

//     uint8_t currentState = (A << 1) | B;

//     switch (previousState)
//     {
//         case 0:
//             if (currentState == 2) encoderCount++;
//             else if (currentState == 1) encoderCount--;
//             break;

//         case 1:
//             if (currentState == 0) encoderCount++;
//             else if (currentState == 3) encoderCount--;
//             break;

//         case 3:
//             if (currentState == 1) encoderCount++;
//             else if (currentState == 2) encoderCount--;
//             break;

//         case 2:
//             if (currentState == 3) encoderCount++;
//             else if (currentState == 0) encoderCount--;
//             break;
//     }

//     previousState = currentState;
// }

// // ---------- Motor ----------
// void driveMotor(int pwm)
// {
//     pwm = constrain(pwm, -255, 255);

//     if (pwm == 0)
//     {
//         analogWrite(PWMB, 0);
//         return;
//     }

//     if (pwm > 0)
//     {
//         digitalWrite(BIN1, HIGH);
//         digitalWrite(BIN2, LOW);
//         analogWrite(PWMB, pwm);
//     }
//     else
//     {
//         digitalWrite(BIN1, LOW);
//         digitalWrite(BIN2, HIGH);
//         analogWrite(PWMB, -pwm);
//     }
// }

// void brakeMotor()
// {
//     analogWrite(PWMB, 0);

//     // TB6612 short brake
//     digitalWrite(BIN1, HIGH);
//     digitalWrite(BIN2, HIGH);
// }

// // ---------- Setup ----------
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

//     Serial.println();
//     Serial.println("Enter relative encoder ticks:");
//     Serial.println("Example:");
//     Serial.println("9360");
//     Serial.println("-4680");
// }

// // ---------- Loop ----------
// void loop()
// {
//     if (!Serial.available())
//         return;

//     long moveTicks = Serial.parseInt();

//     while (Serial.available())
//         Serial.read();

//     long start = encoderCount;
//     long target = start + moveTicks;

//     Serial.print("\nMoving ");
//     Serial.print(moveTicks);
//     Serial.println(" ticks...");

//     unsigned long startTime = millis();

//     while (true)
//     {
//         long error = target - encoderCount;

//         if (abs(error) <= 2)
//             break;

//         // timeout
//         if (millis() - startTime > 5000)
//         {
//             Serial.println("Timeout!");
//             break;
//         }

//         // ---------- P Controller ----------
//         int pwm = abs(error) / 15;

//         pwm = constrain(pwm, 40, 255);

//         if (error > 0)
//             driveMotor(pwm);
//         else
//             driveMotor(-pwm);

//         delay(2);
//     }

//     brakeMotor();

//     delay(100);

//     long actualTicks = encoderCount - start;
//     long finalError = moveTicks - actualTicks;

//     float angle =
//         (float)actualTicks * 360.0 / COUNTS_PER_REV;

//     Serial.println();
//     Serial.println("----------------------------");
//     Serial.print("Requested : ");
//     Serial.println(moveTicks);

//     Serial.print("Actual    : ");
//     Serial.println(actualTicks);

//     Serial.print("Error     : ");
//     Serial.println(finalError);

//     Serial.print("Angle     : ");
//     Serial.print(angle, 3);
//     Serial.println(" deg");

//     Serial.print("Encoder   : ");
//     Serial.println(encoderCount);

//     Serial.println("----------------------------");
// }