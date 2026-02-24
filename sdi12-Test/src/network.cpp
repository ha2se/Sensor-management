#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "network.h"

/* ================= CONFIG ================= */
const char *ssid = "iPhone de raed";
const char *password = "11223344";
const char *serverUrl = "http://172.20.10.2:5000/api/data";

void network_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("✅ ESP32 connecté au réseau :");
    Serial.print("SSID : ");
    Serial.println(ssid);
    Serial.print("IP ESP32 : ");
    Serial.println(WiFi.localIP());
}

bool sendTemperature(float temperature)
{
    char json[64];
    snprintf(json, sizeof(json),
             "{\"temperature\":%.2f}", temperature);

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST((uint8_t *)json, strlen(json));
    http.end();

    return code > 0;
}
