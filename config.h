#include <EEPROM.h>

typedef struct
{
  char BOFmark = 'A';
  int compressorId;
  double pidSetpoint; // Pid for compressor over heat protection
  double pidP;
  double pidI;
  double pidD;
  double requestedHotWaterTemp; // temp < requestedHotWaterTemp = use pid for hot water valve
  double heatingHysteresis; //requestedHotWaterTemp + heatingHysteresis = return to normal mode
  double requestedCoolWaterTemp; // temp > requestedCoolWaterTemp = start compressor
  double coolingHysteresis; //requestedCoolWaterTemp - coolingHysteresis = stop temp
  double warmWaterStartTemp; //Temperature for compressor start because of warming need
  double warmWaterStopTemp; // Temperature for compressor stop because of enough warming heat
  double compressorColdLimit; // Force compressor to stop under this temperature.
  double compressorHotLimit; // Force compressor to stop over this temperature.
  int servoMin;
  int servoMax;
  
  // temp sensor onewire addresses
  byte pidSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for hot water shunt pid controlling
  byte hotWaterSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for hot water heating request
  byte coolingStartSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for cooling start
  byte coolingStopSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for cooling stop
  byte warmWaterStartSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for warming start
  byte warmWaterStopSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for warming stop
  byte compressorHotLimitSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for emergency stop
  byte compressorColdLimitSensorAddr[8] = {0,0,0,0,0,0,0,0}; //Sensor address for emergency stop
  char EOFmark = 'E';
}  settingsType;

settingsType settings;


template <class T> int EEPROM_writeGeneric(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.update(ee++, *p++);
  return i;
}

template <class T> int EEPROM_readGeneric(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

bool saveConfig() {
  
  DBG_OUTPUT_PORT.print(F("Writing config to memory "));
  settings.BOFmark = 'A';
  settings.EOFmark = 'E';
  int count = EEPROM_writeGeneric(1, settings);
  DBG_OUTPUT_PORT.print(count);
  DBG_OUTPUT_PORT.println(F("Bytes writed"));
}
bool loadConfig() {
  int count = EEPROM_readGeneric(1, settings);
  DBG_OUTPUT_PORT.print(F("Bytes read: "));
  DBG_OUTPUT_PORT.println(count);
  if (settings.BOFmark == 'A' && settings.EOFmark == 'E')
  {
    DBG_OUTPUT_PORT.println(F("Configuration found"));
  }
  else 
  {
    DBG_OUTPUT_PORT.print(F("Cannot find configuration!"));
    return false;
  }
}
