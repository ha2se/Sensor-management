#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include "network.h"

static BLEAdvertising *advertising = nullptr;

static void writeInt16LE(uint8_t *destination, int16_t value)
{
    uint16_t bits = static_cast<uint16_t>(value);
    destination[0] = static_cast<uint8_t>(bits & 0xFF);
    destination[1] = static_cast<uint8_t>((bits >> 8) & 0xFF);
}

static void writeUInt16LE(uint8_t *destination, uint16_t value)
{
    destination[0] = static_cast<uint8_t>(value & 0xFF);
    destination[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void network_init()
{
    BLEDevice::init("SDI12-ESP32");
    advertising = BLEDevice::getAdvertising();

    Serial.print("BLE address: ");
    Serial.println(BLEDevice::getAddress().toString().c_str());
}

bool sendMeasurements(float ec, float ph, float temperature, float humidity)
{
    if (advertising == nullptr ||
        !isfinite(ec) || !isfinite(ph) ||
        !isfinite(temperature) || !isfinite(humidity) ||
        ec < 0.0f || ec > 65.535f ||
        ph < 0.0f || ph > 6553.5f ||
        temperature < -327.68f || temperature > 327.67f ||
        humidity < 0.0f || humidity > 655.35f) {
        return false;
    }

    uint8_t payload[22] = {
        0xFF, 0xFF, // Company identifier 0xFFFF, exposed by Bleak as the manufacturer-data key.
        0, 0, 0, 0, // Sensor counters
        0, 0,       // EC
        0, 0,       // pH
        0, 0,       // Air temperature
        0, 0,       // Air humidity
        0, 0,       // Soil 1 temperature
        0, 0,       // Soil 1 humidity
        0, 0,       // Soil 2 temperature
        0, 0        // Soil 2 humidity
    };
    writeUInt16LE(payload + 6, static_cast<uint16_t>(lroundf(ec * 1000.0f)));
    writeUInt16LE(payload + 8, static_cast<uint16_t>(lroundf(ph * 10.0f)));
    writeInt16LE(payload + 10, static_cast<int16_t>(lroundf(temperature * 100.0f)));
    writeUInt16LE(payload + 12, static_cast<uint16_t>(lroundf(humidity * 100.0f)));

    BLEAdvertisementData advertisementData;
    advertisementData.setFlags(0x06);
    advertisementData.setManufacturerData(
        std::string(reinterpret_cast<const char *>(payload), sizeof(payload)));

    advertising->stop();
    advertising->setAdvertisementData(advertisementData);
    advertising->start();
    return true;
}
