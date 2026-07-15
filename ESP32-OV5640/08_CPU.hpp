#pragma once

/*
 * ============================================================
 * Module : 08_CPU
 * Purpose: CPU Diagnostics & Benchmark
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */

#include <Arduino.h>
#include <esp_chip_info.h>
#include "Utility.hpp"

void printCPUInfo()
{
    printSeparator();
    Serial.println("                08. CPU DIAGNOSTICS");
    printSeparator();
    Serial.println();

    //----------------------------------------------------------
    // Basic CPU Information
    //----------------------------------------------------------

    esp_chip_info_t chip;
    esp_chip_info(&chip);

    Serial.printf("CPU Cores           : %d\n", chip.cores);
    Serial.printf("CPU Frequency       : %d MHz\n", ESP.getCpuFreqMHz());

#if CONFIG_FREERTOS_UNICORE
    Serial.println("FreeRTOS Mode       : Single Core");
#else
    Serial.println("FreeRTOS Mode       : Dual Core");
#endif

    Serial.printf("Running Core        : %d\n", xPortGetCoreID());

    Serial.printf("FreeRTOS Tick Rate  : %d Hz\n",
                  configTICK_RATE_HZ);

    Serial.println();

    //----------------------------------------------------------
    // Health Check
    //----------------------------------------------------------

    bool pass = true;

    Serial.println("Health Check");

    if (chip.cores == 2)
        Serial.println("✅ Dual Core CPU");
    else
    {
        Serial.println("❌ Unexpected Core Count");
        pass = false;
    }

    if (ESP.getCpuFreqMHz() == 240)
    {
        Serial.println("✅ CPU Running at 240 MHz");
    }
    else if (ESP.getCpuFreqMHz() >= 160)
    {
        Serial.println("⚠ CPU Frequency Reduced");
    }
    else
    {
        Serial.println("❌ CPU Frequency Too Low");
        pass = false;
    }

    Serial.println();

    //----------------------------------------------------------
    // Integer Benchmark
    //----------------------------------------------------------

    Serial.println("CPU Benchmark");

    volatile uint32_t sum = 0;

    const uint32_t iterations = 10000000UL;

    uint32_t start = millis();

    for (uint32_t i = 0; i < iterations; i++)
    {
        sum += i;
    }

    uint32_t elapsed = millis() - start;

    Serial.printf("Iterations          : %lu\n", iterations);
    Serial.printf("Execution Time      : %lu ms\n", elapsed);

    double mops = (double)iterations / elapsed / 1000.0;

    Serial.printf("Performance         : %.2f MOPS\n", mops);

    Serial.printf("Checksum            : %lu\n", sum);

    Serial.println();

    //----------------------------------------------------------
    // Result
    //----------------------------------------------------------

    if (pass)
        Serial.println("✅ CPU TEST PASSED");
    else
        Serial.println("❌ CPU TEST FAILED");

    printSeparator();
}