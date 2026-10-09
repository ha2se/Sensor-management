#ifndef NETWORK_H
#define NETWORK_H

void network_init();
bool sendMeasurements(float ec, float ph, float temperature, float humidity);

#endif
