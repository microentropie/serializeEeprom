#ifndef _CHAR_INT_CHAR_H_
#define _CHAR_INT_CHAR_H_

#include <WString.h>

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif //uint32_t

uint32_t string2Int(const char *str);
template <typename T> T string2T(const char *str);

#if defined(ARDUINO_ARCH_ESP8266)
uint32_t string2Int(const __FlashStringHelper *str);
template <typename T> T string2T(const __FlashStringHelper *str);
#endif //defined(ARDUINO_ARCH_ESP8266)

#define Int2char(intValue, index) (intValue && (index) >= 0 && (index) < sizeof(intValue))? ((intValue) >> ((sizeof(intValue) - (index) - 1) << 3) & 0xFF) : 0
// usage example:
/*
void main()
{
  int name = 0x41424344;
  printf("The string is '%c%c%c%c'.", Int2char(name, 0), Int2char(name, 1), Int2char(name, 2), Int2char(name, 3));
  // Output:
  // The string is 'ABCD'.
}
*/

#endif //_CHAR_INT_CHAR_H_