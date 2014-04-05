////////////////////////////////////////////////////////////////////////////////
//
//  SdSvr.h - Definition of SD card utility functions
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SDSVR_H
#define SDSVR_H

#include <Arduino.h>
#include <SD.h>

////////////////////////////////////////////////////////////////////////////////

class SdSvr
{
public:
  SdSvr();
  virtual ~SdSvr();
  
  virtual void begin(int the_SS, int the_CS);
  virtual void terminate();
  
public:
  // Management of HTML pages
  bool      resFileExists       (const char * the_url) const;
  uint32_t  resFileSize         () const;
  bool      openResFile         (const char * the_url);
  void      closeCurrentResFile ();
  bool      isResFileOpen       () const;
  uint16_t  readResFileBuffer   (uint8_t * the_buffer, uint16_t the_size);

private:
  enum sdStatus_e
  {
    sd_notset,
    sd_notAvailable,
    sd_initialized,
    sd_resFileOpen
  };

private:
  sdStatus_e my_sdStatus;
  File       my_resFile;
};

////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef SDSVR_H

