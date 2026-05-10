/// 👉 Final Program for Mobile Controlled Robot

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>

/* Motor Pins */

/// For Zero PCB
#define PWMA 4
#define AIN1 18
#define AIN2 5

#define PWMB 23
#define BIN1 21
#define BIN2 22

#define STBY 19

/// For Zero PCB
// #define PWMA 4
// #define AIN1 17
// #define AIN2 16

// #define PWMB 21
// #define BIN1 18
// #define BIN2 19

// ///GPIO 5 (STBY) → ⚠️ Strapping pin. Must be HIGH during boot, If your motor driver pulls it LOW → ESP32 may not boot
// ///👉 Fix: Add a pull-up resistor (10kΩ) to GPIO 5
// #define STBY 5

#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1

/* Speed limits */

#define MAX_SPEED 250
#define MAX_TURN 80
#define DEAD_ZONE 1

/* Motor correction */

float motorBalance = 1.00; // auto correction
#define BALANCE_STEP 0.002

/* Smooth acceleration */

int currentLeft = 0;
int currentRight = 0;
#define RAMP_STEP 6

/* ================= Motor Driver ================= */

void applyMotor(int leftSpeed, int rightSpeed)
{
    bool forward = true;

    if (leftSpeed < 0 || rightSpeed < 0)
        forward = false;

    leftSpeed = abs(leftSpeed);
    rightSpeed = abs(rightSpeed);

    leftSpeed = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);

    if (forward)
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

/* ================= Smooth Motor Control ================= */

void drive(int targetLeft, int targetRight)
{
    /* smooth ramp */

    if (currentLeft < targetLeft)
        currentLeft += RAMP_STEP;
    if (currentLeft > targetLeft)
        currentLeft -= RAMP_STEP;

    if (currentRight < targetRight)
        currentRight += RAMP_STEP;
    if (currentRight > targetRight)
        currentRight -= RAMP_STEP;

    applyMotor(currentLeft, currentRight);
}

/* ================= Setup ================= */

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

/* ================= Main Control ================= */

void loop()
{
    Dabble.processInput();

    int x = GamePad.getXaxisData();
    int y = GamePad.getYaxisData();

    /* dead zone */

    if (abs(x) <= DEAD_ZONE)
        x = 0;
    if (abs(y) <= DEAD_ZONE)
        y = 0;

    /* convert joystick */

    int speed = map(y, -7, 7, -MAX_SPEED, MAX_SPEED);
    int turn = map(x, -7, 7, -MAX_TURN, MAX_TURN);

    int leftTarget = speed + turn;
    int rightTarget = speed - turn;

    /* auto balance correction */

    leftTarget *= motorBalance;

    /* adaptive correction when moving straight */

    if (turn == 0 && speed != 0)
    {
        if (currentLeft > currentRight)
            motorBalance -= BALANCE_STEP;
        else if (currentRight > currentLeft)
            motorBalance += BALANCE_STEP;

        motorBalance = constrain(motorBalance, 0.85, 1.15);
    }

    drive(leftTarget, rightTarget);
}