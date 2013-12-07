////////////////////////////////////////////////////////////////////////////////
//
//  SdSvr.cpp - Definition of SD card utility functions
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#include "SdSvr.h"

#ifndef LOCAL_MAX_URL_LENGTH
#  define LOCAL_MAX_URL_LENGTH  128
#endif

////////////////////////////////////////////////////////////////////////////////

static unsigned int local_strlen(const char * s)
{
  if (!s) return 0;
  for (unsigned int u = 0; u < LOCAL_MAX_URL_LENGTH; ++u)
    if (*s++ == 0) return u;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

// Construction without initialization of SD card
SdSvr::SdSvr()
: my_sdStatus(sd_notAvailable)
{}

SdSvr::~SdSvr()
{ my_sdStatus = sd_notAvailable; }

void SdSvr::begin(int the_SS, int the_CS)
{
  my_sdStatus = sd_notAvailable;
  
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(the_SS, OUTPUT);
   
  if (!SD.begin(the_CS)) return;
  my_sdStatus = sd_initialized;
}

void SdSvr::terminate()
{ my_sdStatus = sd_notAvailable; }

////////////////////////////////////////////////////////////////////////////////

bool SdSvr::resFileExists(const char * the_url) const
{
  if (my_sdStatus != sd_initialized) return false;
  return SD.exists(const_cast<char *>(the_url));
}

uint32_t SdSvr::resFileSize() const
{
  if (!isResFileOpen()) return 0;
  return const_cast<File&>(my_resFile).size(); // size() should be const
}

bool SdSvr::openResFile(const char *the_url)
{
  if (my_sdStatus != sd_initialized) return false;
  
  if (my_resFile) my_resFile.close();
  my_resFile = SD.open(const_cast<char *>(the_url), FILE_READ);
  my_sdStatus = sd_resFileOpen;
}

void SdSvr::closeCurrentResFile()
{
  if (my_sdStatus != sd_resFileOpen) return;
  
  if (my_resFile) my_resFile.close();
  my_sdStatus = sd_initialized;
}

bool SdSvr::isResFileOpen() const
{ return my_sdStatus == sd_resFileOpen; }

uint16_t SdSvr::readResFileBuffer(uint8_t * the_buffer, uint16_t the_size)
{
  if (!the_buffer) return 0;
  if (the_size < 1) return 0;
  if (!isResFileOpen()) return 0;

  uint16_t uRead = my_resFile.read(the_buffer, the_size);
  memset(the_buffer + uRead, the_size - uRead, 0);
  return uRead;
}

////////////////////////////////////////////////////////////////////////////////

