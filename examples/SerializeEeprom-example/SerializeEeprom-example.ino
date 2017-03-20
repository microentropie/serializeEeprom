#include <serializeEeprom.h>

/*
Library: serialize structure/class to/from EEPROM.
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

Sources repository: https://github.com/microentropie/serializeEeprom
How to install:
  https://www.arduino.cc/en/guide/libraries
*/


// the structure (or the class) that holds the data
// pay attention NOT to insert pointers !!!
// all the data willlbe saved in binary format to EEPROM.
// As data is locally (in EEPROM) saved, the use of binary format greatly simplifies serialization.
// Thus this serialization is not portable !!
struct CookingSession
{
  int waitTimeMin;
  int cookingTimeMin;
  int keepWarmTimeMin;
};

// define the SAVE implementation for the above structure
bool CookingSession_Save(CookingSession &cfg, unsigned char logLevel = EEPROM_log_RW)
{
  return eepromIf<CookingSession>::Save(&cfg, logLevel);
}

// define the LOAD implementation for the above structure
bool CookingSession_Load(CookingSession &cfg, unsigned char logLevel = EEPROM_log_RW)
{
  return eepromIf<CookingSession>::Load(&cfg, logLevel);
}

// Last requirement:
// structure is saved at a phisical location (...::baseOffset) with
// a signature (...::Signature) and a checksum (computed inside the library),
// this avoids loading invalid data.
// You always must supply address and Signature.
// Data is static, you can only save 1 data session for each type.
//                                                      C S E S
template<> int eepromIf<CookingSession>::Signature = 0x43534553;
template<> int eepromIf<CookingSession>::baseOffset = 300;



void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("Save-to/load-from EEPROM. Example:");
  Serial.println("https://github.com/microentropie/serializeEeprom");
}

void loop()
{
  CookingSession mySession;

  // init:
  mySession.waitTimeMin = 10;
  mySession.cookingTimeMin = 35;
  mySession.keepWarmTimeMin = 2;
  
  // save to EEPROM:
  CookingSession_Save(mySession); // log to serial
  //CookingSession_Save(mySession, EEPROM_log_NO); // no log at all

  // show current data:
  ShowData("Data saved to EEPROM", mySession);
  
  // reset data:
  mySession.waitTimeMin = 1;
  mySession.cookingTimeMin = 2;
  mySession.keepWarmTimeMin = 3;

  // show changed session data:
  ShowData("Data changed", mySession);
  
  // retrieve from EEPROM:
  CookingSession_Load(mySession); // log to serial
  //CookingSession_Load(mySession, EEPROM_log_NO); // no log at all

  // show current data:
  ShowData("Data retrieved from EEPROM", mySession);
  
  Serial.println("-------------------------");
 
  delay(30000);
}
/*
 * output from this sketch
Bubble Sort example 1
https://github.com/microentropie/sort
Original Array:
2,5,6,7,123,-3,10,20,171,14,
Sorted Array:
-3,2,5,6,7,10,14,20,123,171,
Max: 171, Min: -3
*/

// show session data:
void ShowData(const char *msg, CookingSession &mySession)
{
  Serial.print(msg); Serial.println(":");
  Serial.print("waitTimeMin = ");     Serial.println(mySession.waitTimeMin);
  Serial.print("cookingTimeMin = ");  Serial.println(mySession.cookingTimeMin);
  Serial.print("keepWarmTimeMin = "); Serial.println(mySession.keepWarmTimeMin);
  Serial.println();
}
