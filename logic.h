// Ground sircullation mode
#define GROUND_SINK 1
#define GROUND_SOURCE 2
int groundmode = GROUND_SINK;

// work mode states
bool state_water_heating = false; // hot water heating
bool state_room_heating = false; //room radioators need more heat
bool state_cooling = false; // compressor running
bool compressorRun = false;
bool panicColdStopActive = false;
bool panicHotStopActive = false;
boolean maintainceStop = false;


float pidSensor; //Sensor address for hot water pid controlling
float hotWaterSensor; //Sensor address for hot water heating request
float coolingStartSensor; //Sensor address for cooling start
float coolingStopSensor; //Sensor address for cooling stop
float warmWaterStartSensor; //Sensor address for warming start
float warmWaterStopSensor; //Sensor address for warming stop
float compressorHotLimitSensor; //Sensor address for emergency stop
float compressorColdLimitSensor; //Sensor address for emergency stop

void OpenFlowValve()
{
  logMsg(6,F("Flow valve opened"));
  digitalWrite(PIN_FLOW_VALVE, LOW);
}

void StartCompressor()
{
  pbSendCompressorInfo();
  //logMsg(6,"Compressor start");
  if ( panicHotStopActive || panicColdStopActive)
  {
//    //SendSyslog(5,"ERROR: prevent compressor start at panic stop state.");
    pollSleep(10000);
    return;
  }
  compressorRun = true;
  OpenFlowValve();
  
  logMsg(5,F("Compressor started"));
  digitalWrite(PIN_COMPRESSOR_RUN, LOW);
}

void ServoDetach()
{
  //DBG_OUTPUT_PORT.println("Servo released");
  servo.detach();
}
int lastServoval = 0;
void SetServo(double val)
{
  char tmpString[10];  //  Hold The Convert Data
  dtostrf(val,2,2,tmpString); 
  //logMsg(6,strcat("Servo turn ",tmpString));
  int servoval = (int) val;
  servoval = map(servoval, 0, 100, settings.servoMin, settings.servoMax);
  if (lastServoval != servoval){
    servo.attach(PIN_MODE_VALVE_SERVO);
    servo.write(servoval);
    t.after(2000, ServoDetach);
    lastServoval = servoval;
  }
}


void StopWaterHeating()
{
  logMsg(6,F("Water heating stopped"));
  state_water_heating = false;
  SetServo(SERVO_WARM_WATER);
}

void CloseFlowValve()
{
  if (compressorRun == false)
  {
    logMsg(6,F("Flow valve closed"));
    digitalWrite(PIN_FLOW_VALVE, HIGH);
  } 
  else 
  {
    //logMsg(6,"Flow valve close ignored (compressor running)");
    digitalWrite(PIN_FLOW_VALVE, LOW);
  }

}

void StopCompressor()
{
  StopWaterHeating(); // Start compressor next time in cooler exthaus enviroment
  compressorRun = false;
  pbSendCompressorInfo();
  logMsg(6,F("Compressor stopped"));
  digitalWrite(PIN_COMPRESSOR_RUN, HIGH);
  t.after(10000, CloseFlowValve); // After 10 seconds close flow valve
}

void setGroundMode(int mode)
{
  groundmode = mode;
}


void RequestCompressorStop()
{
  if (!state_room_heating && !state_cooling) // stop compressor if no need for warming or cooling
  { 
    setGroundMode(GROUND_SINK);
    StopCompressor();
  } 
  else 
  {
    if (state_room_heating)
    {
      logMsg(6, F( "Stop Cooling continue room heating"));
      setGroundMode(GROUND_SOURCE);
    }
    else if (state_cooling)
    {
      logMsg(6, F("Stop room heating continue cooling"));
      setGroundMode(GROUND_SINK);
    }
  }
}

void StartCooling()
{
  state_cooling = true;
  logMsg(6,F("Start Cooling"));
  pbSendCompressorInfo();
  StartCompressor();
}

void StopCooling()
{
  state_cooling = false;
  logMsg(6,F("Stop Cooling"));
  pbSendCompressorInfo();
  RequestCompressorStop();
}

void StartWarming()
{
  state_room_heating = true;
  logMsg(6,F("Start room heating"));
  pbSendCompressorInfo();
  StartCompressor();
}



void StopWarming()
{
  state_room_heating = false;
  logMsg(6,F("Stop room heating"));
  pbSendCompressorInfo();
  RequestCompressorStop();
}




void StartWaterHeating()
{
  state_water_heating = true;
  heating_isStarting = false;
  SetServo(SERVO_HOT_WATER);
  logMsg(6,F("Water heating started"));
  pbSendCompressorInfo();
}


