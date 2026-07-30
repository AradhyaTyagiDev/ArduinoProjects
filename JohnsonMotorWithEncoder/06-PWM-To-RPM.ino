/*
 * ESP32 PWM -> RPM Test
 * Enter PWM (0-255)
 * Prints RPM every 100 ms
 */

#define PWMB       25
#define BIN1       26
#define BIN2       27
#define STBY       33

#define ENCODER_A  34
#define ENCODER_B  35

const float COUNTS_PER_REV = 9360.0;
const unsigned long SAMPLE_TIME = 100;   // ms

volatile long encoderCount = 0;
volatile uint8_t previousState = 0;

int currentPWM = 0;

//------------------------------------------------------------
// Encoder ISR
//------------------------------------------------------------
void IRAM_ATTR encoderISR()
{
    uint8_t A = digitalRead(ENCODER_A);
    uint8_t B = digitalRead(ENCODER_B);

    uint8_t currentState = (A << 1) | B;

    switch (previousState)
    {
        case 0:
            if(currentState == 2) encoderCount++;
            else if(currentState == 1) encoderCount--;
            break;

        case 1:
            if(currentState == 0) encoderCount++;
            else if(currentState == 3) encoderCount--;
            break;

        case 3:
            if(currentState == 1) encoderCount++;
            else if(currentState == 2) encoderCount--;
            break;

        case 2:
            if(currentState == 3) encoderCount++;
            else if(currentState == 0) encoderCount--;
            break;
    }

    previousState = currentState;
}

//------------------------------------------------------------

void setMotorPWM(int pwm)
{
    pwm = constrain(pwm, 0, 255);

    currentPWM = pwm;

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    analogWrite(PWMB, pwm);
}

void stopMotor()
{
    analogWrite(PWMB, 0);
}

//------------------------------------------------------------

void setup()
{
    Serial.begin(115200);

    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(STBY, OUTPUT);

    digitalWrite(STBY, HIGH);

    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);

    previousState =
        (digitalRead(ENCODER_A) << 1) |
         digitalRead(ENCODER_B);

    attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoderISR, CHANGE);

    Serial.println("-----------------------------------------");
    Serial.println("PWM -> RPM Test");
    Serial.println("Enter PWM (0-255)");
    Serial.println("-----------------------------------------");
}

//------------------------------------------------------------

void loop()
{
    // Read PWM from Serial
    if (Serial.available())
    {
        String s = Serial.readStringUntil('\n');
        s.trim();

        if (s.length())
        {
            int pwm = s.toInt();
            pwm = constrain(pwm, 0, 255);

            setMotorPWM(pwm);

            Serial.println();
            Serial.print("PWM Set To : ");
            Serial.println(pwm);
            Serial.println();
        }
    }

    // Calculate RPM every 100 ms
    static unsigned long lastTime = millis();
    static long lastCount = 0;

    if (millis() - lastTime >= SAMPLE_TIME)
    {
        long currentCount = encoderCount;

        long ticks = currentCount - lastCount;

        float ticksPerSecond = ticks * (1000.0 / SAMPLE_TIME);

        float rpm = (ticksPerSecond * 60.0) / COUNTS_PER_REV;

        Serial.print("PWM: ");
        Serial.print(currentPWM);

        Serial.print("\tTicks/sec: ");
        Serial.print(ticksPerSecond, 1);

        Serial.print("\tRPM: ");
        Serial.println(rpm, 2);

        lastCount = currentCount;
        lastTime += SAMPLE_TIME;
    }
}