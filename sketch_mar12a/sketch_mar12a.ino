/***************************************************
 * ESP32 Joystick Music Player
 * PART 1
 ***************************************************/

#define JOYSTICK_Y 34

#define RED_LED    18
#define GREEN_LED  19
#define BUZZER     25

//----------------- Notes -----------------

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784

//--------------------------------------------------

enum Direction
{
    DIR_CENTER,
    DIR_UP,
    DIR_DOWN
};

Direction currentDirection = DIR_CENTER;
Direction lastDirection = DIR_CENTER;

//--------------------------------------------------
// Twinkle Twinkle Little Star
//--------------------------------------------------

const int twinkleNotes[] =
{
NOTE_C4,NOTE_C4,
NOTE_G4,NOTE_G4,
NOTE_A4,NOTE_A4,
NOTE_G4,

NOTE_F4,NOTE_F4,
NOTE_E4,NOTE_E4,
NOTE_D4,NOTE_D4,
NOTE_C4,

NOTE_G4,NOTE_G4,
NOTE_F4,NOTE_F4,
NOTE_E4,NOTE_E4,
NOTE_D4,

NOTE_G4,NOTE_G4,
NOTE_F4,NOTE_F4,
NOTE_E4,NOTE_E4,
NOTE_D4,

NOTE_C4,NOTE_C4,
NOTE_G4,NOTE_G4,
NOTE_A4,NOTE_A4,
NOTE_G4,

NOTE_F4,NOTE_F4,
NOTE_E4,NOTE_E4,
NOTE_D4,NOTE_D4,
NOTE_C4
};

const int twinkleDuration[] =
{
250,250,
250,250,
250,250,
500,

250,250,
250,250,
250,250,
500,

250,250,
250,250,
250,250,
500,

250,250,
250,250,
250,250,
500,

250,250,
250,250,
250,250,
500,

250,250,
250,250,
250,250,
700
};

const int TWINKLE_LENGTH =
sizeof(twinkleNotes)/sizeof(int);

//--------------------------------------------------
// Mary Had A Little Lamb
//--------------------------------------------------

const int maryNotes[] =
{
NOTE_E4,
NOTE_D4,
NOTE_C4,
NOTE_D4,
NOTE_E4,
NOTE_E4,
NOTE_E4,

NOTE_D4,
NOTE_D4,
NOTE_D4,

NOTE_E4,
NOTE_G4,
NOTE_G4,

NOTE_E4,
NOTE_D4,
NOTE_C4,
NOTE_D4,
NOTE_E4,
NOTE_E4,
NOTE_E4,

NOTE_E4,
NOTE_D4,
NOTE_D4,
NOTE_E4,
NOTE_D4,
NOTE_C4
};

const int maryDuration[] =
{
250,
250,
250,
250,
250,
250,
500,

250,
250,
500,

250,
250,
500,

250,
250,
250,
250,
250,
250,
250,

250,
250,
250,
250,
250,
700
};

const int MARY_LENGTH =
sizeof(maryNotes)/sizeof(int);

//--------------------------------------------------

const int *currentSong = NULL;
const int *currentTime = NULL;

int currentLength = 0;
int currentNote = 0;

bool playing = false;

unsigned long previousMillis = 0;

//--------------------------------------------------

void startSong(const int *song,
               const int *timeArray,
               int length)
{
    currentSong = song;
    currentTime = timeArray;
    currentLength = length;

    currentNote = 0;

    playing = true;

    previousMillis = millis();

    ledcWriteTone(BUZZER,currentSong[currentNote]);
}

//--------------------------------------------------

void stopSong()
{
    playing = false;

    currentSong = NULL;
    currentTime = NULL;

    currentLength = 0;

    currentNote = 0;

    ledcWriteTone(BUZZER,0);
}

//--------------------------------------------------

void updateSong()
{
    if (!playing)
        return;

    if (millis() - previousMillis >= currentTime[currentNote])
    {
        currentNote++;

        // Restart song automatically
        if (currentNote >= currentLength)
        {
            currentNote = 0;
        }

        previousMillis = millis();

        ledcWriteTone(BUZZER, currentSong[currentNote]);
    }
}

//--------------------------------------------------

Direction readJoystick()
{
    int y = analogRead(JOYSTICK_Y);

    Serial.println(y);

    if (y > 3950)
        return DIR_UP;

    if (y < 1200)
        return DIR_DOWN;

    return DIR_CENTER;
}

//--------------------------------------------------

void setup()
{
    Serial.begin(115200);

    analogSetPinAttenuation(JOYSTICK_Y,ADC_11db);

    pinMode(RED_LED,OUTPUT);
    pinMode(GREEN_LED,OUTPUT);

    ledcAttach(BUZZER,2000,8);

    digitalWrite(RED_LED,LOW);
    digitalWrite(GREEN_LED,LOW);

    // Welcome tone
    ledcWriteTone(BUZZER,523);
    delay(120);
    ledcWriteTone(BUZZER,659);
    delay(120);
    ledcWriteTone(BUZZER,784);
    delay(150);
    ledcWriteTone(BUZZER,0);
}

/***************************************************
 * PART 2
 ***************************************************/


void loop()
{
    currentDirection = readJoystick();

    // LEDs always reflect joystick position
    digitalWrite(RED_LED, currentDirection == DIR_DOWN);
    digitalWrite(GREEN_LED, currentDirection == DIR_UP);

    if (currentDirection == DIR_UP)
    {
        if (lastDirection != DIR_UP)
        {
            startSong(
                twinkleNotes,
                twinkleDuration,
                TWINKLE_LENGTH);

            lastDirection = DIR_UP;
        }
    }
    else if (currentDirection == DIR_DOWN)
    {
        if (lastDirection != DIR_DOWN)
        {
            startSong(
                maryNotes,
                maryDuration,
                MARY_LENGTH);

            lastDirection = DIR_DOWN;
        }
    }
    else
    {
        if (lastDirection != DIR_CENTER)
        {
            stopSong();
            lastDirection = DIR_CENTER;
        }
    }

    updateSong();
}