/*
Library: serialize structure/class to/from EEPROM.
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

Sources repository: https://github.com/microentropie/serializeEeprom
How to install:
  https://www.arduino.cc/en/guide/libraries
*/
#if defined(ARDUINO_ARCH_ESP32) 

//#include "esp_partition.h"
//#include "esp_err.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <HardwareSerial.h>
extern "C" { void yield(void); }

#include "serializeEeprom.h"

// ESP32 eeprom size can be configured at any size,
// by creating a custom partition table.
// Arduino decided to only make it 64 bytes.
// A better solution is to totally change the approach
// and use the nvs partition
// (nvs_set_blob() and nvs_get_blob()).


#define KeyBuf_LEN 15
#define ValueBuf_LEN 1984

#define PrepareKey(KeyBuf,id,offs) snprintf(KeyBuf, sizeof(KeyBuf), "eep:%c%c%c%c-%04X", ((id)>>24)&0xFF, ((id)>>16)&0xFF, ((id)>>8)&0xFF, (id)&0xFF, offs)
#define NVS_NAMESPACE "EEP"

static const char *MsgErrorSave = "ERROR(%d) NVS Save() - %s: %s\n";
static const char *MsgErrorLoad = "ERROR(%d) NVS Load() - %s: %s\n";

static const __FlashStringHelper *NvsErrorMessage(esp_err_t err)
{
  switch (err)
  {
  case ESP_ERR_NVS_NOT_INITIALIZED:  /*!< The storage driver is not initialized */
    return F("ESP_ERR_NVS_NOT_INITIALIZED");
  case ESP_ERR_NVS_NOT_FOUND:        /*!< Id namespace doesn’t exist yet and mode is NVS_READONLY */
    return F("ESP_ERR_NVS_NOT_FOUND");
  case ESP_ERR_NVS_TYPE_MISMATCH:    /*!< The type of set or get operation doesn't match the type of value stored in NVS */
    return F("ESP_ERR_NVS_TYPE_MISMATCH");
  case ESP_ERR_NVS_READ_ONLY:        /*!< Storage handle was opened as read only */
    return F("ESP_ERR_NVS_READ_ONLY");
  case ESP_ERR_NVS_NOT_ENOUGH_SPACE: /*!< There is not enough space in the underlying storage to save the value */
    return F("ESP_ERR_NVS_NOT_ENOUGH_SPACE");
  case ESP_ERR_NVS_INVALID_NAME:     /*!< Namespace name doesn’t satisfy constraints */
    return F("ESP_ERR_NVS_INVALID_NAME");
  case ESP_ERR_NVS_INVALID_HANDLE:   /*!< Handle has been closed or is NULL */
    return F("ESP_ERR_NVS_INVALID_HANDLE");
  case ESP_ERR_NVS_REMOVE_FAILED:    /*!< The value wasn’t updated because flash write operation has failed. The value was written however, and update will be finished after re-initialization of nvs, provided that flash operation doesn’t fail again. */
    return F("ESP_ERR_NVS_REMOVE_FAILED");
  case ESP_ERR_NVS_KEY_TOO_LONG:     /*!< Key name is too long */
    return F("ESP_ERR_NVS_KEY_TOO_LONG");
  case ESP_ERR_NVS_PAGE_FULL:        /*!< Internal error; never returned by nvs_ API functions */
    return F("ESP_ERR_NVS_PAGE_FULL");
  case ESP_ERR_NVS_INVALID_STATE:    /*!< NVS is in an inconsistent state due to a previous error. Call nvs_flash_init and nvs_open again, then retry. */
    return F("ESP_ERR_NVS_INVALID_STATE");
  case ESP_ERR_NVS_INVALID_LENGTH:   /*!< String or blob length is not sufficient to store data */
    return F("ESP_ERR_NVS_INVALID_LENGTH");
  case ESP_ERR_NVS_NO_FREE_PAGES:    /*!< NVS partition doesn't contain any empty pages. This may happen if NVS partition was truncated. Erase the whole partition and call nvs_flash_init again. */
    return F("ESP_ERR_NVS_NO_FREE_PAGES");
  case ESP_ERR_NVS_VALUE_TOO_LONG:   /*!< String or blob length is longer than supported by the implementation */
    return F("ESP_ERR_NVS_VALUE_TOO_LONG");
  case ESP_ERR_NVS_PART_NOT_FOUND:   /*!< Partition with specified name is not found in the partition table */
    return F("ESP_ERR_NVS_PART_NOT_FOUND");
  }
  return F("");
}

