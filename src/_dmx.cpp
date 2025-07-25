#include "_dmx.h"

TaskHandle_t TaskForDMX;

int transmitPin = 4;
int receivePin = 5;
int enablePin = 21;

dmx_port_t dmxPort = 1;

byte dmxData[DMX_PACKET_SIZE];

bool dmxIsConnected = false;

portMUX_TYPE dmxMux = portMUX_INITIALIZER_UNLOCKED;

void onDmxFrame();

void loopDMX() {
  dmx_packet_t packet;
  if (dmx_receive(dmxPort, &packet, DMX_TIMEOUT_TICK)) {
    unsigned long now = millis();
    if (!packet.err) {
      if (!dmxIsConnected) {
        dmxIsConnected = true;
      }

      dmx_read(dmxPort, dmxData, packet.size);
      onDmxFrame();
      }
    } else {
      if (dmxIsConnected) {
        dmxIsConnected = false;
      }
    }
} 

void TaskForDMXCode( void * pvParameters ){
  for(;;){
    loopDMX();
  } 
}

void setupDMX() {
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {
    {1, "Default Personality"}
  };
  int personality_count = 1;
  dmx_driver_install(dmxPort, &config, personalities, personality_count);
  dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);

  xTaskCreatePinnedToCore(
                    TaskForDMXCode,   /* Task function. */
                    "TaskForDMX",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskForDMX,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  


}

