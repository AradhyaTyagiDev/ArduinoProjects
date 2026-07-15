/*
 * 01_ChipInfo
 */
#pragma once

#include <WiFi.h>
#include <esp_chip_info.h>
#include <esp_system.h>
#include "Utility.hpp"

void printChipInfo()
{
    printSeparator();
    Serial.println("                 CHIP INFORMATION");
    printSeparator();

    esp_chip_info_t chip;
    esp_chip_info(&chip);

    Serial.printf("Chip Model             : %s\n", ESP.getChipModel());

    Serial.printf("Chip Revision          : %d\n",
                  ESP.getChipRevision());

    Serial.printf("CPU Cores              : %d\n",
                  chip.cores);

    Serial.printf("CPU Frequency          : %d MHz\n",
                  ESP.getCpuFreqMHz());

    Serial.printf("SDK Version            : %s\n",
                  ESP.getSdkVersion());

#ifdef ESP_ARDUINO_VERSION_MAJOR
    Serial.printf("Arduino Core           : %d.%d.%d\n",
                  ESP_ARDUINO_VERSION_MAJOR,
                  ESP_ARDUINO_VERSION_MINOR,
                  ESP_ARDUINO_VERSION_PATCH);
#endif

    Serial.print("Features               : ");

    if (chip.features & CHIP_FEATURE_WIFI_BGN)
        Serial.print("WiFi ");

    if (chip.features & CHIP_FEATURE_BLE)
        Serial.print("BLE ");

    if (chip.features & CHIP_FEATURE_BT)
        Serial.print("ClassicBT ");

    Serial.println();

    Serial.printf("Chip Package           : ESP32-S3\n");

    Serial.printf("Architecture           : Xtensa LX7 Dual Core\n");

    Serial.printf("Instruction Width      : 32-bit\n");

    Serial.printf("Target                 : %s\n", CONFIG_IDF_TARGET);

    Serial.printf("Endian                 : Little Endian\n");

    Serial.printf("Compile Date           : %s\n", __DATE__);

    Serial.printf("Compile Time           : %s\n", __TIME__);

    Serial.printf("Sketch Size            : %u Bytes\n",
                  ESP.getSketchSize());

    Serial.printf("Free Sketch Space      : %u Bytes\n",
                  ESP.getFreeSketchSpace());

    Serial.println();

    Serial.println("Health Check");

    bool pass = true;

    if (chip.cores != 2)
    {
        Serial.println("❌ Unexpected CPU Core Count");
        pass = false;
    }

    if (ESP.getCpuFreqMHz() != 240)
    {
        Serial.println("⚠ CPU is not running at 240 MHz");
    }

    if (ESP.getChipRevision() < 1)
    {
        Serial.println("⚠ Early Silicon Revision");
    }

    if (pass)
    {
        Serial.println("✅ CHIP TEST PASSED");
    }

    printSeparator();
}