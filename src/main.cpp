#include <Arduino.h>

#include "Olgalimex/_config.h"
#include "Olgalimex/_ethernet.h"
#include "Olgalimex/_wifi.h"
#include "Olgalimex/_dmx.h"
#include "Olgalimex/_artnet.h"
#include "Olgalimex/_osc.h"
#include "Olgalimex/_web.h"

#include "ledStrip.h"


void setup()
{
	// generic parameters
	oscPort = 9004;
	chipName = "LedStrips";
	eepromMarker = 0xCD;
	// config : read and write parameters
	// infos : read only parameters
	addIntConfig("dmx/address", 1);
	addIntConfig("dmx/details", 2);
	addIntConfig("dmx/net", 0);
	addIntConfig("dmx/subnet", 0);
	addIntConfig("dmx/universe", 0);
	// addIntConfig("target/port", 0);
	addIntConfig("ledCount/1", 20);
	addIntConfig("ledCount/2", 20);
	addIntConfig("ledCount/3", 20);
	addIntConfig("ledCount/4", 20);
	addIntConfig("ledCount/5", 20);
	addIntConfig("ledCount/6", 20);
	addIntConfig("ledCount/7", 20);
	addIntConfig("ledCount/8", 20);

	Serial.begin(115200);

	initConfig();
	setupEthernet();
	setupWifi();
	setupOSC();
	setupWebServer();
	setupArtnet();
	receivePin = 36;
	setupDMX();
	setupLeds();

	OscWiFi.subscribe(oscPort, "/testSerial", [](const OscMessage &m) { Serial.println("test"); });
}

long lastTest = 0;

void loop()
{
	vTaskDelay(5);
	/*
	if (lastTest + 22 < millis()) {
		onDmxFrame();
		lastTest = millis();
	}
	*/
}

void configUpdatedMain(String key)
{
	if (key == "ledCount/1") {strips[0].nLeds = config["ledCount/1"].as<int>();}
	if (key == "ledCount/2") {strips[1].nLeds = config["ledCount/2"].as<int>();}
	if (key == "ledCount/3") {strips[2].nLeds = config["ledCount/3"].as<int>();}
	if (key == "ledCount/4") {strips[3].nLeds = config["ledCount/4"].as<int>();}
	if (key == "ledCount/5") {strips[4].nLeds = config["ledCount/5"].as<int>();}
	if (key == "ledCount/6") {strips[5].nLeds = config["ledCount/6"].as<int>();}
	if (key == "ledCount/7") {strips[6].nLeds = config["ledCount/7"].as<int>();}
	if (key == "ledCount/8") {strips[7].nLeds = config["ledCount/8"].as<int>();}
}



void onArtnetFrame(uint16_t universeRcv, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
	int net = config["dmx/net"].as<int>();
	int subnet = config["dmx/subnet"].as<int>();
	int univ = config["dmx/universe"].as<int>();
	int extendedUniverse = (net << 8) | (subnet << 4) | univ;
	if (universeRcv != extendedUniverse) return;
	computeStrips(data, -1);
}

void onDmxFrame()
{
	if (outputIsDirty) return;
	computeStrips(dmxData,0);
}


void triggerTriggered(String str) {

}