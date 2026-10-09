#ifndef RS485_SENSOR_H
#define RS485_SENSOR_H

void rs485_init();
bool readSEN0603(float &ec, float &ph);

#endif