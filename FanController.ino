//#define DEBUG

#define VERSION "Attic Fan Controller v1.5 20130527"

#include <dht11.h>
#include <SimpleTimer.h>
#include <x10.h>

// Initialize timer
SimpleTimer timer;

// Thermo Hygrometers
// DHT 11 wiring directions:
//  Connect pin 1 (on the left) of the sensor to +5V
//  Connect pin 2 of the sensor to whatever your DHTPIN is
//  Connect pin 4 (on the right) of the sensor to GROUND
//  Connect a 4.7K-20K resistor from pin 2 (data) to pin 1 (power) of the sensor


#define DHTPIN_1   2  // DHT11 #1
#define DHTPIN_2   3  // DHT11 #2
#define DHTPIN_3   8  // DHT11 #3
                      // pin4 burned on protoshield

#define rxPin 7    // data receive pin
#define txPin 6    // data transmit pin
#define zcPin 5    // zero crossing pin

dht11 AtticDHT_1;  // inside Attic space 1
dht11 AtticDHT_2;  // inside Attic space 2
dht11 OutsideDHT;  // outside house

int Attic1_chk = 0;
int Attic2_chk = 0;
int Outside_chk = 0;


// X10 related values
#define Fan1_HC    C                // Fan_1 on house code C
#define Fan1_UC    UNIT_1           // Fan_1 on unit code 1
#define Fan2_HC    C                // Fan_1 on house code C
#define Fan2_UC    UNIT_2           // Fan_1 on unit code 1
#define Attic1_chk_HC C
#define Attic1_chk_UC UNIT_3
#define Attic2_chk_HC C
#define Attic2_chk_UC UNIT_4
#define Outside_chk_HC C
#define Outside_chk_UC UNIT_5



// general constants
#ifdef DEBUG
//debug
  #define TriggerT 90                 // Trigger temperature degF
  #define TriggerH 60                 // Trigger humidity %RH
  #define TriggerDeltaT 20            // Minimum temperature delta degF
  #define TriggerDeltaH 5             // minimum humidity delta percent %RH
  #define SensorCheckTime 5000
  #define FanRunTime 5000
  #define InitFanRunTime 5000
  #define FanDelayTime 10000
  #define ReportSensorTime 30000
  #define X10CommandDelay 500         // delay after sending command to avoid collisions
#else
//standard
  #define TriggerT 90                 // Trigger temperature degF
  #define TriggerH 60                 // Trigger humidity %RH
  #define TriggerDeltaT 20            // Minimum temperature delta degF
  #define TriggerDeltaH 10            // minimum humidity delta percent %RH
  #define SensorCheckTime 30000       //30 seconds
  #define FanRunTime 600000           //10 minutes
  #define InitFanRunTime 3600000      //60 minutes
  #define FanDelayTime 1800000        //30 minutes
                                      // only let fan run after delay to keep it from cycling
  #define ReportSensorTime 3600000
  #define X10CommandDelay 500         // delay after sending command to avoid collisions
#endif

// sensor offsets
#define Attic1T_offset 0
#define Attic1H_offset 0
#define Attic2T_offset 0
#define Attic2H_offset 0
#define OutsideT_offset 3.6
#define OutsideH_offset 3
//backup sensors
//#define Attic1T_offset 2
//#define Attic1H_offset 13
//#define Attic2T_offset -1
//#define Attic2H_offset 10
//#define OutsideT_offset 0
//#define OutsideH_offset 0


int Fan1_on = 0;      // 0=off/1=on
int Fan1_delay = 0;   // 0=off/1=on

int Fan2_on = 0;      // 0=off/1=on
int Fan2_delay = 0;   // 0=off/1=on


void setup()
{
  Serial.begin(9600); 
  Serial.println(VERSION);
  #ifdef DEBUG
    Serial.println("Debug Mode");
  #endif
  
  delay(X10CommandDelay);
  
  x10.begin(rxPin, txPin, zcPin);
  Serial.println("X10 active");
  Serial.println("Start InitializeSensors");
  InitializeSensors();
  Serial.println("End InitializeSensors");
  timer.setInterval(SensorCheckTime,CheckSensors);
  timer.setInterval(ReportSensorTime,ReportSensorState);
}


