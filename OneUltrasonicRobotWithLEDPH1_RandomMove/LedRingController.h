#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

//====================================================
// Hardware
//====================================================

#define LED_PIN 13
#define NUM_LEDS 12

Adafruit_NeoPixel ring(
  NUM_LEDS,
  LED_PIN,
  NEO_GRB + NEO_KHZ800);

//====================================================
// Globals
//====================================================

uint8_t wheelOffset = 0;

constexpr uint8_t BRIGHTNESS_SAFE          = 30;
constexpr uint8_t BRIGHTNESS_WARNING       = 50;
constexpr uint8_t BRIGHTNESS_DANGER        = 80;
constexpr uint8_t BRIGHTNESS_CRITICAL      = 120;
uint8_t currentLEDBrightness               = BRIGHTNESS_SAFE; //MAX: 255, MIN: 0

//====================================================
// Utility
//====================================================

void setLedIntensity(uint8_t intensity)
{
    ring.setBrightness(intensity);
}

uint32_t colorLerp(
  uint32_t color1,
  uint32_t color2,
  float t) {
  uint8_t r1 = (color1 >> 16) & 0xFF;
  uint8_t g1 = (color1 >> 8) & 0xFF;
  uint8_t b1 = color1 & 0xFF;

  uint8_t r2 = (color2 >> 16) & 0xFF;
  uint8_t g2 = (color2 >> 8) & 0xFF;
  uint8_t b2 = color2 & 0xFF;

  uint8_t r = r1 + ((r2 - r1) * t);
  uint8_t g = g1 + ((g2 - g1) * t);
  uint8_t b = b1 + ((b2 - b1) * t);

  return ring.Color(r, g, b);
}

//=====================================================
// Rainbow Animation for Clear Distance
//=====================================================
//Rotating full rainbow 🌈  Make the wheel have a bright head and fading tail
uint32_t calculateSpinnerSpeed(float distance) {
  distance = constrain(distance, 70.0f, 400.0f);

  return map(
    (long)distance,
    70,
    400,
    30,  // fast
    150  // slow
  );
}

void showRainbowSpinner(uint32_t stepIntervalMs) {
  static int head = 0;
  static uint32_t lastStepTime = 0;

  // Move only after interval expires
  if (millis() - lastStepTime >= stepIntervalMs) {
    lastStepTime = millis();
    head = (head + 1) % NUM_LEDS;
  }

  ring.clear();

  for (int offset = 0; offset < 6; offset++) {
    int index =
      (head - offset + NUM_LEDS) % NUM_LEDS;


   uint8_t brightness = max(int(BRIGHTNESS_SAFE), (int)currentLEDBrightness - (offset * 15));

    uint16_t hue =
      (head * 5000) + (offset * 3000);

    uint32_t color =
      ring.ColorHSV(
        hue,
        255,
        brightness);

    ring.setPixelColor(index, color);
  }

  ring.show();
}

//====================================================
// Rainbow Safe Animation
//====================================================

void showRainbowSpinnerColourful() {
  setLedIntensity(currentLEDBrightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t hue =
      (i * 256 / NUM_LEDS) + wheelOffset;

    ring.setPixelColor(
      i,
      ring.gamma32(
        ring.ColorHSV(
          hue * 256,
          255,
          255)));
  }

  ring.show();

  wheelOffset += 2;
}

//====================================================
// Threat Wheel
//====================================================

void showThreatWheel(
  uint32_t startColor,
  uint32_t endColor,
  uint8_t rotationSpeed) {
  setLedIntensity(currentLEDBrightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    int index =
      (i + wheelOffset) % NUM_LEDS;

    float t =
      (float)i / (NUM_LEDS - 1);

    ring.setPixelColor(
      index,
      colorLerp(
        startColor,
        endColor,
        t));
  }

  ring.show();

  wheelOffset += rotationSpeed;
}

//====================================================
// Emergency Flash
//====================================================

void showDangerFlash() {
  static bool state = false;
  static uint32_t lastToggle = 0;

  if (millis() - lastToggle > 100) {
    state = !state;
    lastToggle = millis();
  }

    uint32_t color =
    state
        ? ring.Color(currentLEDBrightness, 0, 0)
        : ring.Color(0, 0, 0);

  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, color);
  }

  ring.show();
}

void showExtremeDangerFlash() {
  static bool on = false;

  on = !on;

  uint32_t color =
    on
        ? ring.Color(currentLEDBrightness, 0, 0)
        : ring.Color(currentLEDBrightness, 0, 0);

  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, color);
  }

  ring.show();
}

//====================================================
// Distance Logic
//====================================================

void updateLedStatus(float distance) {
  if (distance >= 380) {
    showRainbowSpinnerColourful();
  }
  else if (distance >= 70) {
    uint32_t speed = calculateSpinnerSpeed(distance);

    Serial.printf(
      "Distance: %.1f cm | Spinner Speed: %lu ms\n",
      distance,
      speed);

    showRainbowSpinner(speed);
  } else if (distance > 50) {
    showThreatWheel(
      ring.Color(255, 255, 0),
      ring.Color(255, 120, 0),
      2);
  } else if (distance > 30) {
    // Orange -> Red
    showThreatWheel(
      ring.Color(255, 120, 0),
      ring.Color(255, 0, 0),
      4);
  } else if (distance > 10) {
    // Deep Red
    showThreatWheel(
      ring.Color(180, 0, 0),
      ring.Color(255, 0, 0),
      8);
  } else if (distance > 5) {
    showDangerFlash();
  } else {
    showExtremeDangerFlash();
  }
}
