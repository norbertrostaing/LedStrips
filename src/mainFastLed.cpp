/*
#include <Arduino.h>

#include "_config.h"
#include "_ethernet.h"
#include "_wifi.h"
#include "_dmx.h"
#include "_artnet.h"
#include "_osc.h"
#include "_web.h"

#include "ledStrip.h"

#define MAX_LEDS 120

// Define the array of leds
CRGB leds1[MAX_LEDS];
CRGB leds2[MAX_LEDS];
CRGB leds3[MAX_LEDS];
CRGB leds4[MAX_LEDS];
CRGB leds5[MAX_LEDS];
CRGB leds6[MAX_LEDS];
CRGB leds7[MAX_LEDS];
CRGB leds8[MAX_LEDS];

std::vector<ledStripMain> strips;

bool outputIsDirty = true;
unsigned long lastUpdate = 0;

void setup()
{
	// generic parameters
	oscPort = 9004;
	chipName = "LedStrips";

	// config : read and write parameters
	// infos : read only parameters
	addIntConfig("dmx/address", 1);
	addIntConfig("dmx/details", 2);
	addBoolConfig("dmx/allStripsSameColor", false);
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
	// setupArtnet();
	receivePin = 36;
	setupDMX();

	FastLED.addLeds<NEOPIXEL, 15>(leds1, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 4>(leds2, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 16>(leds3, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 17>(leds4, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 5>(leds5, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 18>(leds6, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 21>(leds7, MAX_LEDS); // GRB ordering is assumed
	FastLED.addLeds<NEOPIXEL, 22>(leds8, MAX_LEDS); // GRB ordering is assumed

	for (int i = 0; i < 8; i++)
	{
		strips.emplace_back();
		strips[i].nLeds = config["ledCount/" + String(i + 1)].as<int>();
	}

	strips[0].data = leds1;
	strips[1].data = leds2;
	strips[2].data = leds3;
	strips[3].data = leds4;
	strips[4].data = leds5;
	strips[5].data = leds6;
	strips[6].data = leds7;
	strips[7].data = leds8;

	OscWiFi.subscribe(oscPort, "/testSerial", [](const OscMessage &m) { Serial.println("test"); });
}

void loop()
{
	if (outputIsDirty) {
		FastLED.show();
		//lastUpdate = millis();
		//Serial.println("led update :"+String(millis()-lastUpdate));
		outputIsDirty = false;
	}
	vTaskDelay(5);
	// put your main code here, to run repeatedly:
	// Turn the LED on, then pause
	// Serial.println("ok");
	// for (int i = 0; i< 18; i++) {
	//   leds1[i] = CRGB::Black; leds1[(i+1)%18] = CRGB::Red;
	//   leds2[i] = CRGB::Black; leds2[(i+1)%18] = CRGB::Red;
	//   leds3[i] = CRGB::Black; leds3[(i+1)%18] = CRGB::Red;
	//   leds4[i] = CRGB::Black; leds4[(i+1)%18] = CRGB::Red;
	//   leds5[i] = CRGB::Black; leds5[(i+1)%18] = CRGB::Red;
	//   leds6[i] = CRGB::Black; leds6[(i+1)%18] = CRGB::Red;
	//   leds7[i] = CRGB::Black; leds7[(i+1)%18] = CRGB::Red;
	//   leds8[i] = CRGB::Black; leds8[(i+1)%18] = CRGB::Red;
	//   FastLED.show();
	// 	delay(50);
	// }
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

void onArtnetFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
	Serial.println("received universe " + String(universe));
}

void onDmxFrame()
{
	long b = millis();
	int ad = config["dmx/address"].as<int>();
	int details = config["dmx/details"].as<int>();
	details = constrain(details, 0, 6);
	bool allStripsSameColor = config["dmx/allStripsSameColor"].as<bool>();
	for (int s = 0; s < 8; s++)
	{
		if (ad+3 > 512) return;
		if (allStripsSameColor && s > 0)
		{
			strips[s].r = strips[s - 1].r;
			strips[s].g = strips[s - 1].g;
			strips[s].b = strips[s - 1].b;
		}
		else
		{
			strips[s].r = dmxData[ad];
			strips[s].g = dmxData[ad + 1];
			strips[s].b = dmxData[ad + 2];
			ad += 3;
		}
		strips[s].dimmer = dmxData[ad] / 255.0;
		ad += 1;
		for (int d = 0; d < details; d++)
		{
			if (allStripsSameColor && s > 0)
			{
				strips[s].details[d].r = strips[s - 1].details[d].r;
				strips[s].details[d].g = strips[s - 1].details[d].g;
				strips[s].details[d].b = strips[s - 1].details[d].b;
			}
			else 
			{
				if (ad+2 > 512) return;
				strips[s].details[d].r = dmxData[ad];
				strips[s].details[d].g = dmxData[ad + 1];
				strips[s].details[d].b = dmxData[ad + 2];
				ad += 3;
			}
			if (ad+6 > 512) return;
			strips[s].details[d].dimmer = dmxData[ad]/255.0;
			strips[s].details[d].size = dmxData[ad + 1]/255.0;
			strips[s].details[d].position = dmxData[ad + 2]/255.0;
			strips[s].details[d].fadeDown = dmxData[ad + 3]/255.0;
			strips[s].details[d].fadeUp = dmxData[ad + 4]/255.0;
			strips[s].details[d].repetition = dmxData[ad + 5]/255.0;
			ad += 6;
		}
	}

	long e = millis();
	//Serial.println(e-b);
	b = millis();
	for (int i = 0; i < 8; i++)
	{
		strips[i].update();
	}
	e = millis();
	//Serial.println(e-b);
	//Serial.println("end");
	outputIsDirty = true;
}

*/