void loop() 
{
  timer.run();
}



void CheckSensors()
{
  float AtticT1 = 0.;
  float AtticH1 = 0.;
  float AtticT2 = 0.;
  float AtticH2 = 0.;
  float OutsideT = 0.;
  float OutsideH = 0.;
  
  Attic1_chk = AtticDHT_1.read(DHTPIN_1);
  //Serial.print("Read Attic1 Sensor: ");
  switch (Attic1_chk)
  {
    case DHTLIB_OK:
                //Serial.println("OK");
                break;
    case DHTLIB_ERROR_CHECKSUM:
                Serial.println("Attic1 Sensor: Checksum error");
                break;
    case DHTLIB_ERROR_TIMEOUT:
                Serial.println("Attic1 Sensor: Time out error");
                break;
    default:
                Serial.println("Attic1 Sensor: Unknown error");
                break;
  }
  
  if (Attic1_chk == DHTLIB_OK)
  {
    //Serial.println("Attic1 set temp/humidity");
    AtticT1 = Fahrenheit((float)AtticDHT_1.temperature)+Attic1T_offset;
    AtticH1 = (float)AtticDHT_1.humidity+Attic1H_offset;
  }
  else
  {
    //Serial.println("Attic1 sensor broken");
    AtticT1 = -999;
    AtticH1 = -999;
  }
  
  Attic2_chk = AtticDHT_2.read(DHTPIN_2);
  //Serial.print("Read Attic2 Sensor: ");
  switch (Attic2_chk)
  {
    case DHTLIB_OK:
                //Serial.println("OK");
                break;
    case DHTLIB_ERROR_CHECKSUM:
                Serial.println("Attic2 Sensor: Checksum error");
                break;
    case DHTLIB_ERROR_TIMEOUT:
                Serial.println("Attic2 Sensor: Time out error");
                break;
    default:
                Serial.println("Attic2 Sensor: Unknown error");
                break;
  }
  
  if (Attic2_chk == DHTLIB_OK)
  {
    //Serial.println("Attic2 set temp/humidity");
    AtticT2 = Fahrenheit((float)AtticDHT_2.temperature)+Attic2T_offset;
    AtticH2 = (float)AtticDHT_2.humidity+Attic2H_offset;
  }
  else
  {
    //Serial.println("Attic2 sensor broken");
    AtticT2 = -999;
    AtticH2 = -999;
  }
  
  Outside_chk = OutsideDHT.read(DHTPIN_3);

  //Serial.print("Read Outside Sensor: ");
  switch (Outside_chk)
  {
    case DHTLIB_OK:
                //Serial.println("OK");
                break;
    case DHTLIB_ERROR_CHECKSUM:
                Serial.println("Outside Sensor: Checksum error");
                break;
    case DHTLIB_ERROR_TIMEOUT:
                Serial.println("Outside Sensor: Time out error");
                break;
    default:
                Serial.println("Outside Sensor: Unknown error");
                break;
  }
  
  if (Outside_chk == DHTLIB_OK)
  {
    //Serial.println("Outside sensor OK");
    OutsideT = Fahrenheit((float)OutsideDHT.temperature)+OutsideT_offset;
    OutsideH = (float)OutsideDHT.humidity+OutsideH_offset;
  }
  else
  {
    //Serial.println("Outside sensor broken");
    OutsideT = -999;
    OutsideH = -999;
  }
  
  Serial.print("Attic_1\tT ");
  Serial.print(AtticT1, 2);
  Serial.print("\tH ");
  Serial.println(AtticH1, 2);
  
  Serial.print("Attic_2\tT ");
  Serial.print(AtticT2, 2);
  Serial.print("\tH ");
  Serial.println(AtticH2, 2);

  Serial.print("Outside\tT ");
  Serial.print(OutsideT, 2);
  Serial.print("\tH ");
  Serial.println(OutsideH, 2);
  Serial.println();
  
  if(Fan1_on==0 && AtticT1>TriggerT && OutsideT+TriggerDeltaT<AtticT1 && Attic1_chk==DHTLIB_OK 
     && Outside_chk==DHTLIB_OK && Fan1_delay==0)
  {
    // Attic1 temperatature trigger
    Serial.println("Attic1: temperature trigger");
    TurnOnFan_1();
  }
  
  if(Fan1_on==0 && AtticH1>TriggerH && RHconvert(OutsideT,OutsideH,AtticT1)+TriggerDeltaH<AtticH1 
     && Attic1_chk==DHTLIB_OK && Outside_chk==DHTLIB_OK && Fan1_delay==0)
  {
    // Attic1 humidity trigger
    Serial.println("Attic1: humidity trigger");
    TurnOnFan_1();
  }
  
  if(Fan2_on==0 && AtticT2>TriggerT && OutsideT+TriggerDeltaT<AtticT2 && Attic2_chk==DHTLIB_OK 
     && Outside_chk==DHTLIB_OK && Fan2_delay==0)
  {
    // Attic2 temperatature trigger
    Serial.println("Attic2: temperature trigger");
    TurnOnFan_2();
  }
  
  if(Fan2_on==0 && AtticH2>TriggerH && RHconvert(OutsideT,OutsideH,AtticT2)+TriggerDeltaH<AtticH2 
     && Attic2_chk==DHTLIB_OK && Outside_chk==DHTLIB_OK && Fan2_delay==0)
  {
    // Attic2 humidity trigger
    Serial.println("Attice2: humidity trigger");
    TurnOnFan_2();
  }
}



