const int SHOCK_PIN = 18;

void setup() {

  Serial.begin(115200);

  pinMode(SHOCK_PIN, INPUT);

}

void loop() {

  int value = digitalRead(SHOCK_PIN);

  Serial.println(value);

  delay(100);

}