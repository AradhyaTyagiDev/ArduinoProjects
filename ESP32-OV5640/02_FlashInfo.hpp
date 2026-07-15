#pragma once

/*
 * ============================================================
 * Module : 02_FlashInfo
 * Purpose: SPI Flash Diagnostics
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */

#include <Arduino.h>
#include "Utility.hpp"

static const char* flashModeToString(FlashMode_t mode)
{
    switch (mode)
    {
        case FM_QIO:  return "QIO";
        case FM_QOUT: return "QOUT";
        case FM_DIO:  return "DIO";
        case FM_DOUT: return "DOUT";
        case FM_FAST_READ: return "FAST_READ";
        case FM_SLOW_READ: return "SLOW_READ";
        default: return "UNKNOWN";
    }
}

static void printFlashInfo()
{
    printSeparator();
    Serial.println("               02. FLASH DIAGNOSTICS");
    printSeparator();

    uint32_t flashSize = ESP.getFlashChipSize();
    uint32_t flashSpeed = ESP.getFlashChipSpeed();
    FlashMode_t flashMode = ESP.getFlashChipMode();

    uint32_t sketchSize = ESP.getSketchSize();
    uint32_t freeSketch = ESP.getFreeSketchSpace();

    Serial.println();

    Serial.printf("Flash Size           : %.2f MB\n",
                  flashSize / 1024.0 / 1024.0);

    Serial.printf("Flash Speed          : %lu MHz\n",
                  flashSpeed / 1000000);

    Serial.printf("Flash Mode           : %s\n",
                  flashModeToString(flashMode));

    Serial.printf("Sketch Size          : %.2f KB\n",
                  sketchSize / 1024.0);

    Serial.printf("Free Sketch Space    : %.2f MB\n",
                  freeSketch / 1024.0 / 1024.0);

    Serial.println();

    Serial.println("Expected Configuration");

    Serial.println("Flash Size           : 16 MB");
    Serial.println("Flash Speed          : 80 MHz");
    Serial.println("Flash Mode           : QIO");

    Serial.println();

    bool pass = true;

    Serial.println("Health Check");

    if (flashSize != 16 * 1024 * 1024)
    {
        Serial.println("❌ Unexpected Flash Size");
        pass = false;
    }
    else
    {
        Serial.println("✅ Flash Size OK");
    }

    if (flashSpeed != 80000000)
    {
        Serial.println("⚠ Flash is not running at 80 MHz");
    }
    else
    {
        Serial.println("✅ Flash Speed OK");
    }

    if (flashMode != FM_QIO)
    {
        Serial.println("⚠ Flash is not using QIO mode");
    }
    else
    {
        Serial.println("✅ Flash Mode OK");
    }

    if (freeSketch < (1024UL * 1024UL))
    {
        Serial.println("⚠ Less than 1 MB free sketch space");
    }
    else
    {
        Serial.println("✅ Plenty of sketch space available");
    }

    Serial.println();

    if (pass)
        Serial.println("✅ FLASH TEST PASSED");
    else
        Serial.println("❌ FLASH TEST FAILED");

    printSeparator();
}