void PanicStop()
{
  StopWaterHeating();
  StopCooling();    
  StopCompressor(); // this is really force shutdown for compressor
  state_cooling = false;
  state_room_heating = false;   
  pbSendCompressorInfo();     
}

bool inPanic(){
  return (panicHotStopActive || panicColdStopActive);
}


void doLogic(byte addr[8], float tempVal)
{
    if (maintainceStop)
    {
      if (state_cooling || state_room_heating)
      {
        logMsg(6,F("Moving to maintainceStop"));
        PanicStop();
      }
      return;
    }
    // Check panic values
    if (matchArray(addr, settings.compressorHotLimitSensorAddr,0,7))
    {
      compressorHotLimitSensor = tempVal;
      if (tempVal > settings.compressorHotLimit)
      {
        if (!panicHotStopActive)
          logMsg(6,F("Hot water over temp. Panic stop"));
        panicHotStopActive = true;
        PanicStop();
      } 
      else
      {
        if (panicHotStopActive)
          logMsg(6,F("Hot water temp back in range. Panic cancelled"));
        panicHotStopActive = false;
      }
    } 
    if (matchArray(addr, settings.compressorColdLimitSensorAddr,0,7))
    {
      compressorColdLimitSensor = tempVal;
      if (tempVal < settings.compressorColdLimit)
      {
       if (!panicColdStopActive)
          logMsg(6,F("Cold water under temp. Panic stop"));
        panicColdStopActive = true;
        PanicStop();
      }
      else
      {
        if (panicColdStopActive)
          logMsg(6,F("Cold water temp back in range. Panic cancelled"));
          
        panicColdStopActive = false;
      }
    }
    
    
    // check if sensor is pid input sensor
    if (matchArray(addr, settings.pidSensorAddr,0,7))
    {
      pidSensor = tempVal;
      if (state_water_heating)
      {
        pidInput = tempVal;
        pid.Compute();
//        sendPidReport();
        SetServo(pidOutput);
      } 
      else 
      {
        SetServo(SERVO_WARM_WATER); // Heating off
      }
    }
     
    if (matchArray(addr, settings.hotWaterSensorAddr,0,7))
    {
      hotWaterSensor = tempVal;
      
      if(state_water_heating)
      {
        //DBG_OUTPUT_PORT.println("water_heating = true");
        if (settings.requestedHotWaterTemp + settings.heatingHysteresis < hotWaterSensor)
          StopWaterHeating();
      }
      else 
      {
        //DBG_OUTPUT_PORT.println("water_heating = false");
        if (settings.requestedHotWaterTemp > tempVal 
          && state_cooling // open heating valve only if there is cooling need
          && !heating_isStarting // move to heating mode only once
          )
        {
          logMsg(6,F("Need for water heating"));
          t.after(10000, StartWaterHeating); // after 10 secods turn to water heating mode
          heating_isStarting = true;
        }
      }
    }

    // Cooling
    if (!state_cooling && matchArray(addr, settings.coolingStartSensorAddr,0,7))
    {
      coolingStartSensor = tempVal;
      
        if (settings.requestedCoolWaterTemp < tempVal && !inPanic())
        {
          logMsg(7,F("Cooling temperature warmer than requestedCoolWaterTemp"));
          StartCooling();
        }
    }
    else if (state_cooling && matchArray(addr, settings.coolingStopSensorAddr,0,7))
    {
      coolingStopSensor = tempVal;
      if (settings.requestedCoolWaterTemp - settings.coolingHysteresis > tempVal)
      {
        logMsg(7,F("Cooling temperature reached requestedCoolWaterTemp - coolingHysteresis "));
        StopCooling();
      }
    }
    
    // Room heating
    if (!state_room_heating && matchArray(addr, settings.warmWaterStartSensorAddr,0,7))
    {
      warmWaterStartSensor = tempVal;
      if (settings.warmWaterStartTemp > tempVal && !inPanic())
      {
        logMsg(7,F("Warming temp lower than warmWaterStartTemp"));
        StartWarming();
      }
    }
    else if (state_room_heating && matchArray(addr, settings.warmWaterStopSensorAddr,0,7))
    {
      warmWaterStopSensor = tempVal;
      if (settings.warmWaterStopTemp < tempVal)
      {
        logMsg(7,F("Warming temp higher than warmWaterStartTemp"));
        StopWarming();
      }
    }
}

void initLogic(){
  pinMode(PIN_COMPRESSOR_RUN, OUTPUT);
  pinMode(PIN_FLOW_VALVE, OUTPUT);
}


//
// Logic end
//
