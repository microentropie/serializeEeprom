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

#define EEPROM_log_NO 0
#define EEPROM_log_RW 1
#define EEPROM_log_VERBOSE 2

class serialize2eeprom
{
public:
  static bool Save(unsigned char logLevel, int dataSignature, int baseOffset, void *p, int dataLen);
  static bool Load(unsigned char logLevel, int dataSignature, int baseOffset, void *p, int dataLen);
private:
  static unsigned char checksum(void *pBuf, int sz);
};



template<typename T>
class eepromIf
{
public:
  static bool Save(T *p, unsigned char logLevel);

  static bool Load(T *p, unsigned char logLevel);

  static int Signature;
  static int baseOffset;
};

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
