#include "_config.h"
#include "_osc.h"

int oscPort = 9004; 

TaskHandle_t TaskForOsc;

void loopOsc() {
  OscWiFi.update();  // should be called to receive + send osc
}

void TaskForOscCode( void * pvParameters ){
  for(;;){
    loopOsc();
    vTaskDelay(pdMS_TO_TICKS(3));
  } 
}

void sendConfig(String k, String remoteIp) {

  if (configTypes[k] == "b") { 
    OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<bool>()); 
  }
  else if (configTypes[k] == "f") { 
    OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<float>()); 
    if (!configOptions[k]["rangeMin"].isNull()) {
      OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<float>(), "range",configOptions[k]["rangeMin"].as<float>(), configOptions[k]["rangeMax"].as<float>());
    }
  }
  else if (configTypes[k] == "i") { 
    OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<int>()); 
    if (!configOptions[k]["rangeMin"].isNull()) {
      OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<int>(), "range",configOptions[k]["rangeMin"].as<int>(), configOptions[k]["rangeMax"].as<int>());
    }
  }
  else if (configTypes[k] == "s") { 
    OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<String>()); 
  }
  else if (configTypes[k] == "e") { 
    OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<String>()); 
    OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/config/"+k, config[k].as<String>(), "options",configOptions[k]["options"].as<String>());
  }

}

void sendInfo(String k, String remoteIp) {
  if (!info[k].isNull() && info[k].is<bool>()) { OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/info/"+k, info[k].as<bool>()); }
  else if (!info[k].isNull() && info[k].is<float>()) { OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/info/"+k, info[k].as<float>()); }
  else if (!info[k].isNull() && info[k].is<int>()) { OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/info/"+k, info[k].as<int>()); }
  else if (!info[k].isNull() && info[k].is<String>()) { OscWiFi.send(remoteIp, oscPort, "/"+chipName+""+String(chipId)+"/info/"+k, info[k].as<String>()); }
}

void setupOSC() {
  xTaskCreatePinnedToCore(
                    TaskForOscCode,   /* Task function. */
                    "TaskForOsc",     /* name of task. */
                    2048,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskForOsc,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  

  for (JsonPair kv : config.as<JsonObject>()) {
    String k = kv.key().c_str();
    OscWiFi.subscribe(oscPort, "/config/"+k, [k](const OscMessage& m){
      if (m.size()>0) {
        if (m.isBool(0)) {writeConfig(k, m.getArgAsBool(0));}
        else if (m.isFloat(0)) {writeConfig(k, m.getArgAsFloat(0));}
        else if (m.isInt32(0)) {writeConfig(k, m.getArgAsInt32(0));}
        else if (m.isStr(0)) {writeConfig(k, m.getArgAsString(0));}
      } else {
        sendConfig(k, m.remoteIP());
      }
    } );
  }

  OscWiFi.subscribe(oscPort, "/config", [](const OscMessage& m){
      for (JsonPair kv : config.as<JsonObject>()) {
        String k = kv.key().c_str();
        sendConfig(k, m.remoteIP());
      }
  });

  for (JsonPair kv : trigger.as<JsonObject>()) {
    String k = kv.key().c_str();
    OscWiFi.subscribe(oscPort, "/trigger/"+k, [k](const OscMessage& m){
      triggerTriggered(k);
    } );
  }
  OscWiFi.subscribe(oscPort, "/trigger", [](const OscMessage& m){
    for (JsonPair kv : trigger.as<JsonObject>()) {
      String k = kv.key().c_str();
      //OscWiFi.send(m.remoteIP(), oscPort, "/"+chipName+""+String(chipId)+"/trigger/"+k); 
    }
  });

  for (JsonPair kv : info.as<JsonObject>()) {
    String k = kv.key().c_str();
    OscWiFi.subscribe(oscPort, "/info/"+k, [k](const OscMessage& m){
      sendInfo(k, m.remoteIP());
    } );
  }
  OscWiFi.subscribe(oscPort, "/info", [](const OscMessage& m){
      for (JsonPair kv : info.as<JsonObject>()) {
        String k = kv.key().c_str();
        sendInfo(k, m.remoteIP());
      }
  });

}


