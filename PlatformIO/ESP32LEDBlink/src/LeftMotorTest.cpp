#define PWMB 5
#define BIN1 22
#define BIN2 23
#define STBY 4

int speedValue = 150;

void setup() {

  Serial.begin(115200);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);   // Enable driver

  Serial.println("Motor Test Ready");
  Serial.println("F = Forward");
  Serial.println("B = Backward");
  Serial.println("S = Stop");
  Serial.println("0-9 = Speed Control");
}

void loop() {

  if (Serial.available()) {

    char cmd = Serial.read();

    if (cmd == 'F') {
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      analogWrite(PWMB, speedValue);
      Serial.println("Forward");
    }

    if (cmd == 'B') {
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      analogWrite(PWMB, speedValue);
      Serial.println("Backward");
    }

    if (cmd == 'S') {
      analogWrite(PWMB, 0);
      Serial.println("Stop");
    }

    if (cmd >= '0' && cmd <= '9') {

      int level = cmd - '0';
      speedValue = map(level, 0, 9, 0, 255);

      analogWrite(PWMB, speedValue);

      Serial.print("Speed set to: ");
      Serial.println(speedValue);
    }
  }
}

/*
Key	Action
F	Forward
B	Backward
S	Stop
0–9	Speed control
*/