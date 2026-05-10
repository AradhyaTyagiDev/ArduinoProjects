/*
PROBLEM With THis code:
### How to make the robot automatically keep perfect straight direction?
Problem in real robots:
1. - Motors are **never identical**
2. - Motors are **never identical**
3. SOLUTION: Competition robots solve this with a **closed-loop correction algorithm**.
	1. Option 1: Use of encoder for the loop
	2. Option 2: **Closed-loop correction algorithm**
		1. we use a **software correction + adaptive balancing** approach.
			1. ✔ Speed limiting  
			2. ✔ Dead-zone (no joystick drift)  
			3. ✔ Smooth steering  
			4. ✔ **Motor auto-balance correction**  
			5. ✔ PWM ramping (smooth acceleration)  
			6. ✔ Competition-grade stability
*/

///SOLUTION OF ABOVE PROBLEMS:

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>

/* Motor Pins */

#define PWMA 4
#define AIN1 18
#define AIN2 5

#define PWMB 23
#define BIN1 22
#define BIN2 21

#define STBY 19

#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1

/* Speed limits */

#define MAX_SPEED 150
#define MAX_TURN 80
#define DEAD_ZONE 1

/* Motor correction */

float motorBalance = 1.00;   // auto correction
#define BALANCE_STEP 0.002

/* Smooth acceleration */

int currentLeft = 0;
int currentRight = 0;
#define RAMP_STEP 6


/* ================= Motor Driver ================= */

void applyMotor(int leftSpeed, int rightSpeed)
{
  bool forward = true;

  if(leftSpeed < 0 || rightSpeed < 0)
    forward = false;

  leftSpeed = abs(leftSpeed);
  rightSpeed = abs(rightSpeed);

  leftSpeed = constrain(leftSpeed,0,255);
  rightSpeed = constrain(rightSpeed,0,255);

  if(forward)
  {
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);

    digitalWrite(BIN1,HIGH);
    digitalWrite(BIN2,LOW);
  }
  else
  {
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,HIGH);

    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
  }

  ledcWrite(LEFT_CHANNEL,leftSpeed);
  ledcWrite(RIGHT_CHANNEL,rightSpeed);
}


/* ================= Smooth Motor Control ================= */

void drive(int targetLeft, int targetRight)
{
  /* smooth ramp */

  if(currentLeft < targetLeft) currentLeft += RAMP_STEP;
  if(currentLeft > targetLeft) currentLeft -= RAMP_STEP;

  if(currentRight < targetRight) currentRight += RAMP_STEP;
  if(currentRight > targetRight) currentRight -= RAMP_STEP;

  applyMotor(currentLeft,currentRight);
}


/* ================= Setup ================= */

void setup()
{
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);

  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);

  pinMode(STBY,OUTPUT);

  digitalWrite(STBY,HIGH);

  ledcSetup(LEFT_CHANNEL,1000,8);
  ledcAttachPin(PWMA,LEFT_CHANNEL);

  ledcSetup(RIGHT_CHANNEL,1000,8);
  ledcAttachPin(PWMB,RIGHT_CHANNEL);

  Serial.begin(115200);

  Dabble.begin("ESP32_ROBOT");
}


/* ================= Main Control ================= */

void loop()
{
  Dabble.processInput();

  int x = GamePad.getXaxisData();
  int y = GamePad.getYaxisData();

  /* dead zone */

  if(abs(x) <= DEAD_ZONE) x = 0;
  if(abs(y) <= DEAD_ZONE) y = 0;

  /* convert joystick */

  int speed = map(y,-7,7,-MAX_SPEED,MAX_SPEED);
  int turn  = map(x,-7,7,-MAX_TURN,MAX_TURN);

  int leftTarget  = speed + turn;
  int rightTarget = speed - turn;

  /* auto balance correction */

  leftTarget *= motorBalance;

  /* adaptive correction when moving straight */

  if(turn == 0 && speed != 0)
  {
    if(currentLeft > currentRight)
      motorBalance -= BALANCE_STEP;
    else if(currentRight > currentLeft)
      motorBalance += BALANCE_STEP;

    motorBalance = constrain(motorBalance,0.85,1.15);
  }

  drive(leftTarget,rightTarget);
}