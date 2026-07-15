#pragma once

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <Arduino.h>

inline void printSeparator()
{
    Serial.println("============================================================");
}

inline void printModuleTitle(const char* title)
{
    printSeparator();
    Serial.println(title);
    printSeparator();
}

inline void printPass(const char* msg)
{
    Serial.print("✅ ");
    Serial.println(msg);
}

inline void printWarn(const char* msg)
{
    Serial.print("⚠ ");
    Serial.println(msg);
}

inline void printFail(const char* msg)
{
    Serial.print("❌ ");
    Serial.println(msg);
}

#endif  // UTILITY_HPP