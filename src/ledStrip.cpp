#include "ledStrip.h"
#include "_config.h"
#include <vector>
#include <cmath>  // pour fmodf, fmaxf

TaskHandle_t TaskForLeds;

std::vector<ledStripMain> strips;
bool outputIsDirty = true;
bool dataIsDirty = true;
bool rgbIsDirty = false;
unsigned long TSRGB = 0;

const uint16_t numPixels = 340;

typedef NeoPixelBus<NeoGrbFeature, X8Ws2812xMethod> NPB;
NPB wsStrips[] = {
    {numPixels, 4},
    {numPixels, 5},
    {numPixels, 13},
    {numPixels, 14},
    {numPixels, 15},
    {numPixels, 16},
    {numPixels, 32},
    {numPixels, 33}
};

// Fonction de mappage float à float
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Écrit un pixel sur le ruban spécifié
void setPixel(int strip, int pixel, uint8_t r, uint8_t g, uint8_t b) {
    int nStrip = strip;
    int deltaPix = 0;
    if (strip >= 8) {
        nStrip -= 8;
        deltaPix = strips[nStrip].nLeds;
    }
    RgbColor c(r, g, b);
    wsStrips[nStrip].SetPixelColor(pixel+deltaPix, c);
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

        setPixel(id, i, uint8_t(rr), uint8_t(gg), uint8_t(bb));
    }
}

void loopLeds() {
    vTaskDelay(pdMS_TO_TICKS(5));

    if (dataIsDirty) {
        for (int i = 0; i < 16; i++) {
            strips[i].update();
        }
        dataIsDirty = false;
        outputIsDirty = true;
    }

    if (rgbIsDirty && TSRGB < millis()) {
        rgbIsDirty = false;
        outputIsDirty = true;
    }

    if (outputIsDirty) {
        outputIsDirty = false;
        for (int i = 0; i < 8; i++) {
            wsStrips[i].Show(false);
        }
    }
}

// Tâche FreeRTOS pour LEDs
void TaskForLedsCode(void *pvParameters) {
    for (;;) {
        loopLeds();
    }
}


void setupLeds() {
    for (int i = 0; i < 16; i++) {
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
        10000,
        NULL,
        1,
        &TaskForLeds,
        1
    );
}
