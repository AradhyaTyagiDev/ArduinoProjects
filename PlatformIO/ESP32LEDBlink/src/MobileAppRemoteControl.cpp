
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

#define MAX_SPEED 150     // limit motor speed
#define MAX_TURN 80       // limit steering
#define DEAD_ZONE 1       // joystick center tolerance

/* Motor calibration (adjust if robot drifts) */

#define LEFT_TRIM 0
#define RIGHT_TRIM 10

/* Motor Control */

void drive(int leftSpeed, int rightSpeed)
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

void loop()
{
  Dabble.processInput();

  int x = GamePad.getXaxisData();
  int y = GamePad.getYaxisData();

  /* Dead zone to stop drift */

  if(abs(x) <= DEAD_ZONE) x = 0;
  if(abs(y) <= DEAD_ZONE) y = 0;

  /* Speed and steering */

  int speed = map(y,-7,7,-MAX_SPEED,MAX_SPEED);
  int turn  = map(x,-7,7,-MAX_TURN,MAX_TURN);

  int leftMotor  = speed + turn + LEFT_TRIM;
  int rightMotor = speed - turn + RIGHT_TRIM;

  drive(leftMotor,rightMotor);
}

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