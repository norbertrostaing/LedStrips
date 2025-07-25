#include <ArduinoJson.h>

extern JsonDocument config;
extern JsonDocument configTypes;
extern JsonDocument configOptions;
extern JsonDocument info;
extern JsonDocument trigger;
extern int chipId;
extern String chipName;
extern int eepromMarker;

void configUpdatedMain(String key);
void configUpdatedWifi(String key);
void configUpdatedEthernet(String key);
void triggerTriggered(String str);

void addBoolConfig(String address, bool defaultValue);
void addIntConfig(String address, int defaultValue);
void addFloatConfig(String address, float defaultValue);
void addStringConfig(String address, String defaultValue);
void addEnumConfig(String address, String defaultValue);

void addTrigger(String str);

void setRange(String address, int min, int max);
void setRange(String address, float min, float max);
void setRange(String address, double min, double max);
void addEnumOption(String address, String option);

void initConfig();

bool getConfig(String key, bool def);

String getConfig(String key, String def);
String getConfig(String key, const char*);
int getConfig(String key, int def);
float getConfig(String key, float def);

String getConfig(String key, String def, String exclude);
String getConfig(String key, const char*, String exclude);
int getConfig(String key, int def, int exclude);
float getConfig(String key, float def, float exclude);

void writeConfig(String key, bool value);
void writeConfig(String key, int value);
void writeConfig(String key, float value);
void writeConfig(String key, String value);

