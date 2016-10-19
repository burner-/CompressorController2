#define DEBUG_ESP_HTTP_SERVER
#include <EthernetWebServer.h> //https://github.com/burner-/EthernetWebServer
EthernetWebServer server(80);  // create a server at port 80


// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}


String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
   char c;
   String ret = "";
   
   for(byte t=0;t<input.length();t++)
   {
     c = input[t];
     if (c == '+') c = ' ';
         if (c == '%') {


         t++;
         c = input[t];
         t++;
         c = (h2int(c) << 4) | h2int(input[t]);
     }
    
     ret.concat(c);
   }
   return ret;
  
}

void getArgumentValue(const char parmname[], boolean *target)
{
  
  if (server.hasArg(parmname))
  {
    
    String argVal = server.arg(parmname);
    if (argVal == "1" || argVal == "true")
    {
      DBG_OUTPUT_PORT.print(parmname);
      DBG_OUTPUT_PORT.println(" true");
      *target = true;
    }
    else 
    {
      DBG_OUTPUT_PORT.print(parmname);
      DBG_OUTPUT_PORT.println(" false");
      *target = false;
    }
  }
}
void getArgumentValue(const char parmname[], String *target)
{
  
  if (server.hasArg(parmname))
  {
    
    String argVal = server.arg(parmname);
    *target = urldecode(argVal);
  }
}

void getArgumentValue(const char parmname[], float *target)
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    *target = argVal.toFloat();
  }
}
void getArgumentValue(const char parmname[], double *target)
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    *target = argVal.toFloat();
  }
}

void getArgumentValue(const char parmname[], int *target)
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    *target = argVal.toInt();
  }
}
bool setAddressConfigValue(const char parmname[], byte target[])
{
  if (server.hasArg(parmname))
  {
    
    String argVal = server.arg(parmname);
    byte addr[8];
    if (argVal.length() == 16)
    {
      //logMsg(5,F("Sensor address change"));
      DBG_OUTPUT_PORT.print(parmname);
      DBG_OUTPUT_PORT.print(" = ");
      DBG_OUTPUT_PORT.println(argVal);
      
      argVal.toUpperCase();
      getAddressBytes(argVal, addr);
      copyByteArray(addr, target, 8);
      return true;
    }
    else
      DBG_OUTPUT_PORT.println(F("ignoring bogus sensor address"));
  }
  return false;
}





void jsonAddValue (String &string, String valuename, String value, bool newline)
{
  string += "\"" + valuename  + "\" : \"" + value + "\"" ;
  if (newline) string += ",\n";
}

void jsonAddSValue (String &string, String valuename, byte value[], bool newline)
{
  String sensorAddr;
  getHexString(value, sensorAddr);
  jsonAddValue(string, valuename, sensorAddr, newline);
}

void jsonAddValue (String &string, String valuename, float value, bool newline)
{
  string += "\"" + valuename  + "\" : \"" + (String) value + "\"" ;
  if (newline) string += ",\n";
}

void jsonAddValue (String &string, String valuename, double value, bool newline)
{
  string += "\"" + valuename  + "\" : \"" + (String) value + "\"" ;
  if (newline) string += ",\n";
}

void jsonAddValue (String &string, String valuename, int value, bool newline)
{
  string += "\"" + valuename  + "\" : \"" + (String) value + "\"" ;
  if (newline) string += ",\n";
}

void jsonAddValue (String &string, String valuename, boolean value, bool newline)
{
  string += "\"" + valuename  + "\" : ";
  if (value)
    string += "true"; 
  else 
    string += "false";
  if (newline) string += ",\n";
}

