#pragma once

/*
 * ============================================================
 * Module : 06_MAC
 * Purpose: WiFi MAC Address Diagnostics
 * Compatible: Arduino ESP32 Core 3.x
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include "Utility.hpp"

static bool isValidMAC(const String &mac)
{
    if (mac.length() != 17)
        return false;

    if (mac == "00:00:00:00:00:00")
        return false;

    if (mac == "FF:FF:FF:FF:FF:FF")
        return false;

    return true;
}

void printMACInfo()
{
    printSeparator();
    Serial.println("                 06. MAC DIAGNOSTICS");
    printSeparator();
    Serial.println();

    bool pass = true;

    //----------------------------------------------------------
    // Station Interface
    //----------------------------------------------------------

    Serial.println("[Station Interface]");

    WiFi.mode(WIFI_STA);
    delay(300);

    String staMAC = WiFi.macAddress();

    Serial.printf("STA MAC             : %s\n", staMAC.c_str());

    if (isValidMAC(staMAC))
    {
        Serial.println("Status              : OK");
    }
    else
    {
        Serial.println("Status              : FAILED");
        pass = false;
    }

    WiFi.mode(WIFI_OFF);
    delay(200);

    Serial.println();

    //----------------------------------------------------------
    // SoftAP Interface
    //----------------------------------------------------------

    Serial.println("[SoftAP Interface]");

    WiFi.mode(WIFI_AP);

    delay(200);

    // Start temporary AP
    WiFi.softAP("ESP32-Diagnostics");

    delay(500);

    String apMAC = WiFi.softAPmacAddress();

    Serial.printf("AP MAC              : %s\n", apMAC.c_str());

    if (isValidMAC(apMAC))
    {
        Serial.println("Status              : OK");
    }
    else
    {
        Serial.println("Status              : FAILED");
        pass = false;
    }

    //----------------------------------------------------------
    // Vendor OUI
    //----------------------------------------------------------

    if (isValidMAC(staMAC))
    {
        Serial.println();

        Serial.print("Vendor OUI          : ");
        Serial.println(staMAC.substring(0, 8));
    }

    //----------------------------------------------------------
    // Compare
    //----------------------------------------------------------

    if (isValidMAC(staMAC) && isValidMAC(apMAC))
    {
        Serial.println();

        if (staMAC == apMAC)
        {
            Serial.println("Warning             : STA and AP MAC are identical");
        }
        else
        {
            Serial.println("MAC Relationship    : OK (Different Interfaces)");
        }
    }

    //----------------------------------------------------------
    // Cleanup
    //----------------------------------------------------------

    WiFi.softAPdisconnect(true);

    WiFi.mode(WIFI_OFF);

    //----------------------------------------------------------
    // Final Result
    //----------------------------------------------------------

    Serial.println();

    if (pass)
        Serial.println("✅ MAC TEST PASSED");
    else
        Serial.println("❌ MAC TEST FAILED");

    printSeparator();
}