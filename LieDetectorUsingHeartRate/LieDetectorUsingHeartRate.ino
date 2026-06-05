// ==========================================
// ESP32 + KY039 Lie Detector Demo
// ==========================================

const int SENSOR_PIN = 34;

const int GREEN_LED = 18;
const int RED_LED   = 19;
const int BUZZER    = 23;

// Configuration
const int SAMPLE_DELAY  = 10;
const int BASELINE_TIME = 5000;   // 5 sec
const int TEST_TIME     = 3000;   // 3 sec

const float LIE_THRESHOLD = 16.0f;

float baseline = 0.0f;

// ==========================================
// Read Average Sensor Value
// ==========================================

float getAverage(unsigned long durationMs)
{
    unsigned long startTime = millis();

    long sum = 0;
    long count = 0;

    while ((millis() - startTime) < durationMs)
    {
        sum += analogRead(SENSOR_PIN);
        count++;

        delay(SAMPLE_DELAY);
    }

    if (count == 0)
        return 0;

    return (float)sum / count;
}

// ==========================================
// Truth Indicator
// ==========================================

void showTruth()
{
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
}

// ==========================================
// Lie Indicator
// ==========================================

void showLie()
{
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    for (int i = 0; i < 3; i++)
    {
        tone(BUZZER, 1000);

        delay(300);

        noTone(BUZZER);

        delay(200);
    }
}

// ==========================================
// Setup
// ==========================================

void setup()
{
    Serial.begin(115200);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    showTruth();

    Serial.println();
    Serial.println("================================");
    Serial.println("KY-039 Lie Detector");
    Serial.println("Place finger on sensor");
    Serial.println("================================");

    delay(2000);

    Serial.println();
    Serial.println("Measuring baseline...");

    baseline = getAverage(BASELINE_TIME);

    Serial.print("Baseline = ");
    Serial.println(baseline);

    Serial.println();
    Serial.println("Ask a question...");
    Serial.println("Waiting 5 seconds");

    delay(5000);
}

// ==========================================
// Main Loop
// ==========================================

void loop()
{
    float current = getAverage(TEST_TIME);

    float changePercent =
        abs((current - baseline) / baseline) * 100.0f;

    Serial.println("--------------------------------");

    Serial.print("Baseline : ");
    Serial.println(baseline);

    Serial.print("Current  : ");
    Serial.println(current);

    Serial.print("Change % : ");
    Serial.println(changePercent);

    if (changePercent > LIE_THRESHOLD)
    {
        Serial.println("POSSIBLE LIE");

        showLie();
    }
    else
    {
        Serial.println("TRUTH");

        showTruth();
    }

    // Slowly adapt baseline
    baseline = (baseline * 0.8f) + (current * 0.2f);

    Serial.print("New Baseline : ");
    Serial.println(baseline);

    delay(1000);
}