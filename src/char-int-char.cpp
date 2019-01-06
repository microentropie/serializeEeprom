#include "char-int-char.h"

// The string is written into an int (4 bytes)
// * extra chars are ignored
// * missing chars cause 0 filling
// Examples:
//   "ABCD"  => 0x41424344 (0x41 is 'A', 0x42 is 'B', ...)
//   "ABCDE" => 0x41424344 (extra 'E' is ignored)
//   "A"     => 0x41000000 (0 filled)
// NOTE: CPU byte order (big-endian, little-endian, ...) are irrelevant
//       as far as the following functions are used
uint32_t string2Int(const char *str)
{
  return string2T<uint32_t>(str);
}
template <typename T> T string2T(const char *str)
{
  if(!str) return 0; // null pointer => do nothing
  T shiftRegister = 0;
  int i;
  for(i = 0; str[i] && i < sizeof(T); ++i)
    shiftRegister = (shiftRegister << 8) | (unsigned char)str[i];
  for(; i < sizeof(T); ++i) // 0 fill
    shiftRegister <<= 8;
  return shiftRegister;
}

#if defined(ARDUINO_ARCH_ESP8266)
uint32_t string2Int(const __FlashStringHelper *str)
{
  return string2T<uint32_t>(str);
}
template <typename T> T string2T(const __FlashStringHelper *str)
{
  if(!str) return 0; // null pointer => do nothing

  PGM_P p = reinterpret_cast<PGM_P>(str);
  T shiftRegister = 0;
  int i;
  for(i = 0; i < sizeof(T); ++i)
  {
    uint8_t byteval = pgm_read_byte(p + i);
    if(byteval == 0)
      break;
    shiftRegister = (shiftRegister << 8) | byteval;
  }
  for(; i < sizeof(T); ++i) // 0 fill
    shiftRegister <<= 8;
  return shiftRegister;
}
#endif //defined(ARDUINO_ARCH_ESP8266)
