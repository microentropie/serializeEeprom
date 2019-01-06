#ifndef _SERIALIZE_EEPROM_
#define _SERIALIZE_EEPROM_

/*
Library: serialize structure/class to/from EEPROM.
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

Sources repository: https://github.com/microentropie/serializeEeprom
How to install:
  https://www.arduino.cc/en/guide/libraries
*/

/*
The various Arduino and Genuino boards have different EEPROM sizes:
512 bytes on the ATmega168 and ATmega8
1024 bytes on the ATmega328,
The Arduino and Genuino 101 boards have an emulated EEPROM space of 1024 bytes.
4096 bytes on the ATmega1280 and ATmega2560
4096 bytes on ESP8266
0-64 bytes on ESP32 (see 'serializeEeprom-Esp32.cpp' for more details)
*/

#define EEPROM_log_NO 0
#define EEPROM_log_RW 1
#define EEPROM_log_VERBOSE 2

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif //uint32_t
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif //uint16_t
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif //uint8_t

//#include <c_types.h>

class serialize2eeprom
{
public:
  static bool Save(unsigned char logLevel, uint32_t dataSignature, uint16_t baseOffset, void *p, uint16_t dataLen);
  static bool Load(unsigned char logLevel, uint32_t dataSignature, uint16_t baseOffset, void *p, uint16_t dataLen);
  static bool Init();
private:
  static unsigned char checksum(void *pBuf, uint16_t sz);
};



template<typename T>
class eepromIf
{
public:
  static bool Save(T *p, unsigned char logLevel);

  static bool Load(T *p, unsigned char logLevel);

  static uint32_t Signature;
  static uint16_t baseOffset;
};

#include "char-int-char.h"

// here is the implementation;
// it is here because they are templates:
template<typename T> bool eepromIf<T>::Save(T *p, unsigned char logLevel)
{
  return serialize2eeprom::Save(logLevel, Signature, baseOffset, p, sizeof(T));
}

template<typename T> bool eepromIf<T>::Load(T *p, unsigned char logLevel)
{
  return serialize2eeprom::Load(logLevel, Signature, baseOffset, p, sizeof(T));
}

// default initialization (put in your code):
//template<> int eepromIf<T>::Signature  = 0x53744450;
//template<> int eepromIf<T>::baseOffset = 0;

#endif //_SERIALIZE_EEPROM_
