void hexPrint(byte b)
{
  if (b < 0x10)
    DBG_OUTPUT_PORT.print("0");  
  DBG_OUTPUT_PORT.print(b, HEX);
}

void copyByteArray(byte inBytes[], byte outBytes[], int lenght )
{
  if (lenght >8) 
    lenght = 8;
  for(int i = 0; i<lenght ; i++)
  {
    outBytes[i] = inBytes[i];
  }
}

byte getHexVal(byte c)
{
  if (c >= '0' && c <= '9')
    return (byte)(c - '0');
  else
    return (byte)(c - 'A' + 10);
}
void getAddressBytes(String string, byte retBytes[])
{
  //string += "AA";
  byte byteArr[18];
  string.getBytes(byteArr, 17); //use over sized buffer & value because that ass rape shit function corrupt last byte
  int bytepos = 0;
  for (int i = 0; i < 16; i += 2)
  {
    byte b = getHexVal(byteArr[i + 1]) + (getHexVal(byteArr[i]) << 4);
    retBytes[bytepos] = b;
    bytepos++;
  }
  DBG_OUTPUT_PORT.println("");
  return;
}

void getHexString(byte bytes[], String &retstring)
{
        retstring = "";
  for (int i = 0; i < 8; i++)
  {
    byte b = bytes[i];
    if (b < 0x10)
      retstring += "0";// zero pad to two digit
    retstring += String(b, HEX);
  }
  retstring.toUpperCase();
  return;
}


bool matchArray(byte arrayA[], byte arrayB[],int from, int to)
{
  for(byte i=from;i<=to;i++)
  {
    if (arrayB[i] != arrayA[i])
      return false;
  }
  return true;
}
