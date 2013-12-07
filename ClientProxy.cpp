////////////////////////////////////////////////////////////////////////////////
//
//  ClientProxy.cpp - Definition of HTTP Client Proxy
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "ClientProxy.h"
#include "utility/W5100.h"

///////////////////////////////////////////////////////////////////////////////

ClientProxy::ClientProxy()
: my_sn(W5100::socket_undefined)
{}

ClientProxy::~ClientProxy()
{}

///////////////////////////////////////////////////////////////////////////////

void ClientProxy::setConnection(W5100::socket_e the_sn)
{ my_sn = the_sn; }

bool ClientProxy::closeConnection()
{ 
  if (!prv_isValidSn()) return true;
  
  while (!W5100::isClosed(my_sn))
    if (W5100::close(my_sn) != W5100::rc_ok)
      return false;
      
  my_sn = W5100::socket_undefined;

  return true;
}

bool ClientProxy::isConnected() const
{
  if (!prv_isValidSn()) return false;
  return W5100::isConnected(my_sn);
}

void ClientProxy::triggerConnTimeout()
{ my_connIdleStart = millis(); }

bool ClientProxy::connTimeoutExpired() const
{ return millis() - my_connIdleStart > 5000; }

///////////////////////////////////////////////////////////////////////////////

W5100::socket_e ClientProxy::socket() const
{ return my_sn; }

uint16_t ClientProxy::localPort() const
{ return (prv_isValidSn() ? W5100::read_Sn_R16(my_sn, W5100_Sn_PORT) : 0); }

uint16_t ClientProxy::remotePort() const
{ return (prv_isValidSn() ? W5100::read_Sn_R16(my_sn, W5100_Sn_DPORT) : 0); }

IPAddress ClientProxy::remoteIpAddr() const
{
  W5100::ipv4_address_t ipAddr = prv_isValidSn() ? W5100::ipv4_address_t(my_sn) : W5100::ipv4_address_t(0,0,0,0);
  return IPAddress(ipAddr.ip0(), ipAddr.ip1(), ipAddr.ip2(), ipAddr.ip3());
}

W5100::mac_address_t ClientProxy::remoteMacAddr() const
{ return (prv_isValidSn() ? W5100::mac_address_t(my_sn) : W5100::mac_address_t(0,0,0,0,0,0)); }

///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::readByte(uint8_t& the_byte)
{
  if (!prv_isValidSn()) return false;

  if (my_unreadByteAvail) 
  { 
    my_unreadByteAvail = false;
    the_byte = my_unreadByte;
    ++my_totRead;
    return true;; 
  }

  if (W5100::waitReceivePending(my_sn) != W5100::rc_ok)
  {
    closeConnection();
    return false;
  }

  uint16_t uRead = W5100::receive(my_sn, &the_byte, 1);
  my_totRead += uRead;
  return (uRead == 1);
}

uint16_t ClientProxy::readBuffer(uint8_t* the_buffer, uint16_t the_size)
{
  if (!prv_isValidSn()) return 0;
  if (!the_buffer || !the_size) return 0;
  
  uint16_t uRead = 0;
  if (my_unreadByteAvail)
  {
    my_unreadByteAvail = false;
    *the_buffer = my_unreadByte;
    the_buffer++;
    uRead++;
    the_size--;
  }
  
  if (W5100::waitReceivePending(my_sn) != W5100::rc_ok)
  { my_totRead += uRead; closeConnection(); return uRead; }
  
  uRead += W5100::receive(my_sn, the_buffer, the_size);
  my_totRead += uRead;
  return uRead;
}

bool ClientProxy::unreadByte(uint8_t the_byte)
{
  if (!prv_isValidSn()) return false;
  if (my_unreadByteAvail) return false;
  my_unreadByte = the_byte;
  my_unreadByteAvail = true;
  --my_totRead;
  return true;
}

bool ClientProxy::peekByte(uint8_t& the_byte)
{ return (readByte(the_byte) && unreadByte(the_byte)); }

bool ClientProxy::anyDataReceived() const
{
  if (!prv_isValidSn()) return false;
  return (W5100::checkReceivePending(my_sn) == W5100::rc_ok);
}

///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::skipAllCRLF()
{
  uint8_t ch;
  while (readByte(ch))
    if ((ch != '\r') && (ch != '\n'))
      return unreadByte(ch);
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::skipAllLWS()
{
  uint8_t ch;
  while (readByte(ch))
    if ((ch != ' ') && (ch != '\t'))
      return unreadByte(ch); 
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::skipToNextLine()
{
  // Ignore everything up to CRLF, but do not consume CRLF.
  // CRLF is considered as the beginning of next line. Although weird, this approach
  // helps a lot in recognizing field boundaries in form/multipart content types
  uint8_t ch;
  while (readByte(ch))
    if ((ch == '\r') || (ch == '\n')) 
      return unreadByte(ch); 
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::readCRLF()
{
  uint8_t ch;
  if (!readByte(ch)) return false;
  if (ch != '\r') { unreadByte(ch); return false; }
  if (!readByte(ch)) return false;
  if (ch != '\n') { unreadByte(ch); return false; }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t ClientProxy::readToken(char * the_buffer, uint16_t the_bufferLen)
{
  if (!the_buffer) return 0;
  if (!the_bufferLen) return 0;
  
  uint16_t u = 0;
  for (; u < the_bufferLen-1; ++u)
  {
    uint8_t ch;
    if (!readByte(ch)) break;
    if ((ch == ' ') || (ch == '\r') || (ch == '\n')) { unreadByte(ch); break; }
    the_buffer[u] = ch;
  }  
  the_buffer[u] = 0;
  return u;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t ClientProxy::readToEOL(char * the_buffer, uint16_t the_bufferLen)
{
  // This function reads until one of the following conditions is met:
  // - there are no more chars available
  // - the buffer length is exceeded
  // - a CR char is met (but not consumed) not at the beginning of line
  if (!the_buffer) return 0;
  if (!the_bufferLen) return 0;

  uint16_t u = 0;
  for (; u < the_bufferLen-1; ++u)
  {
    uint8_t ch;
    if (!readByte(ch)) break;
    if ((ch == '\r') && (u > 0)) { unreadByte(ch); break; }
    the_buffer[u] = ch;
  }  
  the_buffer[u] = 0;
  return u;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::writeByte(uint8_t the_byte)
{
  if (!prv_isValidSn()) return false;
  if (!W5100::canTransmitData(my_sn)) { closeConnection(); return false; }
  if (W5100::send(my_sn, &the_byte, 1) != 1) return false;
  if (W5100::waitSendCompleted(my_sn) != W5100::rc_ok) { closeConnection(); return false; }
  ++my_totWrite;
  return true;
}

uint16_t ClientProxy::writeBuffer(uint8_t * the_buffer, uint16_t the_size)
{
  if (!prv_isValidSn()) return 0;
  if (!W5100::canTransmitData(my_sn)) { closeConnection(); return 0; }
  the_size = W5100::send(my_sn, the_buffer, the_size);
  if (W5100::waitSendCompleted(my_sn) != W5100::rc_ok) { closeConnection(); return 0; }
  my_totWrite += the_size;
  return the_size;
}

void ClientProxy::flush()
{
  if (prv_isValidSn()) return;
  if (W5100::waitSendCompleted(my_sn) != W5100::rc_ok) closeConnection();
}
  
///////////////////////////////////////////////////////////////////////////////

bool ClientProxy::prv_isValidSn() const
{ return my_sn != W5100::socket_undefined; }

///////////////////////////////////////////////////////////////////////////////

