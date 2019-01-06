/*
Library: serialize structure/class to/from EEPROM.
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

Sources repository: https://github.com/microentropie/serializeEeprom
How to install:
  https://www.arduino.cc/en/guide/libraries
*/

#include "serializeEeprom.h"

unsigned char serialize2eeprom::checksum(void *pBuf, uint16_t sz)
{
  signed char chksum = 45;
  for (int i = 0; i < sz; ++i)
    chksum += ((signed char *)pBuf)[i];
  return (unsigned char)chksum;
}
