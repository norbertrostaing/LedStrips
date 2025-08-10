#include <Arduino.h>

#include "_config.h"
#include "_ethernet.h"
#include "_wifi.h"
#include "_dmx.h"
#include "_artnet.h"
#include "_osc.h"
#include "_web.h"

#include "ledStrip.h"


void setup()
{
	// generic parameters
	oscPort = 9004;
	chipName = "LedStrips";
	eepromMarker = 0xBD;
	// config : read and write parameters
	// infos : read only parameters
	addEnumConfig("mode", "details");
	addEnumOption("mode", "rgb");
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
	addIntConfig("ledCount/9", 20);
	addIntConfig("ledCount/10", 20);
	addIntConfig("ledCount/11", 20);
	addIntConfig("ledCount/12", 20);
	addIntConfig("ledCount/13", 20);
	addIntConfig("ledCount/14", 20);
	addIntConfig("ledCount/15", 20);
	addIntConfig("ledCount/16", 20);

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
	if (key == "ledCount/9") {strips[8].nLeds = config["ledCount/9"].as<int>();}
	if (key == "ledCount/10") {strips[9].nLeds = config["ledCount/10"].as<int>();}
	if (key == "ledCount/11") {strips[10].nLeds = config["ledCount/11"].as<int>();}
	if (key == "ledCount/12") {strips[11].nLeds = config["ledCount/12"].as<int>();}
	if (key == "ledCount/13") {strips[12].nLeds = config["ledCount/13"].as<int>();}
	if (key == "ledCount/14") {strips[13].nLeds = config["ledCount/14"].as<int>();}
	if (key == "ledCount/15") {strips[14].nLeds = config["ledCount/15"].as<int>();}
	if (key == "ledCount/16") {strips[15].nLeds = config["ledCount/16"].as<int>();}
}



void onArtnetFrame(uint16_t universeRcv, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
	String mode = config["mode"].as<String>();
	int net = config["dmx/net"].as<int>();
	int subnet = config["dmx/subnet"].as<int>();
	int univ = config["dmx/universe"].as<int>();
	int extendedUniverse = (net << 8) | (subnet << 4) | univ;
	if (mode == "rgb") {
		computeRgbUniverse(data, -1, universeRcv-extendedUniverse);
		return;
	}
	if (universeRcv != extendedUniverse) return;
	computeStrips(data, -1);
}

void onDmxFrame()
{
	if (outputIsDirty) return;
	String mode = config["mode"].as<String>();
	if (mode == "rgb") {
		computeRgbUniverse(dmxData, 0, 0);
		return;
	}
	computeStrips(dmxData,0);
}


void triggerTriggered(String str) {

}