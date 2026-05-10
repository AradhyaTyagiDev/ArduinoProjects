/**
ON/OFF Motor Fan as temperature moves up and down.

| TB6612FNG | ESP32   |
| --------- | ------- |
| PWMA      | GPIO 25 |
| AIN1      | GPIO 26 |
| AIN2      | GPIO 27 |
| STBY      | GPIO 14 |
| VCC       | 3.3V    |
| GND       | GND     |

| TB6612FNG | Motor        |
| --------- | ------------ |
| A01       | Motor wire 1 |
| A02       | Motor wire 2 |

| TB6612FNG | Connection              |
| --------- | ----------------------- |
| VM        | External 9V battery +   |
| GND       | Battery - AND ESP32 GND |

| Temperature | Motor Speed        |
| ----------- | ------------------ |
| <35°C       | OFF                |
| 35–40°C     | Slow               |
| 40–45°C     | Medium             |
| >45°C       | Full Speed + Alarm |
*/

#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

#define RED_LED 18
#define WHITE_LED 5
#define BUZZER 19

#define BUZZER_CHANNEL 0
#define BUZZER_FREQ 2000
#define BUZZER_RESOLUTION 8

#define FAN_PWM 25
#define FAN_IN1 26
#define FAN_IN2 27
#define FAN_STBY 14

#define FAN_CHANNEL 1
#define FAN_FREQ 25000
#define FAN_RESOLUTION 8

bool coolingMode = false;

unsigned long coolingStartTime = 0;

const unsigned long coolingDuration = 10000;  // 10 seconds

DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  // pinMode(BUZZER, OUTPUT);

  dht.begin();

  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION);
  ledcAttachPin(BUZZER, BUZZER_CHANNEL);

  // ===== TB6612FNG MOTOR SETUP =====
  pinMode(FAN_IN1, OUTPUT);
  pinMode(FAN_IN2, OUTPUT);
  pinMode(FAN_STBY, OUTPUT);

  digitalWrite(FAN_STBY, HIGH);

  // Motor Direction
  digitalWrite(FAN_IN1, HIGH);
  digitalWrite(FAN_IN2, LOW);

  // PWM Setup
  ledcSetup(FAN_CHANNEL, FAN_FREQ, FAN_RESOLUTION);
  ledcAttachPin(FAN_PWM, FAN_CHANNEL);

  // Motor OFF initially
  setFanSpeed(0);

  Serial.println("Temperature Alert System Started");
}

void setFanSpeed(int speed) {

  // speed range: 0 - 255
  ledcWrite(FAN_CHANNEL, speed);
}

void loop() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor Error");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C  ");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // ===== SMART FAN CONTROL =====

  if (temperature < 35) {

    setFanSpeed(0);

    Serial.println("FAN OFF");
  }

  else if (temperature >= 35 && temperature < 40) {

    setFanSpeed(80);

    Serial.println("FAN SLOW");
  }

  else if (temperature >= 40 && temperature < 45) {

    setFanSpeed(160);

    Serial.println("FAN MEDIUM");
  }

  else {

    setFanSpeed(255);

    Serial.println("FAN FULL SPEED");
  }

  // ===== FIRE MODE =====
  if (temperature >= 45) {

    coolingMode = true;
    coolingStartTime = millis();

    Serial.println("FIRE ALARM MODE");

    // RED LED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 2000);

    delay(300);

    // WHITE LED
    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);

    ledcWriteTone(BUZZER_CHANNEL, 1000);

    delay(300);
  }

  // ===== COOLING MODE =====
  else if (coolingMode && millis() - coolingStartTime < coolingDuration) {

    Serial.println("COOLING DOWN MODE");

    // Slow blinking
    digitalWrite(RED_LED, HIGH);
    digitalWrite(WHITE_LED, HIGH);

    ledcWriteTone(BUZZER_CHANNEL, 700);

    delay(200);

    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(800);
  }

  // ===== NORMAL MODE =====
  else {

    coolingMode = false;

    Serial.println("NORMAL MODE");

    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(1000);
  }
}
