#include "_dmx.h"

TaskHandle_t TaskForDMX;

int transmitPin = 2;
int receivePin = 5;
int enablePin = 2;

dmx_port_t dmxPort = 1;

byte dmxDataRcv[DMX_PACKET_SIZE];
byte dmxData[DMX_PACKET_SIZE];

bool dmxIsConnected = false;

int packetErrors = 0;


portMUX_TYPE dmxMux = portMUX_INITIALIZER_UNLOCKED;

void onDmxFrame();

void setupDriver() {
  packetErrors = 0;
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {
    {1, "Default Personality"}
  };
  int personality_count = 1;
  dmx_driver_install(dmxPort, &config, personalities, personality_count);
  dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
}

void loopDMX() {
  dmx_packet_t packet;
  if (dmx_receive(dmxPort, &packet, DMX_TIMEOUT_TICK)) {
    unsigned long now = millis();
    if (!packet.err) {
      if (!dmxIsConnected) {
        dmxIsConnected = true;
      }
      dmx_read(dmxPort, dmxDataRcv, packet.size);
      if (packet.size == 513) {
        memcpy(dmxData, dmxDataRcv, 512);
        onDmxFrame();
        vTaskDelay(pdMS_TO_TICKS(10));
      } else if (packet.size == 1)
        packetErrors++;
        
        if (packetErrors > 44) {
        dmx_driver_delete(dmxPort);
        setupDriver();
        Serial.println("reboot DMX Service");
        }
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
    vTaskDelay(pdMS_TO_TICKS(3));
  } 
}

void setupDMX() {
  setupDriver();
  xTaskCreatePinnedToCore(
                    TaskForDMXCode,   /* Task function. */
                    "TaskForDMX",     /* name of task. */
                    2048,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    4,           /* priority of the task */
                    &TaskForDMX,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  


}

