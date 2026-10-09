#include <Arduino.h>
#include "sdi_sensor.h"
#include "rs485_sensor.h"
#include "network.h"

static const uint32_t MEASUREMENT_PAUSE_MS = 4000;
static uint32_t lastMeasurementEnd = 0;

static void measureAll()
{
  float ec = 0.0f;
  float ph = 0.0f;
  float humidity = 0.0f;
  float temperature = 0.0f;

  Serial.println();
  Serial.println("========== MESURES ==========");

  bool senOK = readSEN0603(ec, ph);
  bool tbsOK = readTBSMP02(humidity, temperature);

  if (senOK) {
    Serial.print("EC          : ");
    Serial.print(ec, 2);
    Serial.println(" mS/cm");
    Serial.print("pH          : ");
    Serial.println(ph, 1);
  } else {
    Serial.println("EC          : indisponible");
    Serial.println("pH          : indisponible");
  }

  if (tbsOK) {
    Serial.print("Humidite    : ");
    Serial.print(humidity, 2);
    Serial.println(" %");
    Serial.print("Temperature : ");
    Serial.print(temperature, 2);
    Serial.println(" deg C");
  } else {
    Serial.println("Humidite    : indisponible");
    Serial.println("Temperature : indisponible");
  }

  if (senOK && tbsOK) {
    if (!sendMeasurements(ec, ph, temperature, humidity)) {
      Serial.println("Erreur : mesures non diffusees en BLE.");
    } else {
      Serial.println("Mesures diffusees en BLE.");
    }
  } else {
    Serial.println("Diffusion BLE ignoree : lecture capteur incomplete.");
  }

  Serial.println("============================");
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  sdi12_init();
  rs485_init();
  network_init();
}

void loop()
{
  if (millis() - lastMeasurementEnd >= MEASUREMENT_PAUSE_MS) {
    measureAll();
    lastMeasurementEnd = millis();
  }

  delay(10);
}
