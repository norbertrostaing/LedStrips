#include "ledStrip.h"
#include "_config.h"
#include <vector>
#include <cmath>  // pour fmodf, fmaxf

TaskHandle_t TaskForLeds;

std::vector<ledStripMain> strips;
bool outputIsDirty = true;
bool dataIsDirty = false;
bool rgbIsDirty = false;
unsigned long TSRGB = 0;

const uint16_t numPixels = 340;

typedef NeoPixelBus<NeoGrbFeature, X8Ws2812xMethod> NPB;
//typedef NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1X8Ws2811Method> NPB;
//typedef NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0X8Ws2812xMethod> NPB;
NPB wsStrips[] = {
    {numPixels, 4},//4
    {numPixels, 5},//5
    {numPixels, 13},//13
    {numPixels, 14},//14
    {numPixels, 15},//15
    {numPixels, 16},//16
    {numPixels, 32},//32
    {numPixels, 33}//33
};

// Fonction de mappage float à float
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Écrit un pixel sur le ruban spécifié
void setPixel(int strip, int pixel, uint8_t r, uint8_t g, uint8_t b) {
    RgbColor c(r, g, b);
    wsStrips[strip].SetPixelColor(pixel, c);
}

RgbColor getPixel(int strip, int pixel) {
    RgbColor c = wsStrips[strip].GetPixelColor(pixel);
    return c;
}

// --- Classe ledStripDetail ---

ledStripDetail::ledStripDetail() {
}

static float trapezoid(float x, float fadeInStart, float solidStart, float solidEnd, float fadeOutEnd) {
    if (x <= fadeInStart || x >= fadeOutEnd) return 0.0f;
    if (x < solidStart)  return (x - fadeInStart) / (solidStart - fadeInStart);
    if (x <= solidEnd)    return 1.0f;
    return (fadeOutEnd - x) / (fadeOutEnd - solidEnd);
}

void ledStripDetail::update() {
    float center      = 2.0f * position;
    float halfSolid   = size;
    float halfFadeIn  = fadeDown * 2.0f;
    float halfFadeOut = fadeUp   * 2.0f;

    fadeInStart = center - halfSolid - halfFadeIn;
    solidStart  = center - halfSolid;
    solidEnd    = center + halfSolid;
    fadeOutEnd  = center + halfSolid + halfFadeOut;
}

float ledStripDetail::getVal(float pos) {
    float period = 1.0f + repetition * 20.0f;
    float x      = fmodf(pos * period, 2.0f);

    float v0 = trapezoid(x,     fadeInStart, solidStart, solidEnd, fadeOutEnd);
    float v1 = trapezoid(x + 2.0f, fadeInStart, solidStart, solidEnd, fadeOutEnd);
    float v2 = trapezoid(x - 2.0f, fadeInStart, solidStart, solidEnd, fadeOutEnd);

    float maxVal = fmaxf(v0, fmaxf(v1, v2));
    return maxVal * dimmer;
}

ledStripMain::ledStripMain() {
    for (int i = 0; i < 6; i++) {
        details.emplace_back();
    }
}

void ledStripMain::update() {
    int nDetails = config["dmx/details"].as<int>();

    for (int i = 0; i < nDetails && i < details.size(); i++) {
        details[i].update();
    }

    float rBase = r * dimmer;
    float gBase = g * dimmer;
    float bBase = b * dimmer;

    for (int i = 0; i < nLeds; i++) {
        float pos = float(i) / float(nLeds - 1);
        float rr = rBase, gg = gBase, bb = bBase;

        for (int d = 0; d < nDetails && d < details.size(); d++) {
            float grad = details[d].getVal(pos);
            if (grad > 0) {
                rr = mapFloat(grad, 0, 1, rr, details[d].r);
                gg = mapFloat(grad, 0, 1, gg, details[d].g);
                bb = mapFloat(grad, 0, 1, bb, details[d].b);
            }
        }

        RgbColor actual = getPixel(id, i);

        if (rr > actual.R + soothUp) {rr = actual.R+soothUp; } 
        if (gg > actual.G + soothUp) {gg = actual.G+soothUp; } 
        if (bb > actual.B + soothUp) {bb = actual.B+soothUp; } 

        if (rr < actual.R - smoothDown) {rr = actual.R-smoothDown; } 
        if (gg < actual.G - smoothDown) {gg = actual.G-smoothDown; } 
        if (bb < actual.B - smoothDown) {bb = actual.B-smoothDown; } 


        setPixel(id, i, uint8_t(rr), uint8_t(gg), uint8_t(bb));
    }
}

void loopLeds() {
    if (dataIsDirty) {
        for (int i = 0; i < 8; i++) {
            strips[i].update();
        }
        dataIsDirty = false;
        outputIsDirty = true;
    }

    if (outputIsDirty) {
        for (int i = 0; i < 8; i++) {
            wsStrips[i].Show();
        }
        outputIsDirty = false;
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

// Tâche FreeRTOS pour LEDs
void TaskForLedsCode(void *pvParameters) {
    for (;;) {
        loopLeds();
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}


void setupLeds() {
    for (int i = 0; i < 8; i++) {
        strips.emplace_back();
        strips[i].nLeds = config["ledCount/" + String(i + 1)].as<int>();
        strips[i].id    = i;
    }

    for (int i = 0; i < 8; i++) {
        wsStrips[i].Begin();
        for (int p = 0; p< numPixels; p++) {
            setPixel(i,p,0,0,0);
        }
    }


    xTaskCreatePinnedToCore(
        TaskForLedsCode,
        "TaskForLeds",
        2048,
        NULL,
        1,
        &TaskForLeds,
        1
    );
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
	    strips[s].soothUp = 255-data[ad + 1];
	    strips[s].smoothDown = 255-data[ad + 2];
		ad += 3;
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

	dataIsDirty = true;
	long e = millis();
}

