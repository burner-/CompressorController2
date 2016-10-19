#define HOSTNAME "compressor2"
#define DBG_OUTPUT_PORT Serial

#include <Arduino.h>
#include <OneWire.h>
#include <Ethernet.h>
#include "global.h"
#include "webserver.h"
#include <ArduinoJson.h>
#include <Timer.h>
//#include <Event.h>
#include <Servo.h>
#include <LinkedList.h>
#include <PID_v1.h>
#include "config.h"


#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h> //https://github.com/nanopb/nanopb
#include "communication.h"

#define PIN_ONEWIRE 6
#define PIN_MODE_VALVE_SERVO 7
#define PIN_COMPRESSOR_RUN A2

#define PIN_FLOW_VALVE A1 // cold water flow valve

#define PIN_FLOW_PULSE1 A2
#define PIN_FLOW_PULSE2 A3
#define PIN_FLOW_PULSE3 A4
#define COMPRESSOR_ID 2

// define persentage for each position
#define SERVO_WARM_WATER 100 // water for room heating
#define SERVO_HOT_WATER 0 // use water

// define servo value for min/max persentage
#define SERVO_MIN -50
#define SERVO_MAX 185

#define UDP_PORT 8889

byte mac[] = { 0xDE, 0xAD, 0xBE, 0x1F, 0x4A, 0xA2 };
IPAddress ip(10, 220, 2, 22);
IPAddress ip_gw(10, 220, 0, 1);
IPAddress ip_dns(8, 8, 8, 8);
IPAddress ip_mask(255, 255, 0, 0);

IPAddress bcast(255, 255, 255, 255);
byte loghost[] = { 37, 16, 111, 10 };

//compressor pid
double pidInput = 0;
double pidOutput = 0;


PID pid = PID(&pidInput, &pidOutput, &settings.pidSetpoint,settings.pidP,settings.pidI,settings.pidD, REVERSE);
bool autoconfigure = false;
// Kun kuumavesisÃ¯Â¿Â½iliÃ¯Â¿Â½n ala lÃ¯Â¿Â½mpÃ¯Â¿Â½ on alle toivotun aktivoituu kuumavesitoiminto. 
// Kuumavesitoiminnon aikana PID sÃ¯Â¿Â½Ã¯Â¿Â½din pitÃ¯Â¿Â½Ã¯Â¿Â½ huolen, ettÃ¯Â¿Â½ kompressorin poistolÃ¯Â¿Â½mpÃ¯Â¿Â½ ei
// ylitÃ¯Â¿Â½ sille asetettua maksimia olettaen, ettÃ¯Â¿Â½ lÃ¯Â¿Â½mminvesiverkosta saadaan aina kylmempÃ¯Â¿Â½Ã¯Â¿Â½ lauhdutusta kuin
// kuumavesivaraajasta. Kun kuumavesivaraajan lÃ¯Â¿Â½mpÃ¯Â¿Â½ nousee takaisin haluttuun lÃ¯Â¿Â½mpÃ¯Â¿Â½Ã¯Â¿Â½n sulkeutuu kuumavesivaraajan syÃ¯Â¿Â½ttÃ¯Â¿Â½


// packet types
typedef enum packetType { 
	TYPE_SENDERROR = 0x5,
	TYPE_1WIRE = 0x10,
	TYPE_PID_INFO = 0x11,
	TYPE_PB_TEMP_INFO = 0x20,
  TYPE_PB_COMPRESSOR_INFO = 0x21,
	// server to node
	TYPE_PID_SET_ADDRESS = 0xF0,
	TYPE_PID_SET_SETPOINT = 0xF1,
	TYPE_PID_SET_TUNINGS = 0xF2,
	TYPE_SET_COMPRESSOR_SETTINGS = 0x70
};

#include "debug.h"

//timers
Timer t;
bool heating_isStarting = false;
unsigned int localPort = UDP_PORT; // local port to listen on
EthernetUDP Udp;
bool SetMode = false; //DBG_OUTPUT_PORT working mode
Servo servo;


void doJob()
{
  readEthernet();
  yield();
}

void pollSleep(int time)
{
  time = time / 10;
  for(int i=0; time > i;i++)
  {
    delay(10);
    doJob();
  }
}

#include "logic.h"



void printOnewireAddress(byte address[])
{
  for(int i = 0; i < 8; i++) 
  {
    hexPrint(address[i]);
  }
}



