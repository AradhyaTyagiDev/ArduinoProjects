#pragma once

/*
 * ============================================================
 * Module : 03_PSRAM
 * Purpose: External PSRAM Diagnostics
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */


#include <Arduino.h>
#include <esp_heap_caps.h>
#include "Utility.hpp"

void printPSRAMInfo()
{
    printSeparator();
    Serial.println("               03. PSRAM DIAGNOSTICS");
    printSeparator();

    Serial.println();

    //--------------------------------------------------------------------
    // Basic Information
    //--------------------------------------------------------------------

    bool psramAvailable = psramFound();

    Serial.printf("PSRAM Present        : %s\n",
                  psramAvailable ? "YES" : "NO");

    if (!psramAvailable)
    {
        Serial.println();
        Serial.println("❌ PSRAM NOT DETECTED");
        printSeparator();
        return;
    }

    uint32_t totalPSRAM = ESP.getPsramSize();
    uint32_t freePSRAM = ESP.getFreePsram();

    size_t largestBlock =
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);

    size_t totalFree =
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    Serial.printf("Total PSRAM          : %.2f MB\n",
                  totalPSRAM / 1024.0 / 1024.0);

    Serial.printf("Free PSRAM           : %.2f MB\n",
                  freePSRAM / 1024.0 / 1024.0);

    Serial.printf("Largest Free Block   : %.2f MB\n",
                  largestBlock / 1024.0 / 1024.0);

    float fragmentation =
        100.0f -
        ((float)largestBlock / (float)totalFree) * 100.0f;

    Serial.printf("Fragmentation        : %.2f %%\n",
                  fragmentation);

    //--------------------------------------------------------------------
    // Allocation Test
    //--------------------------------------------------------------------

    Serial.println();
    Serial.println("Allocation Test");

    const size_t TEST_SIZE = 1024 * 1024;   // 1 MB

    uint8_t* buffer =
        (uint8_t*)heap_caps_malloc(TEST_SIZE, MALLOC_CAP_SPIRAM);

    if (buffer == nullptr)
    {
        Serial.println("❌ Failed to allocate 1 MB");
        printSeparator();
        return;
    }

    Serial.println("✅ 1 MB Allocation Successful");

    //--------------------------------------------------------------------
    // Memory Pattern Test
    //--------------------------------------------------------------------

    Serial.println();
    Serial.println("Memory Integrity Test");

    for (size_t i = 0; i < TEST_SIZE; i++)
        buffer[i] = (uint8_t)(i & 0xFF);

    bool pass = true;

    for (size_t i = 0; i < TEST_SIZE; i++)
    {
        if (buffer[i] != (uint8_t)(i & 0xFF))
        {
            Serial.printf("❌ Memory Error at %u\n",
                          (unsigned)i);
            pass = false;
            break;
        }
    }

    if (pass)
        Serial.println("✅ Read/Write Verification Passed");

    free(buffer);

    //--------------------------------------------------------------------
    // Health Check
    //--------------------------------------------------------------------

    Serial.println();
    Serial.println("Health Check");

    if (totalPSRAM == (8 * 1024 * 1024))
        Serial.println("✅ PSRAM Size OK");
    else
        Serial.println("⚠ Unexpected PSRAM Size");

    if (fragmentation < 5.0f)
        Serial.println("✅ Memory Fragmentation Low");
    else if (fragmentation < 20.0f)
        Serial.println("⚠ Moderate Fragmentation");
    else
        Serial.println("❌ High Fragmentation");

    if (freePSRAM > (7 * 1024 * 1024))
        Serial.println("✅ Free Memory Excellent");
    else if (freePSRAM > (6 * 1024 * 1024))
        Serial.println("⚠ Free Memory Reduced");
    else
        Serial.println("❌ Low Free Memory");

    Serial.println();

    if (pass)
        Serial.println("✅ PSRAM TEST PASSED");
    else
        Serial.println("❌ PSRAM TEST FAILED");

    printSeparator();
}