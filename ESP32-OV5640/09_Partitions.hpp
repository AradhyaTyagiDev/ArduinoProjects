#pragma once

/*
 * ============================================================
 * Module : 09_Partitions
 * Purpose: Flash Partition Diagnostics
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */

#include <Arduino.h>
#include <esp_partition.h>
#include "Utility.hpp"
#include <esp_ota_ops.h>

static const char* partitionTypeToString(esp_partition_type_t type)
{
    switch (type)
    {
        case ESP_PARTITION_TYPE_APP:
            return "Application";

        case ESP_PARTITION_TYPE_DATA:
            return "Data";

        default:
            return "Unknown";
    }
}

static const char* partitionSubtypeToString(uint8_t subtype)
{
    switch (subtype)
    {
        case ESP_PARTITION_SUBTYPE_APP_FACTORY:
            return "Factory";

        case ESP_PARTITION_SUBTYPE_APP_OTA_0:
            return "OTA_0";

        case ESP_PARTITION_SUBTYPE_APP_OTA_1:
            return "OTA_1";

        case ESP_PARTITION_SUBTYPE_APP_TEST:
            return "Test";

        default:
            return "Other";
    }
}

void printPartitionInfo()
{
    printSeparator();
    Serial.println("            09. PARTITION DIAGNOSTICS");
    printSeparator();
    Serial.println();

    const esp_partition_t *running =
        esp_ota_get_running_partition();

    bool pass = true;

    if (running == nullptr)
    {
        Serial.println("❌ Unable to determine running partition");
        printSeparator();
        return;
    }

    //----------------------------------------------------------
    // Running Partition
    //----------------------------------------------------------

    Serial.printf("Label               : %s\n",
                  running->label);

    Serial.printf("Type                : %s\n",
                  partitionTypeToString(running->type));

    Serial.printf("Subtype             : %s\n",
                  partitionSubtypeToString(running->subtype));

    Serial.printf("Address             : 0x%08X\n",
                  running->address);

    Serial.printf("Size                : %.2f MB\n",
                  running->size / 1024.0 / 1024.0);

    Serial.printf("Encrypted           : %s\n",
                  running->encrypted ? "YES" : "NO");

    Serial.println();

    //----------------------------------------------------------
    // Sketch Information
    //----------------------------------------------------------

    uint32_t sketchSize = ESP.getSketchSize();
    uint32_t freeSketch = ESP.getFreeSketchSpace();

    Serial.printf("Current Sketch      : %.2f KB\n",
                  sketchSize / 1024.0);

    Serial.printf("Free Sketch Space   : %.2f MB\n",
                  freeSketch / 1024.0 / 1024.0);

    Serial.println();

    //----------------------------------------------------------
    // OTA Support
    //----------------------------------------------------------

    bool otaSupported =
        (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0 ||
         running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1);

    Serial.print("OTA Support         : ");

    if (otaSupported)
    {
        Serial.println("YES");
    }
    else
    {
        Serial.println("NO (Factory Partition)");
    }

    Serial.println();

    //----------------------------------------------------------
    // Health Check
    //----------------------------------------------------------

    Serial.println("Health Check");

    if (running->size >= (2 * 1024 * 1024))
    {
        Serial.println("✅ Application Partition Size OK");
    }
    else
    {
        Serial.println("⚠ Application Partition is Small");
    }

    if (freeSketch > (1024UL * 1024UL))
    {
        Serial.println("✅ Sufficient Free Sketch Space");
    }
    else
    {
        Serial.println("⚠ Low Free Sketch Space");
    }

    if (running->type == ESP_PARTITION_TYPE_APP)
    {
        Serial.println("✅ Running from Application Partition");
    }
    else
    {
        Serial.println("❌ Invalid Running Partition");
        pass = false;
    }

    Serial.println();

    //----------------------------------------------------------
    // Result
    //----------------------------------------------------------

    if (pass)
    {
        Serial.println("✅ PARTITION TEST PASSED");
    }
    else
    {
        Serial.println("❌ PARTITION TEST FAILED");
    }

    printSeparator();
}