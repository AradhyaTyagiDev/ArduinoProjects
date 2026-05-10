#define PWMA 18
#define AIN1 19
#define AIN2 21
#define STBY 4

int speedValue = 150;

void setup() {

  Serial.begin(115200);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
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
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, speedValue);
      Serial.println("Forward");
    }

    if (cmd == 'B') {
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, speedValue);
      Serial.println("Backward");
    }

    if (cmd == 'S') {
      analogWrite(PWMA, 0);
      Serial.println("Stop");
    }

    if (cmd >= '0' && cmd <= '9') {

      int level = cmd - '0';
      speedValue = map(level, 0, 9, 0, 255);

      analogWrite(PWMA, speedValue);

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