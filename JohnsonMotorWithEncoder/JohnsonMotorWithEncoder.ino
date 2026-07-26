// /*
//   ESP32 + TB6612 + Johnson Encoder Test
//   START FROM RPM =0 AND GO TO RPM =255
// */

// #define PWMB 25
// #define BIN1 26
// #define BIN2 27
// #define STBY 33

// #define ENCODER_A 34
// #define ENCODER_B 35

// volatile long encoderCount = 0;

// void IRAM_ATTR encoderISR()
// {
//     bool A = digitalRead(ENCODER_A);
//     bool B = digitalRead(ENCODER_B);

//     if (A == B)
//         encoderCount++;
//     else
//         encoderCount--;
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(PWMB, OUTPUT);
//     pinMode(BIN1, OUTPUT);
//     pinMode(BIN2, OUTPUT);
//     pinMode(STBY, OUTPUT);

//     pinMode(ENCODER_A, INPUT_PULLUP);
//     pinMode(ENCODER_B, INPUT_PULLUP);

//     attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);

//     digitalWrite(STBY, HIGH);

//     digitalWrite(BIN1, HIGH);
//     digitalWrite(BIN2, LOW);

//     Serial.println();
//     Serial.println("========== Encoder Test ==========");
// }

// void loop()
// {
//     static int pwm = 40;
//     static unsigned long pwmTimer = millis();
//     static unsigned long printTimer = millis();

//     analogWrite(PWMB, pwm);

//     // Increase PWM every 2 seconds
//     if (millis() - pwmTimer >= 2000)
//     {
//         pwmTimer = millis();

//         if (pwm < 255)
//         {
//             pwm += 40;
//             if (pwm > 255)
//                 pwm = 255;

//             Serial.println();
//             Serial.print("PWM -> ");
//             Serial.println(pwm);
//         }
//     }

//     static long lastCount = 0;

//     // Print every 200 ms
//     if (millis() - printTimer >= 200)
//     {
//         printTimer = millis();

//         long count = encoderCount;
//         long delta = count - lastCount;

//         Serial.print("PWM:");
//         Serial.print(pwm);

//         Serial.print("  Count:");
//         Serial.print(count);

//         Serial.print("  Delta:");
//         Serial.println(delta);

//         lastCount = count;
//     }
// }