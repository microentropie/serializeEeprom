/*
Library: serialize structure/class to/from EEPROM.
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

Sources repository: https://github.com/microentropie/serializeEeprom
How to install:
  https://www.arduino.cc/en/guide/libraries
*/

#include <EEPROM.h>
#include <arduino.h>


#include "serializeEeprom.h"

/*
The various Arduino and Genuino boards have different EEPROM sizes:
512 bytes on the ATmega168 and ATmega8
1024 bytes on the ATmega328,
The Arduino and Genuino 101 boards have an emulated EEPROM space of 1024 bytes.
4096 bytes on the ATmega1280 and ATmega2560
4096 bytes on ESP8266
*/

#if defined ESP8266
#define SPI_FLASH_SEC_SIZE 4096 // ESP8266
#else
#error "Unknown EEPROM size for this micro"
#endif

#define EEPROM_MAX_USED_SIZE 1024

#if EEPROM_MAX_USED_SIZE > SPI_FLASH_SEC_SIZE
#error "EEPROM size exceeded"
#endif

bool serialize2eeprom::Save(unsigned char logLevel, int dataSignature, int baseOffset, void *p, int dataLen)
{
  int id;
  int version;
  uint8_t chksum;

  // (1) write 'integrity' info:
  id = dataSignature; // signature for this settings
  version = dataLen;
  EEPROM.begin(EEPROM_MAX_USED_SIZE);
  int pos = baseOffset;
  EEPROM.put(pos, id); pos += sizeof(id);
  EEPROM.put(pos, version); pos += sizeof(version);
  chksum = checksum(p, dataLen);
  //Serial.print("eeprom save, Checksum=");
  //Serial.println(chksum);
  EEPROM.put(pos, chksum); pos += sizeof(chksum);

  // (2) write the payload:
  uint8_t *pData = EEPROM.getDataPtr(); // EEPROM class holds a buffer
  memcpy(pData + pos, p, dataLen); // trick: write directly to buffer, internal 'dirty' flag is already set
  pos += dataLen;
  //for(int i = 0; i < dataLen; ++i)
  //  EEPROM.put(pos++, ((uint8_t *)p)[i]);

  // check if out of EEPROM space
  if (pos > EEPROM_MAX_USED_SIZE)
    Serial.println(F("ERROR: Save() out of EEPROM space, increase EEPROM_MAX_USED_SIZE (max SPI_FLASH_SEC_SIZE) !"));

  if(logLevel > 1)
  {
    Serial.print(F("EEPROM Save(): struct size = "));
    Serial.print(pos - baseOffset);
    Serial.print(F(" bytes. Used "));
    Serial.print(pos);
    Serial.print(F(" / "));
    Serial.print(EEPROM_MAX_USED_SIZE);
    Serial.println(F(" bytes."));
  }
  Serial.print(F("EEPROM Save(): written cfg '"));
  Serial.write(dataSignature >> 24); Serial.write((dataSignature >> 16) & 255); Serial.write((dataSignature >> 8) & 255); Serial.write(dataSignature & 255);
  Serial.print(F("', offsets: [")); Serial.print(baseOffset); Serial.print(", "); Serial.print(pos); Serial.println("[");

  //EEPROM.commit(); // phisically write to EEPROM
  EEPROM.end();    // phisically write to EEPROM + release EEPROM buffer
  yield();

  return (pos <= EEPROM_MAX_USED_SIZE);
}

bool serialize2eeprom::Load(unsigned char logLevel, int dataSignature, int baseOffset, void *p, int dataLen)
{
  int id;
  int version;
  uint8_t chksum;

  int pos = baseOffset;
  EEPROM.begin(EEPROM_MAX_USED_SIZE);

  // (1) read 'integrity' info:
  EEPROM.get(pos, id); pos += sizeof(id);
  EEPROM.get(pos, version); pos += sizeof(version);
  if (id != dataSignature || version != dataLen)
  {
    EEPROM.end(); // dealloc
    if(logLevel > 0)
    {
      Serial.print(F("EEPROM Load(): invalid cfg '"));
      Serial.write(dataSignature >> 24); Serial.write((dataSignature >> 16) & 255); Serial.write((dataSignature >> 8) & 255); Serial.write(dataSignature & 255);
      Serial.print(F("', offsets: [")); Serial.print(baseOffset); Serial.print(", "); Serial.print(pos); Serial.println("[");
    }
    return false;
  }
  EEPROM.get(pos, chksum); pos += sizeof(chksum);
  uint8_t *pData = EEPROM.getDataPtr(); // EEPROM class holds a buffer
  int ok = checksum(pData + pos, dataLen) == chksum; // trick: peek directly from internal buffer

  // (2) read the payload:
  if (ok)
    memcpy(p, pData + pos, dataLen); // trick: read directly from internal buffer
  pos += dataLen;

  if(logLevel > 0)
  {
    if (ok)
    {
      Serial.print(F("EEPROM Load(): recovered cfg '"));
      Serial.write(dataSignature >> 24); Serial.write((dataSignature >> 16) & 255); Serial.write((dataSignature >> 8) & 255); Serial.write(dataSignature & 255);
      Serial.print(F("', offsets: [")); Serial.print(baseOffset); Serial.print(", "); Serial.print(pos); Serial.println("[");
    }
    else
      Serial.println(F("EEPROM Load(): invalid checksum"));
  }
  EEPROM.end(); // dealloc

  return ok;
}

unsigned char serialize2eeprom::checksum(void *pBuf, int sz)
{
  signed char chksum = 45;
  for (int i = 0; i < sz; ++i)
    chksum += ((signed char *)pBuf)[i];
  return (unsigned char)chksum;
}

