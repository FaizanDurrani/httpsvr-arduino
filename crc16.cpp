/*---------------------------------------------------------------------------*/

#include <Arduino.h>
#include "crc16.h"

/*---------------------------------------------------------------------------*/

uint16_t crc16_add(unsigned char b, uint16_t acc)
{
  acc ^= b;
  acc = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}

/*---------------------------------------------------------------------------*/

uint16_t crcsum(const char* message, uint32_t length, uint16_t crc = CRC_INIT)
{
  uint32_t i;

  for(i = 0; i < length; ++i) 
  {
    crc = crc16_add(*message, crc);
    ++message;
  }
  return crc;
}

/*---------------------------------------------------------------------------*/

