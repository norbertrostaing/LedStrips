#include <Artnet.h>
#include <SPI.h>

void setupArtnet();

void onArtnetFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP);
