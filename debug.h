#include <Syslog.h> //https://github.com/burner-/Syslog
#include <utility/w5100.h>
void SendSyslog(uint8_t loglevel, const char message[])
{
 Syslog.logger(1,loglevel,HOSTNAME, message);
}
void SendSyslog(uint8_t loglevel, const __FlashStringHelper *ifsh)
{
  Syslog.logger(1,loglevel,HOSTNAME, ifsh);
}

void logMsg(int level, const __FlashStringHelper *ifsh)
{
          DBG_OUTPUT_PORT.println(ifsh);
         SendSyslog(level,ifsh);
}

void logMsg(int level, char msg[])
{
          DBG_OUTPUT_PORT.println(msg);
          SendSyslog(level,msg);
}
void dumpType(byte type)
{
  if (type == TYPE_SENDERROR)
    DBG_OUTPUT_PORT.print(F("SENDERROR"));
  else if (type == TYPE_1WIRE)
    DBG_OUTPUT_PORT.print(F("1WIRE"));
  else if (type == TYPE_PID_INFO)
    DBG_OUTPUT_PORT.print(F("PID_INFO"));
  else if (type == TYPE_PID_SET_ADDRESS)
    DBG_OUTPUT_PORT.print(F("PID_SET_ADDRESS"));
  else if (type == TYPE_PID_SET_SETPOINT)
    DBG_OUTPUT_PORT.print(F("PID_SET_SETPOINT"));
  else if (type == TYPE_PID_SET_TUNINGS)
    DBG_OUTPUT_PORT.print(F("PID_SET_TUNINGS"));
  else if (type == TYPE_SET_COMPRESSOR_SETTINGS)
    DBG_OUTPUT_PORT.print(F("TYPE_SET_COMPRESSOR_SETTINGS"));
  else if (type == TYPE_PB_TEMP_INFO)
    DBG_OUTPUT_PORT.print(F("TYPE_PB_TEMP_INFO"));
    
  else
    DBG_OUTPUT_PORT.print(type);
}
byte socketStat[MAX_SOCK_NUM];

void ShowSockStatus()
{
  for (int i = 0; i < MAX_SOCK_NUM; i++) {
    Serial.print(F("Socket#"));
    Serial.print(i);
    uint8_t s = W5100.readSnSR(i);
    socketStat[i] = s;
    Serial.print(F(":0x"));
    Serial.print(s,16);
    Serial.print(F(" "));
    Serial.print(W5100.readSnPORT(i));
    Serial.print(F(" D:"));
    uint8_t dip[4];
    W5100.readSnDIPR(i, dip);
    for (int j=0; j<4; j++) {
      Serial.print(dip[j],10);
      if (j<3) Serial.print(".");
    }
    Serial.print(F("("));
    Serial.print(W5100.readSnDPORT(i));
    Serial.println(F(")"));
  }
}

