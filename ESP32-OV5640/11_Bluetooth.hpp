#pragma once

/*
 * ============================================================
 * Module : 11_Bluetooth
 * Purpose: BLE Diagnostics
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "Utility.hpp"

void printBluetoothInfo()
{
    printSeparator();
    Serial.println("           11. BLUETOOTH DIAGNOSTICS");
    printSeparator();
    Serial.println();

    bool pass = true;

    const char* deviceName = "ESP32-S3-Diagnostics";

    //----------------------------------------------------------
    // BLE Initialization
    //----------------------------------------------------------

    Serial.println("[BLE Initialization]");

    try
    {
        BLEDevice::init(deviceName);
        Serial.println("Status              : SUCCESS");
    }
    catch (...)
    {
        Serial.println("Status              : FAILED");
        pass = false;
    }

    if (!pass)
    {
        Serial.println();
        Serial.println("❌ BLUETOOTH TEST FAILED");
        printSeparator();
        return;
    }

    //----------------------------------------------------------
    // Create BLE Server
    //----------------------------------------------------------

    Serial.println();
    Serial.println("[BLE Server]");

    BLEServer* pServer = BLEDevice::createServer();

    if (pServer == nullptr)
    {
        Serial.println("Server              : FAILED");
        pass = false;
    }
    else
    {
        Serial.println("Server              : CREATED");
    }

    //----------------------------------------------------------
    // Advertising
    //----------------------------------------------------------

    BLEAdvertising* pAdvertising = nullptr;

    if (pass)
    {
        Serial.println();
        Serial.println("[BLE Advertising]");

        pAdvertising = BLEDevice::getAdvertising();

        if (pAdvertising == nullptr)
        {
            Serial.println("Advertising         : FAILED");
            pass = false;
        }
        else
        {
            pAdvertising->start();

            Serial.println("Advertising         : STARTED");

            delay(1000);

            pAdvertising->stop();

            Serial.println("Advertising         : STOPPED");
        }
    }

    //----------------------------------------------------------
    // Device Information
    //----------------------------------------------------------

    Serial.println();
    Serial.println("[Device Information]");

    Serial.printf("Device Name         : %s\n", deviceName);

    Serial.printf("Chip Model          : %s\n",
                  ESP.getChipModel());

    Serial.printf("CPU Frequency       : %d MHz\n",
                  ESP.getCpuFreqMHz());

    //----------------------------------------------------------
    // Memory Check
    //----------------------------------------------------------

    Serial.println();
    Serial.println("[Memory]");

    Serial.printf("Free Heap           : %u KB\n",
                  ESP.getFreeHeap() / 1024);

    Serial.printf("Minimum Heap        : %u KB\n",
                  ESP.getMinFreeHeap() / 1024);

    //----------------------------------------------------------
    // Shutdown
    //----------------------------------------------------------

    Serial.println();
    Serial.println("[Shutdown]");

    BLEDevice::deinit(true);

    Serial.println("BLE Deinitialized   : YES");

    //----------------------------------------------------------
    // Result
    //----------------------------------------------------------

    Serial.println();

    if (pass)
    {
        Serial.println("✅ BLUETOOTH TEST PASSED");
    }
    else
    {
        Serial.println("❌ BLUETOOTH TEST FAILED");
    }

    printSeparator();
}