bool serialize2eeprom::Init()
{
#if !_NVS_INITIALIZATION_REQUIRED_
  return true;
#else //_NVS_INITIALIZATION_REQUIRED_
  esp_err_t ret = nvs_flash_init();
  ESP_ERROR_CHECK(ret);
  if (ret == ESP_ERR_NVS_NOT_FOUND)
    Serial.println(F("ERROR: NVS Init() 'Id namespace doesn’t exist yet and mode is NVS_READONLY' !"));

  if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
  {
    // partition not formatted
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  /*
  if (err != ESP_OK)
  {
    const esp_partition_t* nvs_partition =
    esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    if(!nvs_partition) printf("FATAL ERROR: No NVS partition found\n");
      return ret;
   err = esp_partiton_erase_range(nvs_partition, 0, nvs_partition->size);
  }
  */

  return ret == ESP_OK;
#endif //_NVS_INITIALIZATION_REQUIRED_
}

bool serialize2eeprom::Save(unsigned char logLevel, uint32_t _dataSignature, uint16_t _baseOffset, void *p, uint16_t _dataLen)
{
  uint8_t chksum;
  uint16_t offs, len;
  uint32_t id;
  char KeyBuf[KeyBuf_LEN];
  char ValueBuf[ValueBuf_LEN];


  if (sizeof(_dataLen) + sizeof(chksum) + _dataLen > sizeof(ValueBuf))
  {
    Serial.println(F("ERROR: NVS Save() exceeded max nvs blob size !"));
    return false;
  }
  id = (uint32_t)_dataSignature;
  offs = (uint16_t)_baseOffset;
  len = (uint16_t)_dataLen;

  // (1) prepare key (name and offset):
  PrepareKey(KeyBuf, id, offs);

  // (2) prepare value:
  int pos = 0;
  *(uint16_t *)&ValueBuf[pos] = len; pos += sizeof(len);
  chksum = checksum(p, len);
  *(uint8_t *)&ValueBuf[pos] = chksum; pos += sizeof(chksum);
  memcpy(&ValueBuf[pos], p, len);
  pos += len;

  // (3) write to nvs
  esp_err_t err;
  nvs_handle nvsHandle;

  err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
  if (err != ESP_OK)
  {
    printf(MsgErrorSave, err, "nvs_open", NvsErrorMessage(err));
    return false;
  }
  err = nvs_set_blob(nvsHandle, KeyBuf, ValueBuf, pos);
  // if space is full: clear all
  if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE)
  {
    err = nvs_erase_all(nvsHandle);
    if (err == ESP_OK)
      err = nvs_set_blob(nvsHandle, KeyBuf, ValueBuf, pos);
  }
  if (err == ESP_OK)
    err = nvs_commit(nvsHandle);
  nvs_close(nvsHandle);
  yield();
  if (err != ESP_OK)
  {
    printf(MsgErrorSave, err, "nvs_set_blob", NvsErrorMessage(err));
    return false;
  }

  Serial.print(F("NVS Save(): written '"));
  Serial.print(KeyBuf);
  Serial.print(F("', size: ")); Serial.print(pos); Serial.println(" bytes.");

  return true;
}

bool serialize2eeprom::Load(unsigned char logLevel, uint32_t _dataSignature, uint16_t _baseOffset, void *p, uint16_t _dataLen)
{
  uint8_t chksum;
  uint16_t offs, len;
  uint32_t id;
  char KeyBuf[KeyBuf_LEN];
  char ValueBuf[ValueBuf_LEN];


  id = (uint32_t)_dataSignature;
  offs = (uint16_t)_baseOffset;
  len = (uint16_t)_dataLen;
  // (1) prepare key (name and offset):
  PrepareKey(KeyBuf, id, offs);

  // (2) read from nvs EEPROM
  esp_err_t err;
  nvs_handle nvsHandle;

  err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvsHandle);
  if (err != ESP_OK)
  {
    printf(MsgErrorLoad, err, "nvs_open", NvsErrorMessage(err));
    return false;
  }
  size_t l = sizeof(ValueBuf);
  err = nvs_get_blob(nvsHandle, KeyBuf, ValueBuf, &l);
  nvs_close(nvsHandle);
  if (err != ESP_OK)
  {
    printf(MsgErrorLoad, err, "nvs_get_blob", NvsErrorMessage(err));
    return false;
  }

  // (2) check 'integrity' info:
  if (sizeof(len) + sizeof(chksum) + len != l)
  {
    Serial.println(F("ERROR: NVS Load(), invalid data !"));
    return false;
  }

  int pos = 0;
  len = *(uint16_t *)&ValueBuf[pos]; pos += sizeof(len);
  chksum = *(uint8_t *)&ValueBuf[pos]; pos += sizeof(chksum);
  if (len != _dataLen)
  {
    Serial.println(F("ERROR: NVS Load(), invalid data !"));
    return false;
  }
  void *pReadIn = &ValueBuf[pos];
  if (chksum != checksum(pReadIn, _dataLen))
  {
    Serial.println(F("ERROR: NVS Load(), invalid data !"));
    return false;
  }

  // ok, data is valid, transfer to destination buffer
  memcpy(p, pReadIn, _dataLen);
  pos += _dataLen;

  bool ok = true;
  if (logLevel > 0)
  {
    if (ok)
    {
      Serial.print(F("NVS Load(): recovered '"));
      Serial.print(KeyBuf);
      Serial.print(F("', size: ")); Serial.print(pos); Serial.println(" bytes.");
    }
    else
      Serial.println(F("NVS Load(): invalid checksum"));
  }

  return ok;
}
#endif //ARDUINO_ARCH_ESP32
