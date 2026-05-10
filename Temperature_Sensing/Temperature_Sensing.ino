#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

#define RED_LED 18
#define WHITE_LED 5
#define BUZZER 19

#define BUZZER_CHANNEL 0
#define BUZZER_FREQ 2000
#define BUZZER_RESOLUTION 8

DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  // pinMode(BUZZER, OUTPUT);

  dht.begin();

  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION);
  ledcAttachPin(BUZZER, BUZZER_CHANNEL);



  Serial.println("Temperature Alert System Started");
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

  // ===== VERY COLD MODE =====
  if (temperature < 20) {

    Serial.println("ICE MODE");

    // White LED ON
    digitalWrite(WHITE_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    // Soft cold beep
    ledcWriteTone(BUZZER_CHANNEL, 400);

    delay(200);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(800);
  }

  // ===== NORMAL MODE =====
  else if (temperature >= 20 && temperature < 35) {

    Serial.println("NORMAL MODE");

    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(1000);
  }

  // ===== WARNING MODE =====
  else if (temperature >= 35 && temperature < 50) {

    Serial.println("WARNING MODE");

    digitalWrite(RED_LED, HIGH);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 1200);

    delay(500);

    digitalWrite(RED_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 0);

    delay(500);
  }

  // ===== FIRE ALARM MODE =====
  else {

    Serial.println("FIRE ALARM MODE");

    // RED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(WHITE_LED, LOW);

    ledcWriteTone(BUZZER_CHANNEL, 2000);

    delay(300);

    // WHITE
    digitalWrite(RED_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);

    ledcWriteTone(BUZZER_CHANNEL, 1000);

    delay(300);
  }
}