// Allow remote nodes to send temperature values. 
void netRecvProtoBuffTempInfo(byte packetBuffer[], int packetSize )
{
  TempInfo message;
  pb_istream_t stream = pb_istream_from_buffer(packetBuffer, packetSize);
  bool status = pb_decode(&stream, TempInfo_fields, &message);
  if (!status)
  {
    DBG_OUTPUT_PORT.print(F("Message decoding failed: "));
    DBG_OUTPUT_PORT.println(PB_GET_ERROR(&stream));
  } 
  else
  {
    if (message.SensorAddr.size == 8)
    {
      updateSensorInfo(message.SensorAddr.bytes,message.Temp,false);
      doLogic(message.SensorAddr.bytes, message.Temp);
    }
  }
}

void readEthernet()
{
  while(Udp.available()){
	//Process ethernet
	int packetSize = Udp.parsePacket();
	if(packetSize)
	{
		//DBG_OUTPUT_PORT.print(F("Received udp packet of size "));
		//DBG_OUTPUT_PORT.println(packetSize);
    
		//DBG_OUTPUT_PORT.print(F("From "));
		IPAddress remote = Udp.remoteIP();
		for (int i =0; i < 4; i++)
		{
			DBG_OUTPUT_PORT.print(remote[i], DEC);
			if (i < 3)
			{
				DBG_OUTPUT_PORT.print(".");
			}
		}
		DBG_OUTPUT_PORT.print(":");
		DBG_OUTPUT_PORT.print(Udp.remotePort());
		
		int messagetype = Udp.read();
		
		DBG_OUTPUT_PORT.print("  ");
		dumpType(messagetype);
    DBG_OUTPUT_PORT.println();
    
		byte packetBuffer[UDP_TX_PACKET_MAX_SIZE - 1];
		Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE - 1);
		
		// parse messages by type
		if (messagetype == TYPE_SET_COMPRESSOR_SETTINGS)
		{
//			netRecvCompressorSettings(packetBuffer, (packetSize -1));
		} 
		else if (messagetype == TYPE_PB_TEMP_INFO)
		{
			netRecvProtoBuffTempInfo(packetBuffer, (packetSize -1));
		}
	}
  }
}



void readSerialConsole()
{
	char c = DBG_OUTPUT_PORT.read();

    if (c == 's')
	{
		SetMode = true;
		DBG_OUTPUT_PORT.println(F("Setmode, give id and press enter"));
	}
	if (SetMode)
	{
		int id = DBG_OUTPUT_PORT.parseInt();
		if (id > 0)
		{
			DBG_OUTPUT_PORT.print(F("Device id set to "));
			DBG_OUTPUT_PORT.println(id);
			settings.compressorId = id;
			SetMode = false;
			saveConfig();
		}
	}
}




void hexPrintArray(byte bytes[], int len)
{
  for (int i = 0; i < len; i++) {
    hexPrint(bytes[i]);
  }
}


