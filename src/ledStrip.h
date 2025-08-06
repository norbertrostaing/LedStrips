#include <Arduino.h>
#include <vector>
#include <NeoPixelBus.h>

extern bool outputIsDirty;
extern bool dataIsDirty;

void loopLeds();
void setupLeds();

class ledStripDetail {
public:
  float dimmer = 0;
  float r=0, g=0, b=0;
  float position=0;
  float size=0;
  float fadeUp=0;
  float fadeDown=0;
  float repetition=0;
  
  float beginFade = 0;
  float beginSolid = 0;
  float endSolid = 0;
  float endFade = 0;
  bool beginSolidLoops = false;
  bool endSolidLoops = false;
  bool beginFadeLoops = false;
  bool endFadeLoops = false;
  
  ledStripDetail();
  void update();
  float getVal(float pos);
};

class ledStripMain {
public:
    float dimmer;
    float r, g, b;
    int nLeds;
    int id;
    std::vector<ledStripDetail> details; // Tableau dynamique de LedStripDetail

    ledStripMain() ;
    void update() ;
};

extern std::vector<ledStripMain> strips;


