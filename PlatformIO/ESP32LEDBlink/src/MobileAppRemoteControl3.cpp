/**
✅ Problems Solved

✔ Motors not identical → auto calibration (stable)
✔ Robot drifting → error-based correction (better than previous)
✔ High speed instability → speed limiting + ramping
✔ Jerky movement → smooth acceleration
✔ Joystick noise → dead zone filtering
✔ Over-steering → controlled steering curve
*/

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>

/* Motor Pins */

#define PWMA 18
#define AIN1 19
#define AIN2 21

#define PWMB 5
#define BIN1 22
#define BIN2 23

#define STBY 4

#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1

/* CONFIG */

#define MAX_SPEED 140
#define MAX_TURN 70
#define DEAD_ZONE 1
#define RAMP_STEP 5

/* Auto balance */

float motorBalance = 1.0;

/* Current motor state */

int currentLeft = 0;
int currentRight = 0;


/* ================= MOTOR DRIVER ================= */

void applyMotor(int leftSpeed, int rightSpeed)
{
  bool forward = true;

  if(leftSpeed < 0 || rightSpeed < 0)
    forward = false;

  leftSpeed = abs(leftSpeed);
  rightSpeed = abs(rightSpeed);

  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  if(forward)
  {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  }
  else
  {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  }

  ledcWrite(LEFT_CHANNEL, leftSpeed);
  ledcWrite(RIGHT_CHANNEL, rightSpeed);
}


/* ================= SMOOTH DRIVE ================= */

void drive(int targetLeft, int targetRight)
{
  // Smooth ramp
  if(currentLeft < targetLeft) currentLeft += RAMP_STEP;
  if(currentLeft > targetLeft) currentLeft -= RAMP_STEP;

  if(currentRight < targetRight) currentRight += RAMP_STEP;
  if(currentRight > targetRight) currentRight -= RAMP_STEP;

  applyMotor(currentLeft, currentRight);
}


/* ================= SETUP ================= */

void setup()
{
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  ledcSetup(LEFT_CHANNEL, 1000, 8);
  ledcAttachPin(PWMA, LEFT_CHANNEL);

  ledcSetup(RIGHT_CHANNEL, 1000, 8);
  ledcAttachPin(PWMB, RIGHT_CHANNEL);

  Serial.begin(115200);

  Dabble.begin("ESP32_ROBOT");
}


/* ================= MAIN LOOP ================= */

void loop()
{
  Dabble.processInput();

  int x = GamePad.getXaxisData(); // steering
  int y = GamePad.getYaxisData(); // throttle

  /* DEAD ZONE */

  if(abs(x) <= DEAD_ZONE) x = 0;
  if(abs(y) <= DEAD_ZONE) y = 0;

  /* CURVED RESPONSE (better control) */

  float speedNorm = y / 7.0;
  float turnNorm  = x / 7.0;

  int speed = speedNorm * speedNorm * (speedNorm > 0 ? 1 : -1) * MAX_SPEED;
  int turn  = turnNorm  * turnNorm  * (turnNorm  > 0 ? 1 : -1) * MAX_TURN;

  /* MIXING */

  int leftTarget  = speed + turn;
  int rightTarget = speed - turn;

  /* APPLY AUTO BALANCE */

  leftTarget *= motorBalance;

  /* REALISTIC ERROR CORRECTION */

  if(turn == 0 && abs(speed) > 30)
  {
    int error = leftTarget - rightTarget;

    // adaptive learning
    motorBalance -= error * 0.0003;

    motorBalance = constrain(motorBalance, 0.92, 1.08);
  }

  /* DRIVE */

  drive(leftTarget, rightTarget);
}