void pbSendTemp(byte addr[], float tempVal)
{   
      TempInfo tempinfo = {};
      tempinfo.SensorAddr.size = 8;
      
      tempinfo.has_SensorAddr = true;
      tempinfo.has_Temp = true;
      

      copyByteArray(addr,tempinfo.SensorAddr.bytes,8);

      tempinfo.Temp = tempVal;

      uint8_t bp_send_buffer[20];
      pb_ostream_t stream = pb_ostream_from_buffer(bp_send_buffer, sizeof(bp_send_buffer));
      bool status = pb_encode(&stream, TempInfo_fields, &tempinfo);
      size_t message_length = stream.bytes_written;
      
      //DBG_OUTPUT_PORT.print(" : ");
      //DBG_OUTPUT_PORT.println(message_length);

      if (message_length == 0)
      {
        DBG_OUTPUT_PORT.println(F("ERROR IN SERIALIZER"));
        //SendSyslog(4,F("ERROR IN DBG_OUTPUT_PORTIZER"));
        //wdt_enable(WDTO_30MS);
        //while(1){};
      }
      if (message_length != 15)
      {
        logMsg(4,F("Out of memory?"));
      }

      if (!status)
      {
        
        logMsg(4,F("Encoding failed"));
        DBG_OUTPUT_PORT.println(PB_GET_ERROR(&stream));
      } 
      else 
      {
          Udp.beginPacket(bcast, localPort);
          Udp.write(TYPE_PB_TEMP_INFO);
          Udp.write(bp_send_buffer,message_length);
          Udp.endPacket();
      }
    
}
void pbSendCompressorInfo()
{
  
      CompressorInfo compressorinfo = {};
      compressorinfo.has_pidOutput = true;
      
      compressorinfo.compressorId = COMPRESSOR_ID;
      compressorinfo.state_water_heating = state_water_heating; // hot water heating
      compressorinfo.state_room_heating = state_room_heating; //room radioators need more heat
      compressorinfo.state_cooling = state_cooling; // cool network need more coolness
      compressorinfo.compressorRun = compressorRun; // compressor running
      compressorinfo.panicColdStopActive = panicColdStopActive;
      compressorinfo.panicHotStopActive = panicHotStopActive;
      compressorinfo.maintainceStop = maintainceStop;
      compressorinfo.pidOutput = pidOutput; // hot water servo position

      uint8_t bp_send_buffer[70];
      pb_ostream_t stream = pb_ostream_from_buffer(bp_send_buffer, sizeof(bp_send_buffer));
      bool status = pb_encode(&stream, CompressorInfo_fields, &compressorinfo);
      size_t message_length = stream.bytes_written;
      
      DBG_OUTPUT_PORT.print(F("Compressor info : "));
      DBG_OUTPUT_PORT.println(message_length);

      if (message_length == 0)
      {
        DBG_OUTPUT_PORT.println(F("ERROR IN SERIALIZER"));
      }
      
      if (!status)
      {
        
        logMsg(4,F("Encoding failed"));
        DBG_OUTPUT_PORT.println(PB_GET_ERROR(&stream));
      } 
      else 
      {
          Udp.beginPacket(bcast, localPort);
          Udp.write(TYPE_PB_COMPRESSOR_INFO);
          Udp.write(bp_send_buffer,message_length);
          Udp.endPacket();
      }
}



 /*
  * Tempsensor
  * 
  */

// 1-wire
//bool allFound = false;
OneWire ds(PIN_ONEWIRE);  // onewire pin
//int sensorCount = 0;

class SensorInfo
{
public:
  byte SensorAddress[8];
  float Temperature = 0;
  boolean online = false;
  boolean local = false;
};
LinkedList<SensorInfo*> sensors = LinkedList<SensorInfo*>();



