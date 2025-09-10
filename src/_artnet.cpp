#include "_artnet.h"

TaskHandle_t TaskForArtnet;

// Artnet settings
Artnet artnet;

void loopArtnet()
{
  while(artnet.read() != 0) {}
}


void TaskForArtnetCode( void * pvParameters ){
  for(;;){
    loopArtnet();
    vTaskDelay(pdMS_TO_TICKS(3));
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
                    2048,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    4,           /* priority of the task */
                    &TaskForArtnet,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  


}

