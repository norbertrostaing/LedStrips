#include "_artnet.h"

TaskHandle_t TaskForArtnet;

// Artnet settings
Artnet artnet;

void loopArtnet()
{
  // we call the read function inside the loop
  artnet.read();
}


void TaskForArtnetCode( void * pvParameters ){
  for(;;){
    delay(5);
    loopArtnet();
  } 
}

void setupArtnet()
{
  //Serial.begin(115200);
  artnet.begin();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onArtnetFrame);

  xTaskCreatePinnedToCore(
                    TaskForArtnetCode,   /* Task function. */
                    "TaskForArtnet",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskForArtnet,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  


}

