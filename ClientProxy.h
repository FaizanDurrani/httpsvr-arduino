////////////////////////////////////////////////////////////////////////////////
//
//  ClientProxy.h - Definition of HTTP Client Proxy
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include <Arduino.h>
#include <IPAddress.h>

#include "utility/W5100.h"
#include "utility/vinit.h"

////////////////////////////////////////////////////////////////////////////////

class ClientProxy
{
public:
  ClientProxy();
  virtual ~ClientProxy();

public:
  // Connection management functions
  void                  setConnection     (W5100::socket_e the_sn);
  bool                  closeConnection   ();
  bool                  isConnected       () const;
  void                  triggerConnTimeout();
  bool                  connTimeoutExpired() const;
  
  // Connection info functions
  W5100::socket_e       socket            () const;
  uint16_t              localPort         () const;
  uint16_t              remotePort        () const;
  IPAddress             remoteIpAddr      () const;
  W5100::mac_address_t  remoteMacAddr     () const;

  // Low level read functions
  bool                  readByte          (uint8_t&);
  uint16_t              readBuffer        (uint8_t* the_buffer, uint16_t the_size);
  bool                  unreadByte        (uint8_t);
  bool                  peekByte          (uint8_t&);
  bool                  anyDataReceived   () const;
  uint32_t              totRead           () const { return my_totRead; }

  // High level read functions
  bool                  skipAllCRLF       ();
  bool                  skipAllLWS        ();
  bool                  skipToNextLine    ();
  bool                  readCRLF          ();
  uint16_t              readToken         (char * the_buffer, uint16_t the_bufferLen);
  uint16_t              readToEOL         (char * the_buffer, uint16_t the_bufferLen);
  
  // Low level write functions
  bool                  writeByte         (uint8_t the_byte);
  uint16_t              writeBuffer       (uint8_t * the_buffer, uint16_t the_size);
  void                  flush             ();
  uint32_t              totWrite          () const { return my_totWrite; }

private:
  bool                  prv_isValidSn     () const;
  
private:
  W5100::socket_e       my_sn;
  ext::vinit<uint8_t>   my_unreadByte;
  ext::vinit<bool>      my_unreadByteAvail;
  ext::vinit<uint32_t>  my_totRead;
  ext::vinit<uint32_t>  my_totWrite;
  ext::vinit<uint32_t>  my_connIdleStart;
};

////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef CLIENTPROXY_H