void ReportSensorState()
{
  Serial.print("Attic1_chk - ");
  Serial.println(Attic1_chk);
  Serial.print("Attic2_chk - ");
  Serial.println(Attic2_chk);
  Serial.print("Outside_chk - ");
  Serial.println(Outside_chk);
  
  if (Attic1_chk == DHTLIB_OK)
  {
    Serial.println("Attic1 sensor OK");
  }
  else
  {
    Serial.println("Attic1 sensor Broken");
    x10.beginTransmission(Attic1_chk_HC);
    x10.write(Attic1_chk_UC);
    delay(X10CommandDelay);
    x10.write(ON);
    delay(X10CommandDelay);
    x10.endTransmission();
  }

  if (Attic2_chk == DHTLIB_OK)
  {
    Serial.println("Attic2 sensor OK");
  }
  else
  {
    Serial.println("Attic2 sensor Broken");
    x10.beginTransmission(Attic2_chk_HC);
    x10.write(Attic2_chk_UC);
    delay(X10CommandDelay);
    x10.write(ON);
    delay(X10CommandDelay);
    x10.endTransmission();
  }
  
  if (Outside_chk == DHTLIB_OK)
  {
    Serial.println("Outside sensor OK");
  }
  else
  {
    Serial.println("Outside sensor Broken");
    x10.beginTransmission(Outside_chk_HC);
    x10.write(Outside_chk_UC);
    delay(X10CommandDelay);
    x10.write(ON);
    delay(X10CommandDelay);
    x10.endTransmission();
  }
  Serial.println();
}


void TurnOnFan_1()
{
  // turn fan1 on and start run timer
  Serial.println("Turn ON Fan 1");
  timer.setTimeout(FanRunTime,TurnOffFan_1);
  Fan1_on=1;
  x10.beginTransmission(Fan1_HC);
  x10.write(Fan1_UC);
  delay(X10CommandDelay);
  x10.write(ON);
  delay(X10CommandDelay);
  x10.endTransmission();
}


