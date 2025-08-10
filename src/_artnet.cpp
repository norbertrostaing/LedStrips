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
    vTaskDelay(pdMS_TO_TICKS(1));
    //taskYIELD();
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
                    8192,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskForArtnet,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  


}

