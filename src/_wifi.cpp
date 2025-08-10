#include "_wifi.h"

TaskHandle_t TaskForWifi;

void loopWifi() {
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() != config["wifi/name"].as<String>()) {
      WiFi.disconnect();
      info["wifi/IP"] = "not connected";
      info["wifi/SM"] = "not connected";
    } else {
      info["wifi/IP"] = WiFi.localIP();
      info["wifi/SM"] = WiFi.subnetMask();
    }
    if (config["wifi/name"] != "" && WiFi.status() != WL_CONNECTED) {
        //WiFi.mode(WIFI_STA); //Optional
        WiFi.begin(config["wifi/name"].as<String>(), config["wifi/pass"].as<String>());
        Serial.println("\nConnecting");

        int count = 0;
        while(WiFi.status() != WL_CONNECTED && count < 10){
            count ++;
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to the WiFi network");
            Serial.print("Local ESP32 IP: ");
            Serial.println(WiFi.localIP());
            info["wifi/IP"] = WiFi.localIP();
            info["wifi/SM"] = WiFi.subnetMask();
        } else {
            info["wifi/IP"] = "not connected";
            info["wifi/SM"] = "not connected";
        }
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
}

void TaskForWifiCode( void * pvParameters ){
  for(;;){
    loopWifi();
  } 
}

void setupWifi() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(chipName+" "+String(chipId), "raclette");

  addStringConfig("wifi/name", "");
  addStringConfig("wifi/pass", "");
  info["wifi/IP"] = "not connected";
  info["wifi/SM"] = "not connected";

  xTaskCreatePinnedToCore(
                    TaskForWifiCode,   /* Task function. */
                    "TaskForWifi",     /* name of task. */
                    2048,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskForWifi,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
}

void configUpdatedWifi(String k) {
}
