#include <Arduino.h>
#include <esp_dmx.h>

extern byte dmxData[];

void setupDMX();
void onDmxFrame();

extern int receivePin;