#include <Arduino.h>
#include <ModbusMaster.h>
#include "rs485_sensor.h"

#define RXD2 4
#define TXD2 2
#define RS485_RTS 13
#define MODBUS_ADDR 1

static ModbusMaster node;

static void preTransmission()
{
  digitalWrite(RS485_RTS, HIGH);
  delayMicroseconds(2);
}

static void postTransmission()
{
  delayMicroseconds(2);
  digitalWrite(RS485_RTS, LOW);
}

void rs485_init()
{
  digitalWrite(RS485_RTS, LOW);
  pinMode(RS485_RTS, OUTPUT);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(MODBUS_ADDR, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Interface SEN0603 RS-485 initialisee.");
}

bool readSEN0603(float &ec, float &ph)
{
  while (Serial2.available()) {
    Serial2.read();
  }

  uint8_t result = node.readHoldingRegisters(0x0000, 4);
  if (result != node.ku8MBSuccess) {
    Serial.print("Erreur SEN0603 Modbus, code : ");
    Serial.println(result);
    return false;
  }

  ec = node.getResponseBuffer(2) / 1000.0f;
  ph = node.getResponseBuffer(3) / 10.0f;
  return true;
}