void TurnOnFan_2()
{
  // turn fan1 on and start run timer
  Serial.println("Turn ON Fan 2");
  timer.setTimeout(FanRunTime,TurnOffFan_2);
  Fan2_on=1;
  x10.beginTransmission(Fan2_HC);
  x10.write(Fan2_UC);
  delay(X10CommandDelay);
  x10.write(ON);
  delay(X10CommandDelay);
  x10.endTransmission();
}


void TurnOffFan_1()
{
  // turn off fan1 and start delay timer
  Serial.println("Turn OFF Fan 1");
  Fan1_on=0;
  Serial.println("Fan1 delay time START");
  timer.setTimeout(FanDelayTime,FanDelayTime_1);
  Fan1_delay=1;
  x10.beginTransmission(Fan1_HC);
  x10.write(Fan1_UC);
  delay(X10CommandDelay);
  x10.write(OFF);
  delay(X10CommandDelay);
  x10.endTransmission();
}

void TurnOffFan_2()
{
  // turn off fan2 and start delay timer
  Serial.println("Turn OFF Fan 2");
  Fan2_on=0;
  Serial.println("Fan2 delay time START");
  timer.setTimeout(FanDelayTime,FanDelayTime_2);
  Fan2_delay=1;
  x10.beginTransmission(Fan2_HC);
  x10.write(Fan2_UC);
  delay(X10CommandDelay);
  x10.write(OFF);
  delay(X10CommandDelay);
  x10.endTransmission();
}

void FanDelayTime_1()
{
  // turn off fan1 delay time
  Serial.println("Fan1 delay time END");
  Fan1_delay=0;
}

void FanDelayTime_2()
{
  // turn off fan2 delay time
  Serial.println("Fan2 delay time END");
  Fan2_delay=0;
}


void InitializeSensors()
{
//  turn fans on for InitFanRunTime
  Serial.println("Turn fans ON : InitializeSensors");
  x10.beginTransmission(Fan1_HC);
  x10.write(Fan1_UC);
  delay(X10CommandDelay);
  x10.write(ON);
  delay(X10CommandDelay);
  x10.endTransmission();

  x10.beginTransmission(Fan2_HC);
  x10.write(Fan2_UC);
  delay(X10CommandDelay);
  x10.write(ON);
  delay(X10CommandDelay);
  x10.endTransmission();
  
  delay(InitFanRunTime);
  
//  assume inside and outside air temp/humidity are the same
  
//  turn off fans
  Serial.println("Turn fans OFF : InitializeSensors");
  x10.beginTransmission(Fan1_HC);
  x10.write(Fan1_UC);
  delay(X10CommandDelay);
  x10.write(OFF);
  delay(X10CommandDelay);
  x10.endTransmission();

  x10.beginTransmission(Fan2_HC);
  x10.write(Fan2_UC);
  delay(X10CommandDelay);
  x10.write(OFF);
  delay(X10CommandDelay);
  x10.endTransmission();
  
  timer.setTimeout(FanDelayTime,FanDelayTime_1);
  timer.setTimeout(FanDelayTime,FanDelayTime_2);
  Fan1_delay=1;
  Fan2_delay=1;
}


// *************************************************************
// Helper functions
double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32;
}

double Celcius(double Fahrenheit)
{
  return (Fahrenheit - 32) / 1.8;
}


double RHconvert(double TempF_1, double Humidity, double TempF_2)
{
  // Convert Humidity from temperature T1 to T2
  // TempF_1 in degF
  // Humidity in %RH
  // TempF_2 in degF
  
  double T1 = Celcius(TempF_1);
  double T2 = Celcius(TempF_2);
  
  double VaporDensity1 = 5.018 + 0.32321 * T1 + 0.0081847 * pow(T1,2) + 0.00031243 * pow(T1,3);
  double AbsHumidity = Humidity * VaporDensity1 / 100.0;
  double VaporDensity2 = 5.018 + 0.32321 * T2 + 0.0081847 * pow(T2,2) + 0.00031243 * pow(T2,3);
  return (double)(AbsHumidity / VaporDensity2 * 100.0);
return 1.0;
}
