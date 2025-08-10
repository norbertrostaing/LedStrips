#include "_ethernet.h"

TaskHandle_t TaskForEthernet;

bool ethernetConnected = false;

void updateIpAddress() {
  IPAddress noIP(0,0,0,0);
  IPAddress staticIP;
  IPAddress staticSM;
  staticIP.fromString(config["ethernet/staticIP"].as<String>());
  staticSM.fromString(config["ethernet/staticSM"].as<String>());
  ETH.config(noIP, noIP, noIP);
  vTaskDelay(pdMS_TO_TICKS(100));
  if (staticIP != noIP && staticSM != noIP) {
    ETH.config(staticIP, noIP, staticSM);
  } else {
    bool validIP = false;
    for (int i = 0; i < 10 && !validIP; i++) {
      validIP = ETH.localIP() != IPAddress(0,0,0,0);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (!validIP) {
      uint8_t x = (chipId >> 8) & 0xFF;  // Bits 8 à 15
      uint8_t y = chipId & 0xFF;        
      IPAddress autoIP(169, 254, x, y);
      IPAddress subnet(255, 255, 0, 0);
      ETH.config(autoIP, noIP, subnet);   
      vTaskDelay(pdMS_TO_TICKS(200));     
    }
  }
  info["ethernet/IP"] = ETH.localIP();
  info["ethernet/SM"] = ETH.subnetMask();
  Serial.println(ETH.localIP());
}

void loopEthernet() {
    /*
    if (!ethernetConnected && ETH.linkUp()) {
      updateIpAddress();
      Serial.println("connected");
      ethernetConnected = true;

    } else if (ethernetConnected && !ETH.linkUp()) {
      Serial.println("disconnected");
      ethernetConnected = false;
      ETH.config(IPAddress(0,0,0,0),IPAddress(0,0,0,0),IPAddress(0,0,0,0));
      info["ethernet/IP"] = "not connected";
      info["ethernet/SM"] = "not connected";
    }
    */
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void TaskForEthernetCode( void * pvParameters ){
  for(;;){
    loopEthernet();
  } 
}

void setupEthernet() {
  ETH.begin();

  addStringConfig("ethernet/staticIP", "");
  addStringConfig("ethernet/staticSM", "");
  info["ethernet/IP"] = "not connected";
  info["ethernet/SM"] = "not connected";
  updateIpAddress();
  // xTaskCreatePinnedToCore(
  //                   TaskForEthernetCode,   /* Task function. */
  //                   "TaskForEthernet",     /* name of task. */
  //                   2048,       /* Stack size of task */
  //                   NULL,        /* parameter of the task */
  //                   1,           /* priority of the task */
  //                   &TaskForEthernet,      /* Task handle to keep track of created task */
  //                   1);          /* pin task to core 0 */                  
  
 
}

void configUpdatedEthernet(String k) {
  if (k == "ethernet/staticIP" || k == "ethernet/staticSM") {
    updateIpAddress();
  }
}

