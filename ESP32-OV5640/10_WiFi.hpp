#pragma once

/*
 * ============================================================
 * Module : 10_WiFi
 * Purpose: WiFi Diagnostics
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include "Utility.hpp"

//--------------------------------------------------------------
// Optional Credentials (Required only for Throughput Test)
//--------------------------------------------------------------

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

//--------------------------------------------------------------
// RSSI Quality
//--------------------------------------------------------------

static const char* rssiToQuality(int rssi)
{
    if (rssi >= -50) return "Excellent";
    if (rssi >= -60) return "Very Good";
    if (rssi >= -70) return "Good";
    if (rssi >= -80) return "Fair";
    return "Poor";
}

//--------------------------------------------------------------
// Hardware Test
//--------------------------------------------------------------

static bool testWiFiHardware()
{
    Serial.println("[1] WiFi Hardware");

    WiFi.mode(WIFI_STA);
    delay(300);

    String mac = WiFi.macAddress();

    Serial.printf("MAC Address         : %s\n", mac.c_str());

    bool pass = (mac != "00:00:00:00:00:00");

    if (pass)
        Serial.println("Status              : PASS");
    else
        Serial.println("Status              : FAIL");

    Serial.println();

    return pass;
}

//--------------------------------------------------------------
// Scan Test
//--------------------------------------------------------------

static void scanWiFi()
{
    Serial.println("[2] WiFi Scan");

    Serial.println("Scanning...");

    int count = WiFi.scanNetworks();

    Serial.printf("Networks Found      : %d\n\n", count);

    if (count == 0)
    {
        Serial.println("No WiFi Networks Found\n");
        return;
    }

    Serial.printf("%-3s %-32s %-8s %-8s %-10s\n",
                  "#",
                  "SSID",
                  "RSSI",
                  "CH",
                  "Quality");

    Serial.println(
    "--------------------------------------------------------------------------");

    for (int i = 0; i < count; i++)
    {
        Serial.printf("%-3d %-32s %-8d %-8d %-10s\n",
                      i + 1,
                      WiFi.SSID(i).c_str(),
                      WiFi.RSSI(i),
                      WiFi.channel(i),
                      rssiToQuality(WiFi.RSSI(i)));
    }

    WiFi.scanDelete();

    Serial.println();
}

//--------------------------------------------------------------
// Connection Test
//--------------------------------------------------------------

static bool connectWiFi()
{
    if (strlen(WIFI_SSID) == 0)
    {
        Serial.println("[3] Connection Test");
        Serial.println("Skipped (No Credentials)");
        Serial.println();

        return false;
    }

    Serial.println("[3] Connection Test");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 15000)
    {
        delay(250);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connection          : FAILED");
        Serial.println();

        return false;
    }

    Serial.println("Connection          : SUCCESS");

    Serial.printf("IP Address          : %s\n",
                  WiFi.localIP().toString().c_str());

    Serial.printf("Gateway             : %s\n",
                  WiFi.gatewayIP().toString().c_str());

    Serial.printf("RSSI                : %d dBm\n",
                  WiFi.RSSI());

    Serial.printf("Signal Quality      : %s\n",
                  rssiToQuality(WiFi.RSSI()));

    Serial.println();

    return true;
}

//--------------------------------------------------------------
// Simple Throughput Test
//--------------------------------------------------------------

static void throughputTest()
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    Serial.println("[4] Throughput Test");

    WiFiClient client;

    const char* host = "example.com";

    uint32_t start = millis();

    if (!client.connect(host, 80))
    {
        Serial.println("Unable to Connect");
        Serial.println();
        return;
    }

    client.println("GET / HTTP/1.1");
    client.println("Host: example.com");
    client.println("Connection: close");
    client.println();

    uint32_t bytes = 0;

    while (client.connected() || client.available())
    {
        while (client.available())
        {
            client.read();
            bytes++;
        }
    }

    uint32_t elapsed = millis() - start;

    double kbps = 0.0;

    if (elapsed > 0)
    {
        kbps = (bytes / 1024.0) / (elapsed / 1000.0);
    }

    Serial.printf("Downloaded          : %lu Bytes\n", bytes);
    Serial.printf("Time                : %lu ms\n", elapsed);
    Serial.printf("Approx Speed        : %.2f KB/s\n", kbps);

    Serial.println();
}

//--------------------------------------------------------------
// Main Function
//--------------------------------------------------------------

void printWiFiInfo()
{
    printSeparator();
    Serial.println("               10. WIFI DIAGNOSTICS");
    printSeparator();
    Serial.println();

    bool hw = testWiFiHardware();

    scanWiFi();

    bool connected = connectWiFi();

    if (connected)
        throughputTest();

    WiFi.disconnect(true);

    WiFi.mode(WIFI_OFF);

    if (hw)
        Serial.println("✅ WIFI TEST PASSED");
    else
        Serial.println("❌ WIFI TEST FAILED");

    printSeparator();
}