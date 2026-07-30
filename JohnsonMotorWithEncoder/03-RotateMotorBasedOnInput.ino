// /*
// * Accepts a signed number from the Serial Monitor (e.g. 100, -250, 9992)
// * Moves in the appropriate direction
// * Stops when the encoder reaches the requested count
// * Prints the current encoder value while moving
    
//     Simple Position Test
//     Send:
//       100
//       -50
//       9992

// ❌ However, expect overshoot until we add PWM ramping and PID. 
//         Motor can never stop exactly at 50 ticks with constant PWM = 80 because of inertia
//         At Lower PWM like 30~80     (Minor Error)
//             Target : 50 -> Stopped at: 51
//             Target : 50 -> Stopped: 41
//         At Higher PWM 255: (8 bit of error)
//           Target : 1000 -> Stopped at: 1008
//           Target : 20000 -> Stopped at: 20008

// ✅ SOLUTION: For exact positioning (±1 encoder tick), 
//       the next step is to implement deceleration (reduce PWM as the target approaches) or a PID position controller.
// */

// #define PWMB 25
// #define BIN1 26
// #define BIN2 27
// #define STBY 33

// #define ENCODER_A 34
// #define ENCODER_B 35

// volatile long encoderCount = 0;
// volatile uint8_t previousState = 0;

// const int PWM = 255;

// void IRAM_ATTR encoderISR()
// {
//     uint8_t A = digitalRead(ENCODER_A);
//     uint8_t B = digitalRead(ENCODER_B);

//     uint8_t current = (A << 1) | B;

//     switch (previousState)
//     {
//         case 0:
//             if(current==2) encoderCount++;
//             else if(current==1) encoderCount--;
//             break;

//         case 1:
//             if(current==0) encoderCount++;
//             else if(current==3) encoderCount--;
//             break;

//         case 3:
//             if(current==1) encoderCount++;
//             else if(current==2) encoderCount--;
//             break;

//         case 2:
//             if(current==3) encoderCount++;
//             else if(current==0) encoderCount--;
//             break;
//     }

//     previousState = current;
// }

// void motorForward()
// {
//     digitalWrite(BIN1,HIGH);
//     digitalWrite(BIN2,LOW);
//     analogWrite(PWMB,PWM);
// }

// void motorReverse()
// {
//     digitalWrite(BIN1,LOW);
//     digitalWrite(BIN2,HIGH);
//     analogWrite(PWMB,PWM);
// }

// void motorStop()
// {
//     analogWrite(PWMB,0);
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(BIN1,OUTPUT);
//     pinMode(BIN2,OUTPUT);
//     pinMode(PWMB,OUTPUT);
//     pinMode(STBY,OUTPUT);

//     digitalWrite(STBY,HIGH);

//     pinMode(ENCODER_A,INPUT_PULLUP);
//     pinMode(ENCODER_B,INPUT_PULLUP);

//     previousState =
//         (digitalRead(ENCODER_A)<<1) |
//          digitalRead(ENCODER_B);

//     attachInterrupt(digitalPinToInterrupt(ENCODER_A),encoderISR,CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENCODER_B),encoderISR,CHANGE);

//     Serial.println("Enter target position:");
// }

// void loop()
// {
//     if (!Serial.available())
//         return;

//     long moveTicks = Serial.parseInt();

//     while (Serial.available())
//         Serial.read();

//     long start = encoderCount;
//     long target = start + moveTicks;

//     Serial.print("Start : ");
//     Serial.println(start);

//     Serial.print("Target: ");
//     Serial.println(target);

//     if (moveTicks > 0)
//         motorForward();
//     else if (moveTicks < 0)
//         motorReverse();
//     else
//         return;

//     while (true)
//     {
//         long pos = encoderCount;

//         Serial.println(pos);

//         if (moveTicks > 0)
//         {
//             if (pos >= target)
//                 break;
//         }
//         else
//         {
//             if (pos <= target)
//                 break;
//         }

//         delay(1);
//     }

//     motorStop();

//     Serial.print("Stopped at : ");
//     Serial.println(encoderCount);
// }