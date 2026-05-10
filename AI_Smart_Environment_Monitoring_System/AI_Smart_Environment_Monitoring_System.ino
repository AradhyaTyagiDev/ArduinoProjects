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

#define BLYNK_TEMPLATE_ID "TMPL33caWwBPm"
#define BLYNK_TEMPLATE_NAME "AI SMART ENVIRONMENT MONITORING SYSTEM"
#define BLYNK_AUTH_TOKEN "DcK3m_QWqS5KCupZ1gxZHhXbOn7mQ4V-"

#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

enum class FanMode {
  Off,
  Slow,
  Medium,
  Full
};

enum class SystemMode {
  Normal,
  Cooling,
  Warning,
  Fire,
  Recovery
};

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

char ssid[] = "Angel_2G";
char pass[] = "R05168$#@";

bool coolingMode = false;

unsigned long coolingStartTime = 0;

const unsigned long coolingDuration = 10000;  // 10 seconds

float maxTemp = -100;
float minTemp = 100;

FanMode currentFanMode = FanMode::Off;
SystemMode currentSystemMode = SystemMode::Normal;

unsigned long lastSensorReadTime = 0;

bool fireAlertSent = false;

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

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Temperature Alert System Started");
}

void loop() {

  //This improves connection stability.
  Blynk.run();

  static float temperature = 0;
  static float humidity = 0;

  if (millis() - lastSensorReadTime >= 2000) {

    lastSensorReadTime = millis();

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

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
  }

  if (temperature > maxTemp) {
    maxTemp = temperature;
  }

  if (temperature < minTemp) {
    minTemp = temperature;
  }

  // ===== SMART FAN CONTROL =====

  if (temperature < 35) {

    currentFanMode = FanMode::Off;
    currentSystemMode = SystemMode::Normal;
  } else if (temperature >= 35 && temperature < 40) {

    currentFanMode = FanMode::Slow;
    currentSystemMode = SystemMode::Cooling;
  } else if (temperature >= 40 && temperature < 45) {

    currentFanMode = FanMode::Medium;
    currentSystemMode = SystemMode::Warning;
  } else {

    currentFanMode = FanMode::Full;
    currentSystemMode = SystemMode::Fire;
  }

  // Apply fan speed
  setFanSpeed(getFanPWM(currentFanMode));

  Serial.print("Fan Mode: ");
  Serial.println(getFanModeText(currentFanMode));

  Serial.print("System Mode: ");
  Serial.println(getSystemModeText(currentSystemMode));

  // ===== BLYNK UPDATES =====
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);

  Blynk.virtualWrite(V2, getSystemModeText(currentSystemMode));

  Blynk.virtualWrite(V3, maxTemp);
  Blynk.virtualWrite(V4, minTemp);

  Blynk.virtualWrite(V5, getFanPercent(currentFanMode));

  Blynk.virtualWrite(V6, currentFanMode != FanMode::Off ? 1 : 0);

  Blynk.virtualWrite(V7, currentSystemMode == SystemMode::Fire ? 1 : 0);

  // ===== FIRE MODE =====
  if (temperature >= 45) {

    if (!coolingMode) {
      coolingMode = true;
      coolingStartTime = millis();
    }

    Serial.println("FIRE ALARM MODE");

    currentSystemMode = SystemMode::Fire;

    if (!fireAlertSent) {

      Blynk.logEvent(
        "fire_alert",
        "🔥 Critical Temperature Detected!");

      fireAlertSent = true;
    }

    // ===== WEE =====

    digitalWrite(RED_LED, HIGH);
    digitalWrite(WHITE_LED, LOW);

    // Strong HIGH tone
    ledcWriteTone(BUZZER_CHANNEL, 2000);

    yield();
    delay(280);

    // FULL STOP removes humming
    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(50);

    // ===== WOO =====

    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);

    // Strong LOW tone
    ledcWriteTone(BUZZER_CHANNEL, 1800);

    yield();
    delay(250);

    // FULL STOP
    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(20);
  }

  // ===== COOLING MODE =====
  else if (coolingMode && millis() - coolingStartTime < coolingDuration) {

    Serial.println("COOLING DOWN MODE");

    currentSystemMode = SystemMode::Recovery;

    // Slow blinking
    digitalWrite(RED_LED, HIGH);
    digitalWrite(WHITE_LED, HIGH);

    ledcWriteTone(BUZZER_CHANNEL, 700);

    yield();
    Blynk.run();
    delay(200);

    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    yield();
    Blynk.run();
    delay(250);
  }
  // ===== NORMAL MODE =====
  else {

    coolingMode = false;
    fireAlertSent = false;

    Serial.println("NORMAL MODE");

    currentSystemMode = SystemMode::Normal;

    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    yield();
    Blynk.run();
    delay(300);
  }
}

void setFanSpeed(int speed) {

  // speed range: 0 - 255
  ledcWrite(FAN_CHANNEL, speed);
}

int getFanPWM(FanMode mode) {

  switch (mode) {

    case FanMode::Off:
      return 0;

    case FanMode::Slow:
      return 80;

    case FanMode::Medium:
      return 160;

    case FanMode::Full:
      return 255;
  }

  return 0;
}

int getFanPercent(FanMode mode) {

  switch (mode) {

    case FanMode::Off:
      return 0;

    case FanMode::Slow:
      return 30;

    case FanMode::Medium:
      return 60;

    case FanMode::Full:
      return 100;
  }

  return 0;
}

String getFanModeText(FanMode mode) {

  switch (mode) {

    case FanMode::Off:
      return "OFF";

    case FanMode::Slow:
      return "SLOW";

    case FanMode::Medium:
      return "MEDIUM";

    case FanMode::Full:
      return "FULL";
  }

  return "UNKNOWN";
}

String getSystemModeText(SystemMode mode) {

  switch (mode) {

    case SystemMode::Normal:
      return "NORMAL";

    case SystemMode::Cooling:
      return "COOLING";

    case SystemMode::Warning:
      return "WARNING";

    case SystemMode::Fire:
      return "FIRE MODE";

    case SystemMode::Recovery:
      return "RECOVERY";
  }

  return "UNKNOWN";
}
