/*
 * 01_ChipInfo
 */

#include <WiFi.h>
#include <esp_chip_info.h>
#include <esp_system.h>
#include "01_ChipInfo.hpp"
#include "02_FlashInfo.hpp"
#include "03_PSRAM.hpp"
#include "06_MAC.hpp"
#include "08_CPU.hpp"
#include "09_Partitions.hpp"
#include "10_WiFi.hpp"
#include "11_Bluetooth.hpp"


void setup()
{
    Serial.begin(115200);

    delay(2000);

    printChipInfo();
    printFlashInfo();
    printPSRAMInfo();
    printMACInfo();
    printCPUInfo();
    printPartitionInfo();
    printWiFiInfo();
    printBluetoothInfo();

}

void loop()
{

}