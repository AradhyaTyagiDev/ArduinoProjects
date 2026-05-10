
#include <Arduino.h>

int led1 = 4;
int led2 = 5;
int led3 = 18;
int buzzer = 19;

// Musical notes
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440

// Twinkle Twinkle melody
int melody[] = {
    C4, C4, G4, G4, A4, A4, G4,
    F4, F4, E4, E4, D4, D4, C4,

    G4, G4, F4, F4, E4, E4, D4,
    G4, G4, F4, F4, E4, E4, D4,

    C4, C4, G4, G4, A4, A4, G4,
    F4, F4, E4, E4, D4, D4, C4};

int duration[] = {
    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,

    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,

    400, 400, 400, 400, 400, 400, 800,
    400, 400, 400, 400, 400, 400, 800};

int notes = sizeof(melody) / sizeof(melody[0]);

void playLED(int note)
{

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);

    // Low notes
    if (note <= D4)
    {
        digitalWrite(led1, HIGH);
    }
    // Medium notes
    else if (note <= G4)
    {
        digitalWrite(led2, HIGH);
    }
    // High notes
    else
    {
        digitalWrite(led3, HIGH);
    }
}

void setup()
{

    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(buzzer, OUTPUT);

    Serial.begin(115200);
    Serial.println("Twinkle Twinkle LED Dance 🎵");
}

void loop()
{

    for (int i = 0; i < notes; i++)
    {

        playLED(melody[i]);

        tone(buzzer, melody[i]);
        delay(duration[i]);

        noTone(buzzer);
        delay(50);
    }

    delay(2000);
}