#include <Arduino.h>
#include "sdi_sensor.h"
#include "network.h"

void setup()
{
  Serial.begin(115200);
  delay(1000);

  sdi12_init();
  network_init();
}

void loop()
{
  float temperature = readTemperature();

  if (temperature != -999.0f)
  {
    Serial.print("🌡 Température = ");
    Serial.println(temperature);
    sendTemperature(temperature);
  }
  else
  {
    Serial.println("❌ Erreur capteur");
  }

  delay(5000);
}