void readTempSensor(byte addr[8], bool needConversion = false)
{
  int i = 0;
  byte present = 0;
  byte data[9] = {};
  float tempVal = 0;
  if (needConversion) // Each temp sensor need internal temp conversion from sensor to scratchpad
  {
    ds.reset();
    doJob();
    ds.select(addr);
    doJob();
    ds.write(0x44);
    pollSleep(750);
  }
  present = ds.reset();
  doJob();
  ds.select(addr); // select sensor
  doJob();
  ds.write(0xBE);         // Read Scratchpad
  doJob();
  hexPrintArray(addr,8);
  DBG_OUTPUT_PORT.print(" \t");
  // read measurement data
  for (i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  if (OneWire::crc8(data, 8) != data[8])
  {
    
    logMsg(6,F(" Data CRC is not valid!"));
    return;
  }
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
  // print also as human readable mode  
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
  tempVal = (float)Whole;
  tempVal += ((float)Fract) / 100.0;
  if (SignBit) // If its negative
  {
    tempVal = 0 - tempVal;
  }
  doJob();
  DBG_OUTPUT_PORT.print(tempVal);
  if (tempVal != 85.00 && tempVal != -85)
  {
    DBG_OUTPUT_PORT.println("");
    updateSensorInfo(addr, tempVal, true);
    doLogic(addr, tempVal);
    pbSendTemp (addr,tempVal); // send temperature info
  } 
  else 
    DBG_OUTPUT_PORT.println(" ERR");
}

void readAllTempSensors()
{
  // Do conversion to all sensors and then read them one by one
    ds.reset();
    doJob();
    ds.skip();
    doJob();
    ds.write(0x44);
    pollSleep(750);
    SensorInfo *sensor;
    for (int i = 0; i < sensors.size(); i++)
    {
      sensor = sensors.get(i);
      if (sensor->local) 
      {
        readTempSensor(sensor->SensorAddress, false);
        
      }
    }
}


void markSensorsToOffline()
{
  SensorInfo *sensor;
  for (int i = 0; i < sensors.size(); i++)
  {
    sensor = sensors.get(i);
    if (sensor->local)
      sensor->online = false;
  }
}

boolean ByteArrayCompare(byte a[], byte b[], int array_size)
{
  for (int i = 0; i < array_size; ++i)
    if (a[i] != b[i])
      return(false);
  return(true);
}


void updateSensorInfo(byte address[8], float curTemp, bool local)
{
  SensorInfo *sensor;
  for (int i = 0; i < sensors.size(); i++)
  {
    sensor = sensors.get(i);
    if (ByteArrayCompare(sensor->SensorAddress, address, 8))
    {
      sensor->online = true;
      sensor->Temperature = curTemp;
      sensor->local = local;
      return;
    }
  }

  SensorInfo *newsensor = new SensorInfo();
  copyByteArray(address, newsensor->SensorAddress, 8);
  newsensor->online = true;
  newsensor->Temperature = curTemp;
  newsensor->local = local;
  sensors.add(newsensor);
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.println(F("Sensor added"));
}



void searchAllTempSensors()
{
  int count = 0;
  doJob();
  ds.reset_search();
  markSensorsToOffline();
  byte addr[8];
  while (ds.search(addr)) {
    // debug print sensor address
    DBG_OUTPUT_PORT.print(F("Found "));
    hexPrintArray(addr,8);
    DBG_OUTPUT_PORT.println("");
    if (OneWire::crc8(addr, 7) == addr[7]) 
    {
      if (addr[0] == 0x10) {
        //#DBG_OUTPUT_PORT.print("Device is a DS18S20 family device.\n");
        count++;
        readTempSensor(addr, true);
      }
      else if (addr[0] == 0x28) {
        count++;
        //#DBG_OUTPUT_PORT.print("Device is a DS18B20 family device.\n");
        readTempSensor(addr, true);
      }
      else {
        DBG_OUTPUT_PORT.print(F("Device family is not recognized: 0x"));
        DBG_OUTPUT_PORT.println(addr[0], HEX);
      }
    } 
    else 
    {
      DBG_OUTPUT_PORT.println(F(" Address CRC is not valid!"));
    }
  }
  DBG_OUTPUT_PORT.print("All ");
  DBG_OUTPUT_PORT.print(count);
  DBG_OUTPUT_PORT.println(F(" sensors found"));
}
void wwwSearchAllTempSernsors ()
{
  searchAllTempSensors();
  handle_sensors(false);
}









void InitWebserver()
{
  server.on("/settings", set_settings);
  server.on("/metrics", handle_prometheus);
  server.on("/tempsensors", handle_all_sensors);
  server.on("/tempsensors/all", handle_all_sensors);
  server.on("/tempsensors/local", handle_local_sensors);
  server.on("/tempsensors/update", searchAllTempSensors);
  
  
  server.begin();
  DBG_OUTPUT_PORT.println(F("Webserver started"));
}

void setup()
{
  
  //DBG_OUTPUT_PORT
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.println(F("Initialize ver 2 ...")); 
  loadConfig();
  // network services
 //Ethernet.begin(mac,ip,ip_dns,ip_gw,ip_mask);
  Ethernet.begin(mac,ip,ip_dns,ip_gw,ip_mask);
  
  InitWebserver();
  
  Udp.begin(localPort);
  DBG_OUTPUT_PORT.println(F("Ethernet up")); 
  pid.SetTunings(settings.pidP,settings.pidI,settings.pidD);
  

  //tell the PID to range 
  pid.SetOutputLimits(0, 100);
  //turn the PID on
  pid.SetMode(AUTOMATIC);
  
//  PrintSettings();
  //DBG_OUTPUT_PORT.println(F("To set system id press s"));
  Syslog.setLoghost(loghost,Udp);
  //Syslog.setLoghost(loghost);
  initLogic();
  searchAllTempSensors();
  logMsg(5,F("Controller started"));
  t.every(10000, ShowSockStatus);
  t.every(5000, readAllTempSensors); // read all known sensors at each 5 seconds
}


void loop()
{
  
  //readDBG_OUTPUT_PORT();
  // Read next onewire sensor and send values
  //readOnewireAndSendValues();
  t.update(); // low precision timer. Do this update only at there!
  doJob();
  server.handleClient();
  readEthernet();
  //Serial.print(".");
}




void handle_prometheus()
{
 int exceptedSize = 170;
 //int exceptedSize = (sensors.size() * 45) + 170;
 int sentSize = 0;
 server.setContentLength(exceptedSize);
 server.send(200, "text/plain", "");
 
 SensorInfo *sensor;
 String retbuf = "";
 /*
 for (int i = 0; i < sensors.size(); i++)
 {
   sensor = sensors.get(i);
   String sensorAddr;
   getHexString(sensor->SensorAddress, sensorAddr);
   retbuf="";
   retbuf+="tempsensor{address=\"";
   retbuf+=sensorAddr;
   retbuf+="\"} ";
   
   retbuf+=sensor->Temperature;
   retbuf+="\n";
   server.sendContent(retbuf);
   sentSize += retbuf.length();
 }
 */
   retbuf="";
   retbuf+="compressor{id=\"2\"} ";
   retbuf+= compressorRun ? "1\n" : "0\n";
   
   retbuf+="water_heating{id=\"2\"} ";
   retbuf+= state_water_heating ? "1\n" : "0\n";

   retbuf+="room_heating{id=\"2\"} ";
   
   retbuf+= state_room_heating ? "1\n" : "0\n";
sentSize += retbuf.length();
   server.sendContent(retbuf);
   retbuf = "";
   
   retbuf+="cooling{id=\"2\"} ";
   retbuf+= state_cooling ? "1\n" : "0\n";

   retbuf+="panicColdStopActive{id=\"2\"} ";
   retbuf+= panicColdStopActive ? "1\n" : "0\n";

   retbuf+="panicHotStopActive{id=\"2\"} ";
   retbuf+= panicHotStopActive ? "1\n" : "0\n";

   retbuf+="maintainceStop{id=\"2\"} ";
   retbuf+= maintainceStop ? "1\n" : "0\n";
  
   sentSize += retbuf.length();
   server.sendContent(retbuf);
   retbuf = "";
   
   while(exceptedSize>sentSize){
    retbuf += "\n";
    sentSize++;
   }
   server.sendContent(retbuf);
}


void handle_all_sensors()
{
  handle_sensors(true);
}
void handle_local_sensors()
{
  handle_sensors(false);
}
void handle_sensors(bool all)
{
   int exceptedSize = (sensors.size() * 48) + 2;
 int sentSize = 0;
 server.setContentLength(exceptedSize);
 server.send(200, "text/plain", "[");
 
 SensorInfo *sensor;
 String retbuf = "";
  for (int i = 0; i < sensors.size(); i++)
  {
    if (all && sensor->local)
    {
     retbuf = "";
     sensor = sensors.get(i);
     String sensorAddr;
     getHexString(sensor->SensorAddress, sensorAddr);
     retbuf+="{\"address\": \"";
     retbuf+=sensorAddr;
     retbuf+="\", \"temp\": ";
     retbuf+=sensor->Temperature;
     retbuf+="}";
     if (i + 1 < sensors.size())
        retbuf+=",\n";
     server.sendContent(retbuf);
     sentSize += retbuf.length();
    }
  }

 retbuf = "]";
 sentSize++;
 while(exceptedSize>sentSize){
  retbuf += "\n";
  sentSize++;
  }
 server.sendContent(retbuf);
}




int sendWebContent(String str)
{
  server.sendContent(str);
  return str.length();
}

void set_settings()
{

    //getArgumentValue("devicename", &settings.DeviceName); 
    
    getArgumentValue("pidSetpoint", &settings.pidSetpoint);
    getArgumentValue("pidP", &settings.pidP);
    getArgumentValue("pidI", &settings.pidI);
    getArgumentValue("pidD", &settings.pidD);
    getArgumentValue("requestedHotWaterTemp", &settings.requestedHotWaterTemp);
    getArgumentValue("heatingHysteresis", &settings.heatingHysteresis);
    getArgumentValue("requestedCoolWaterTemp", &settings.requestedCoolWaterTemp);
    getArgumentValue("coolingHysteresis", &settings.coolingHysteresis);
    getArgumentValue("warmWaterStartTemp", &settings.warmWaterStartTemp);
    getArgumentValue("warmWaterStopTemp", &settings.warmWaterStopTemp);
    getArgumentValue("compressorColdLimit", &settings.compressorColdLimit);
    getArgumentValue("compressorHotLimit", &settings.compressorHotLimit);
    
    setAddressConfigValue("pidSensorAddr", settings.pidSensorAddr);
    setAddressConfigValue("hotWaterSensorAddr", settings.hotWaterSensorAddr);
    setAddressConfigValue("coolingStartSensorAddr", settings.coolingStartSensorAddr);
    setAddressConfigValue("coolingStopSensorAddr", settings.coolingStopSensorAddr);
    setAddressConfigValue("warmWaterStartSensorAddr", settings.warmWaterStartSensorAddr);
    setAddressConfigValue("warmWaterStopSensorAddr", settings.warmWaterStopSensorAddr);
    setAddressConfigValue("compressorHotLimitSensorAddr", settings.compressorHotLimitSensorAddr);
    setAddressConfigValue("compressorColdLimitSensorAddr", settings.compressorColdLimitSensorAddr);
    getArgumentValue("maintainceStop", &maintainceStop);
    
  
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "save") 
        saveConfig();
    }

  int exceptedSize = 1000;
  int sentSize = 0;
  server.setContentLength(exceptedSize);
  server.sendHeader("Cache-Control", F("no-transform,public,max-age=6,s-maxage=6"));
  server.sendHeader(F("Access-Control-Allow-Origin"), "*");
  server.send(200, "text/plain", "{");

  String values = "";
  //jsonAddValue(values, "devicename", settings.DeviceName,true);
  jsonAddSValue(values, F("pidSensorAddr"), settings.pidSensorAddr,true);
  jsonAddSValue(values, F("hotWaterSensorAddr"), settings.hotWaterSensorAddr,true);
  jsonAddSValue(values, F("coolingStartSensorAddr"), settings.coolingStartSensorAddr,true);
  sentSize += sendWebContent(values);
  values = "";
  
  jsonAddSValue(values, F("coolingStopSensorAddr"), settings.coolingStopSensorAddr,true);
  jsonAddSValue(values, F("warmWaterStartSensorAddr"), settings.warmWaterStartSensorAddr,true);
  jsonAddSValue(values, F("warmWaterStopSensorAddr"), settings.warmWaterStopSensorAddr,true);
  jsonAddSValue(values, F("compressorHotLimitSensorAddr"), settings.compressorHotLimitSensorAddr,true);
  jsonAddSValue(values, F("compressorColdLimitSensorAddr"), settings.compressorColdLimitSensorAddr,true);
  sentSize += sendWebContent(values);
  values = "";
  
  jsonAddValue(values, F("pidSetpoint"), settings.pidSetpoint,true);
  jsonAddValue(values, F("pidP"), settings.pidP,true);
  jsonAddValue(values, F("pidI"), settings.pidI,true);
  jsonAddValue(values, F("pidD"), settings.pidD,true);
  sentSize += sendWebContent(values);
  values = "";
  
  jsonAddValue(values, F("requestedHotWaterTemp"), settings.requestedHotWaterTemp,true);
  jsonAddValue(values, F("heatingHysteresis"), settings.heatingHysteresis,true);
  jsonAddValue(values, F("requestedCoolWaterTemp"), settings.requestedCoolWaterTemp,true);
  jsonAddValue(values, F("coolingHysteresis"), settings.coolingHysteresis,true);
  jsonAddValue(values, F("warmWaterStartTemp"), settings.warmWaterStartTemp,true);
  sentSize += sendWebContent(values);
  values = "";
  
  jsonAddValue(values, F("warmWaterStopTemp"), settings.warmWaterStopTemp,true);
  jsonAddValue(values, F("compressorColdLimit"), settings.compressorColdLimit,true);
  jsonAddValue(values, F("compressorHotLimit"), settings.compressorHotLimit,true);  
  jsonAddValue(values, F("maintainceStop"), maintainceStop,false);
  values += "\n}";
  sentSize += sendWebContent(values);
  values ="";
  
  while(exceptedSize>sentSize){
    values += "\n";
    sentSize++;
  }
  server.sendContent(values);
}

void handle_unknown()
{
    server.send(404, "text/plain", F("FileNotFound"));
    DBG_OUTPUT_PORT.println("");
    DBG_OUTPUT_PORT.print(F("404 Not found: "));
    DBG_OUTPUT_PORT.println(server.uri());
     
}












