#include <Arduino.h>
#include <stdio.h>
#include <string.h>

// ---------- TBSMP02 : SDI-12, UART1 ----------
#define SDI_TXD          22
#define SDI_RXD          23
#define SDI_TX_ENABLE    21
#define SDI_BREAK_ENABLE 19
#define SDI_ADDR         '0'

HardwareSerial SDI12Serial(1);

// ---------- Parametres ----------
const uint32_t RESPONSE_TIMEOUT_MS = 1500;

// ---------- Communication SDI-12 ----------
void sdi12_clearRx()
{
  while (SDI12Serial.available()) {
    SDI12Serial.read();
  }
}

void sdi12_break()
{
  digitalWrite(SDI_TX_ENABLE, HIGH);
  digitalWrite(SDI_BREAK_ENABLE, LOW);
  delay(20);

  digitalWrite(SDI_BREAK_ENABLE, HIGH);
  digitalWrite(SDI_TX_ENABLE, LOW);
  delay(10);
}

void sdi12_send(const char *command)
{
  sdi12_clearRx();
  sdi12_break();
  sdi12_clearRx();

  SDI12Serial.print(SDI_ADDR);
  SDI12Serial.print(command);
  SDI12Serial.flush();

  // Liberer le bus pour la reponse du capteur.
  digitalWrite(SDI_TX_ENABLE, HIGH);

  // Supprimer l'echo de notre commande.
  sdi12_clearRx();
}

size_t sdi12_readLine(char *buffer, size_t capacity)
{
  if (capacity == 0) {
    return 0;
  }

  size_t length = 0;
  bool overflow = false;
  uint32_t start = millis();

  buffer[0] = '\0';

  while (millis() - start < RESPONSE_TIMEOUT_MS) {
    while (SDI12Serial.available()) {
      int value = SDI12Serial.read();

      if (value < 0) {
        continue;
      }

      char c = (char)value;

      if (c == '\r') {
        continue;
      }

      if (c == '\n') {
        buffer[length] = '\0';
        return overflow ? 0 : length;
      }

      if (length < capacity - 1) {
        buffer[length++] = c;
      } else {
        overflow = true;
      }
    }

    delay(1);
  }

  buffer[length] = '\0';
  return 0;
}

bool parseMeasurementAck(const char *buffer, size_t length,
                         unsigned int &seconds,
                         unsigned int &valueCount)
{
  // Format : atttn
  // Exemple : 00012 = adresse 0, 1 seconde, 2 valeurs.
  if (length != 5 || buffer[0] != SDI_ADDR) {
    return false;
  }

  for (size_t i = 1; i < 5; ++i) {
    if (buffer[i] < '0' || buffer[i] > '9') {
      return false;
    }
  }

  seconds = (buffer[1] - '0') * 100
          + (buffer[2] - '0') * 10
          + (buffer[3] - '0');

  valueCount = buffer[4] - '0';
  return true;
}

// ---------- Lecture humidite / temperature ----------
bool readTBSMP02(float &humidity, float &temperature)
{
  char response[96] = {0};
  unsigned int seconds = 0;
  unsigned int valueCount = 0;

  sdi12_send("M1!");
  size_t length = sdi12_readLine(response, sizeof(response));

  if (!parseMeasurementAck(response, length, seconds, valueCount)) {
    Serial.print("Erreur TBSMP02 : reponse M1 invalide : ");
    Serial.println(length ? response : "(aucune)");
    return false;
  }

  if (valueCount != 2) {
    Serial.print("Erreur TBSMP02 : nombre de valeurs = ");
    Serial.println(valueCount);
    return false;
  }

  // Attendre la fin de conversion avant le BREAK de D0.
  delay((uint32_t)seconds * 1000UL + 100UL);

  sdi12_send("D0!");
  length = sdi12_readLine(response, sizeof(response));

  if (length <= 1 || response[0] != SDI_ADDR) {
    Serial.print("Erreur TBSMP02 : reponse D0 invalide : ");
    Serial.println(length ? response : "(aucune)");
    return false;
  }

  if (sscanf(response + 1, "%f%f", &humidity, &temperature) != 2) {
    Serial.print("Erreur TBSMP02 : donnees invalides : ");
    Serial.println(response);
    return false;
  }

  return true;
}

void sdi12_init()
{
  // SDI-12 : commencer avec les drivers desactives.
  digitalWrite(SDI_TX_ENABLE, HIGH);
  digitalWrite(SDI_BREAK_ENABLE, HIGH);
  pinMode(SDI_TX_ENABLE, OUTPUT);
  pinMode(SDI_BREAK_ENABLE, OUTPUT);

  // Configuration testee : TX inverse, RX normal.
  SDI12Serial.begin(1200, SERIAL_7E1,
                    SDI_RXD, SDI_TXD, true);
  SDI12Serial.setRxInvert(false);

  sdi12_clearRx();
  Serial.println("Interface TBSMP02 SDI-12 initialisee.");
}

