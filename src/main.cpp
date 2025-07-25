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
	eepromMarker = 0xAD;
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
	loopLeds();
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

void computeRgbUniverse(uint8_t *data, int delta, int deltaUniverse) {
	int pixBuddy = config["dmx/rgbpixlink"].as<int>();
	int currentId = 0;
	// meh
}

void computeStrips(uint8_t *data, int delta) {
	long b = millis();
	int ad = config["dmx/address"].as<int>()+delta;
	int details = config["dmx/details"].as<int>();
	details = constrain(details, 0, 6);
	for (int s = 0; s < 8; s++)
	{
		if (ad+3 > 512) return;
		strips[s].r = data[ad];
		strips[s].g = data[ad + 1];
		strips[s].b = data[ad + 2];
		ad += 3;
		strips[s].dimmer = data[ad] / 255.0;
		ad += 1;
		for (int d = 0; d < details; d++)
		{
			if (ad+2 > 512) return;
			strips[s].details[d].r = data[ad];
			strips[s].details[d].g = data[ad + 1];
			strips[s].details[d].b = data[ad + 2];
			ad += 3;
			if (ad+6 > 512) return;
			strips[s].details[d].dimmer = data[ad]/255.0;
			strips[s].details[d].size = data[ad + 1]/255.0;
			strips[s].details[d].position = data[ad + 2]/255.0;
			strips[s].details[d].fadeDown = data[ad + 3]/255.0;
			strips[s].details[d].fadeUp = data[ad + 4]/255.0;
			strips[s].details[d].repetition = data[ad + 5]/255.0;
			ad += 6;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		strips[i].update();
	}
	long e = millis();
	//Serial.println(strips[0].r);
	//Serial.println(e-b);
	//Serial.println("end");
	outputIsDirty = true;
}



void onArtnetFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
	String mode = config["mode"].as<String>();
	int net = config["dmx/net"].as<int>();
	int subnet = config["dmx/subnet"].as<int>();
	int univ = config["dmx/universe"].as<int>();
	int extendedUniverse = (net << 8) | (subnet << 4) | univ;
	if (mode == "rgb") {
		computeRgbUniverse(dmxData, 0, 0);
		return;
	}
	if (universe != extendedUniverse) return;
	computeStrips(data, -1);
}

void onDmxFrame()
{
	// for (int i = 0; i< 10; i++) {
	// 	Serial.print(dmxData[i]);
	// 	Serial.print("\t");
	// }
	// Serial.println();
	String mode = config["mode"].as<String>();
	if (mode == "rgb") {
		computeRgbUniverse(dmxData, 0, 0);
		return;
	}
	computeStrips(dmxData,0);
}


void triggerTriggered(String str) {

}