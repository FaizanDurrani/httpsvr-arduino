#ifndef CRC16_H
#define CRC16_H

#include <Arduino.h>

#define CRC_INIT 0xFFFF

#ifdef __cplusplus
extern "C" {
#endif

/*
* Derived from CRC algorithm for JTAG ICE mkII, published in Atmel
* Appnote AVR067. Converted from C++ to C.
*/

extern uint16_t crcsum(const char* message, uint32_t length, uint16_t crc);

#ifdef __cplusplus
}
#endif

#endif
