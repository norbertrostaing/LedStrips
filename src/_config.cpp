#include "_config.h"
#include <EEPROM.h>

#define EEPROM_SIZE 4096 // Taille de l'EEPROM simulée
#define JSON_START_ADDR 0 // Adresse de départ pour le JSON dans l'EEPROM
#define JSON_MAX_SIZE 2048 // Taille maximale pour l'objet JSON

JsonDocument config;
JsonDocument info;
JsonDocument trigger;
JsonDocument configTypes;
JsonDocument configOptions;

int chipId;
String chipName = "Olgalimex";

int eepromMarker = 0xED;

void saveConfigToEEPROM() {
  // Sérialiser le JSON dans un buffer
  char buffer[JSON_MAX_SIZE];
  size_t jsonSize = serializeJson(config, buffer, sizeof(buffer));

  if (jsonSize > JSON_MAX_SIZE) {
    Serial.println("Erreur : Le JSON est trop grand pour l'EEPROM !");
    return;
  }

  // Stocker le JSON dans l'EEPROM
  for (size_t i = 0; i < jsonSize; i++) {
    EEPROM.write(JSON_START_ADDR + i, buffer[i]);
  }

  // Marquer la fin de l'objet avec un caractère null
  EEPROM.write(JSON_START_ADDR + jsonSize, '\0');

  // Sauvegarder les changements
  EEPROM.commit();
}

bool loadConfigFromEEPROM() {
  // Lire le JSON stocké dans un buffer
  char buffer[JSON_MAX_SIZE];
  for (size_t i = 0; i < JSON_MAX_SIZE; i++) {
    buffer[i] = EEPROM.read(JSON_START_ADDR + i);
    if (buffer[i] == '\0') break; // Fin du JSON
  }

  // Désérialiser le JSON
  DeserializationError error = deserializeJson(config, buffer);
  if (error) {
    Serial.println("Erreur de désérialisation du JSON !");
    return false;
  }

  return true;
}

bool isFirstBoot() {
  // Vérifier si une signature ou un drapeau est présent
  return EEPROM.read(EEPROM_SIZE - 1) != eepromMarker; // marker comme marqueur
}

void markAsBooted() {
  // Écrire le drapeau indiquant que ce n'est plus le premier démarrage
  EEPROM.write(EEPROM_SIZE - 1, eepromMarker);
  EEPROM.commit();
}

void addBoolConfig(String address, bool defaultValue) {
  if (!config[address].is<bool>()) config[address] = defaultValue;
  configTypes[address] = "b";
}

void addIntConfig(String address, int defaultValue) {
  if (!config[address].is<int>()) config[address] = defaultValue;
  configTypes[address] = "i";
}

void addFloatConfig(String address, float defaultValue) {
  if (!config[address].is<float>()) config[address] = defaultValue;
  configTypes[address] = "f";
}

void addStringConfig(String address, String defaultValue) {
  if (!config[address].is<String>()) config[address] = defaultValue;
  configTypes[address] = "s";
}

void addEnumConfig(String address, String defaultValue) {
  if (!config[address].is<String>()) config[address] = defaultValue;
  configTypes[address] = "e";
  configOptions[address].to<JsonObject>();
  configOptions[address]["options"].to<JsonArray>();
  addEnumOption(address, defaultValue);
}

void setRange(String address, int min, int max) {
  configOptions[address].to<JsonObject>();
  configOptions[address]["rangeMin"] = min;
  configOptions[address]["rangeMax"] = max;
}

void setRange(String address, float min, float max) {
  configOptions[address].to<JsonObject>();
  configOptions[address]["rangeMin"] = min;
  configOptions[address]["rangeMax"] = max;
}

void addTrigger(String str) {
  trigger[str] = "";
}

void setRange(String address, double min, double max)
{
    configOptions[address].to<JsonObject>();
    configOptions[address]["rangeMin"] = (float)min;
    configOptions[address]["rangeMax"] = (float)max;
}

void addEnumOption(String address, String option) {
  configOptions[address]["options"].as<JsonArray>().add(option);
}





void initConfig() {
  uint64_t macAddress = ESP.getEfuseMac();
  uint64_t macAddressTrunc = macAddress << 40;
  chipId = macAddressTrunc >> 40;

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Erreur d'initialisation de l'EEPROM !");
    return;
  }

  if (isFirstBoot()) {
    saveConfigToEEPROM();
    markAsBooted();
  } else {
    if (loadConfigFromEEPROM()) {
      // OK !
    } else {
      Serial.println("Erreur lors du chargement du JSON depuis l'EEPROM !");
    }
  }
}

void configUpdated(String key) {
  configUpdatedMain(key);
  configUpdatedEthernet(key);
  configUpdatedWifi(key);
}



bool getConfig(String key, bool def) {
  if (config[key].as<bool>() != false) {
    return config[key].as<bool>();
  }
  return def;
}


String getConfig(String key, const char* c) {
  return getConfig(key, (String)c, "");
}

String getConfig(String key, String def) {
  return getConfig(key, def, "");
}

int getConfig(String key, int def) {
  return getConfig(key, def, -1);
}

float getConfig(String key, float def) {
  return getConfig(key, def, -1.0);
}


String getConfig(String key, const char* c, String exclude) {
  return getConfig(key, (String)c, exclude);
}

String getConfig(String key, String def, String exclude) {
  if (config[key].as<String>() != exclude) {
    return config[key].as<String>();
  }
  return def;
}

int getConfig(String key, int def, int exclude) {
  if (config[key].as<int>() != exclude) {
    return config[key].as<int>();
  }
  return def;
}

float getConfig(String key, float def, float exclude) {
  if (config[key].as<float>() != exclude) {
    return config[key].as<float>();
  }
  return def;

}


void writeConfig(String key, bool value) {
    config[key] = value;
    configUpdated(key);
    saveConfigToEEPROM();
}

void writeConfig(String key, int value) {
    config[key] = value;
    configUpdated(key);
    saveConfigToEEPROM();
}

void writeConfig(String key, float value) {
    config[key] = value;
    configUpdated(key);
    saveConfigToEEPROM();
}

void writeConfig(String key, String value) {
    config[key] = value;
    configUpdated(key);
    saveConfigToEEPROM();
}