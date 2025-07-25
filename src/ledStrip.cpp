#include "ledStrip.h"
#include "_config.h"

std::vector<ledStripMain> strips;
bool outputIsDirty = true;

const uint16_t numPixels = 240;
//const uint16_t numPixels = 120;

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip1(numPixels, 4);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> strip2(numPixels, 5);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt2Ws2812xMethod> strip3(numPixels, 13);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt3Ws2812xMethod> strip4(numPixels, 14);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt4Ws2812xMethod> strip5(numPixels, 15);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt5Ws2812xMethod> strip6(numPixels, 16);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt6Ws2812xMethod> strip7(numPixels, 32);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt7Ws2812xMethod> strip8(numPixels, 33);

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setPixel(int strip, int pixel, uint8_t r, uint8_t g, uint8_t b) {
	RgbColor c(r, g, b);
	switch (strip) {
	    case 0: strip1.SetPixelColor(pixel, c); break;
	    case 1: strip2.SetPixelColor(pixel, c); break;
	    case 2: strip3.SetPixelColor(pixel, c); break;
	    case 3: strip4.SetPixelColor(pixel, c); break;
	    case 4: strip5.SetPixelColor(pixel, c); break;
	    case 5: strip6.SetPixelColor(pixel, c); break;
	    case 6: strip7.SetPixelColor(pixel, c); break;
	    case 7: strip8.SetPixelColor(pixel, c); break;
	}
}



ledStripDetail::ledStripDetail() {
}

void ledStripDetail::update() {
    beginSolid = (2*position)-(size);
    beginFade = beginSolid-(2*fadeDown);
    endSolid = (2*position)+(size);
    endFade = endSolid+(2*fadeUp);
    
    beginSolidLoops = beginSolid < 0;
    endSolidLoops = endFade > 2;
    beginFadeLoops = !beginSolidLoops && beginFade < 0;
    endFadeLoops = !endSolidLoops && endFade > 2;

}
  
float ledStripDetail::getVal(float pos) {
    float r = (1+repetition*20.0);
    pos = pos*r;
    pos = fmod(pos,2.0);
    
    float valSolid = 0;
    if (size > 0) {
      if (beginSolidLoops && pos-2 >= beginSolid && pos-2 <= endSolid) valSolid = dimmer;
      else if (endSolidLoops && pos+2 >= beginSolid && pos+2 <= endSolid) valSolid = dimmer;
      else if (pos >= beginSolid && pos <= endSolid) valSolid = dimmer;
    }

    float valBeginFade = 0;
    if (beginFadeLoops && pos-2 > beginFade && pos-2 <= beginSolid) valBeginFade = mapFloat(pos-2, beginFade, beginSolid, 0, dimmer);
    else if (pos > beginFade && pos <= beginSolid) valBeginFade = mapFloat(pos, beginFade, beginSolid, 0, dimmer);
    else if (pos-2 > beginFade && pos-2 <= beginSolid) valBeginFade = mapFloat(pos-2, beginFade, beginSolid, 0, dimmer);

    float valEndFade = 0;
    if (endFadeLoops && pos+2 < endFade && pos+2 >= endSolid) valEndFade = mapFloat(pos-2, endFade, endSolid, 0, dimmer);
    else if (pos < endFade && pos >= endSolid) valEndFade = mapFloat(pos, endFade, endSolid, 0, dimmer);
    else if (pos+2 < endFade && pos+2 >= endSolid) valEndFade = mapFloat(pos+2, endFade, endSolid, 0, dimmer);

    return max(valSolid, max(valBeginFade, valEndFade));
}


ledStripMain::ledStripMain() {
    for (int i = 0; i< 6; i++) {
        details.emplace_back();
    }
}


void ledStripMain::update() {
	int nDetails = config["dmx/details"].as<int>();
    for (int i = 0; i< nDetails; i++) {
        details[i].update();
    }
    float rBase = r*dimmer; 
    float gBase = g*dimmer; 
    float bBase = b*dimmer;
    for (int i = 0; i< nLeds; i++) {
        float r = rBase;
        float g = gBase;
        float b = bBase;
        float pos = (float)i/((float)nLeds-1);
        for (int d = 0; d<nDetails; d++) {
            float grad = details[d].getVal(pos);
            if (grad>0) 
            {
                r = mapFloat(grad, 0, 1, r, details[d].r);
                g = mapFloat(grad, 0, 1, g, details[d].g);
                b = mapFloat(grad, 0, 1, b, details[d].b);
            }
        }
        setPixel(id, i, r, g, b);
    }
}



void setupLeds() {

	for (int i = 0; i < 8; i++)
	{
		strips.emplace_back();
		strips[i].nLeds = config["ledCount/" + String(i + 1)].as<int>();
		strips[i].id = i;
	}

    strip1.Begin();
    strip2.Begin();
    strip3.Begin();
    strip4.Begin();
	strip5.Begin();
	strip6.Begin();
	strip7.Begin();
	strip8.Begin();  
}

void loopLeds() {
	if (outputIsDirty) {
		outputIsDirty = false;
        long from = millis();
        strip1.Show();
        strip2.Show();
        strip3.Show();
        strip4.Show();
        strip5.Show();
        strip6.Show();
        strip7.Show();
        strip8.Show();  
        long to = millis();
    }
}