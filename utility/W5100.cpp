////////////////////////////////////////////////////////////////////////////////
//
//  W5100.cpp - Definition of W5100 driver
//  ECA 26Oct2013
//
//  ----------------------
//
//  IMPORTANT: Refer to WIZnet W5100 datasheet for a complete description
//  of the chip and its operation.
//
//  From the Arduino Ethernet Shield Documentation:
//
//    Arduino communicates with both the W5100 and SD card using the SPI bus (through the ICSP header).
//    This is on digital pins 11, 12, and 13 on the Duemilanove and pins 50, 51, and 52 on the Mega.
//    On both boards, pin 10 is used to select the W5100 and pin 4 for the SD card. These pins cannot be used
//    for general i/o. On the Mega, the hardware SS pin, 53, is not used to select either the W5100 or the SD card,
//    but it must be kept as an output or the SPI interface won't work.
//
//    Note that because the W5100 and SD card share the SPI bus, only one can be active at a time.
//    If you are using both peripherals in your program, this should be taken care of by the corresponding libraries.
//    If you're not using one of the peripherals in your program, however, you'll need to explicitly deselect it.
//    To do this with the SD card, set pin 4 as an output and write a high to it. For the W5100, set digital pin 10
//    as a high output. 
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "W5100.h"
#include "SPI.h"

static const uint8_t  uMaxTry = 10;
static const uint16_t oneKB   = 0x0400; // 1024 bytes

////////////////////////////////////////////////////////////////////////////////
// Initialization and termination

void W5100::begin(const mac_address_t& the_macAddr, const ipv4_address_t& the_ipAddr)
{
  // Init SPI for communication
  SPI.begin();
  prv_initSS();
 
  // Reset chip
  write_R8(W5100_MR, W5100_RST);
  while (read_R8(W5100_MR));
  
  // Set TX and RX buffer size for each socket
  write_R8(W5100_RMSR, W5100_S0_2K | W5100_S1_2K | W5100_S2_2K | W5100_S3_2K);
  write_R8(W5100_TMSR, W5100_S0_2K | W5100_S1_2K | W5100_S2_2K | W5100_S3_2K);
  
  // Set MAC and IP address to SHAR and SIPR respectively
  the_macAddr.set();
  the_ipAddr.set();
}

void W5100::terminate()
{
  // Close all connections
  close(socket_0);
  close(socket_1);
  close(socket_2);
  close(socket_3);
  
  // Reset chip
  write_R8(W5100_MR, W5100_RST);
  while (read_R8(W5100_MR));
}

///////////////////////////////////////////////////////////////////////////////
// Socket command functions

W5100::retcode_e W5100::open(socket_e the_socket, uint16_t the_port)
{
  // Check preconditions: socket status must be CLOSED or INIT
  uint8_t sockStatus = status(the_socket);
  if ((sockStatus != W5100_SOCK_INIT) && (sockStatus != W5100_SOCK_CLOSED))
    return rc_invalid_status;
  
  // Clear any previous event flag
  write_Sn_R8(the_socket, W5100_Sn_IR, 0xFF);
  
  // Set socket mode (TCP) and port
  write_Sn_R8 (the_socket, W5100_Sn_MR, W5100_PROTOCOL_TCP);
  write_Sn_R16(the_socket, W5100_Sn_PORT, the_port);
  
  // Issue the OPEN command and wait for completion
  for (unsigned int uTry = 0; uTry < uMaxTry; ++uTry)
  {
    write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_OPEN);
    if (read_Sn_R8(the_socket, W5100_Sn_SR) == W5100_SOCK_INIT) return rc_ok;
  }
  return rc_open_failed;
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::listen(socket_e the_socket)
{
  // Check preconditions: socket status must be INIT
  if (status(the_socket) != W5100_SOCK_INIT)
    return rc_invalid_status;
  
  // Issue the LISTEN command and wait for completion
  for (unsigned int uTry = 0; uTry < uMaxTry; ++uTry)
  {
    write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_LISTEN);
    if (read_Sn_R8(the_socket, W5100_Sn_SR) == W5100_SOCK_LISTEN) return rc_ok;
  }
  return rc_listen_failed;
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::connect(socket_e the_socket, const ipv4_address_t& the_ipAddr, uint16_t the_port)
{
  // Check preconditions: socket status must be INIT
  if (status(the_socket) != W5100_SOCK_INIT)
    return rc_invalid_status;

  // Write IP address and port number to Sn_DIPR and Sn_DPORT respectively
  switch (the_socket)
  {
  case socket_0: the_ipAddr.set(W5100_S0_DIPR0); break;
  case socket_1: the_ipAddr.set(W5100_S1_DIPR0); break;
  case socket_2: the_ipAddr.set(W5100_S2_DIPR0); break;
  case socket_3: the_ipAddr.set(W5100_S3_DIPR0); break;
  default: return rc_invalid_socket;
  }

  write_Sn_R16(the_socket, W5100_Sn_DPORT, the_port);

  // Clear previous CON and TIMEOUT event flag
  set_flags(the_socket, W5100_IR_SEND_OK | W5100_IR_TIMEOUT);
  
  // Issue the CONNECT command and wait for completion or timeout
  write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_CONNECT);
  for (;;)
  {
    volatile uint8_t snFlags = flags(the_socket);
    if (snFlags & W5100_IR_CON)
    {
      // Reset signal bit
      set_flags(the_socket, W5100_IR_CON);
      // Wait for completion of connection
      for (unsigned int uTry = 0; uTry < uMaxTry; ++uTry)
        if (status(the_socket) == W5100_SOCK_ESTABLISHED) return rc_ok;
      return rc_connect_failed;
    }
    else if (snFlags & W5100_IR_TIMEOUT)
    {
      // Reset signal bit
      set_flags(the_socket, W5100_IR_TIMEOUT);
      return rc_connect_timeout;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::disconnect(socket_e the_socket)
{
  // Check preconditions: if socket status is not ESTABLISHED, do nothing
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return rc_ok;
  
  // Issue the DISCONNECT command and wait for completion or timeout
  write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_DISCON);
  for (;;)
  {
    volatile uint8_t snFlags = flags(the_socket);
    if (snFlags & W5100_IR_DISCON)
    {
      // Reset signal bit
      set_flags(the_socket, W5100_IR_DISCON);
      // Wait for completion of disconnection
      for (unsigned int uTry = 0; uTry < uMaxTry; ++uTry)
        if (status(the_socket) == W5100_SOCK_CLOSED) return rc_ok;
      return rc_disconnect_failed;
    }
    else if (snFlags & W5100_IR_TIMEOUT)
    {
      // Reset signal bit
      set_flags(the_socket, W5100_IR_TIMEOUT);
      return rc_disconnect_timeout;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::checkClientConn(socket_e the_socket)
{
  // Check for client connection (non blocking)
  switch (status(the_socket))
  {
  case W5100_SOCK_LISTEN:
    return rc_not_connected;
    
  case W5100_SOCK_ESTABLISHED:
    return rc_ok;
    
  default:
    return rc_invalid_status;
  }
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::waitClientConn(socket_e the_socket)
{ 
  // Wait for client connection (blocking)
  retcode_e rc;
  while ((rc = checkClientConn(the_socket)) == rc_not_connected);
  return rc;    
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::close(socket_e the_socket)
{
  // Check preconditions: if socket status is already CLOSED, do nothing
  if (status(the_socket) == W5100_SOCK_CLOSED)
    return rc_ok;
  
  // Issue the CLOSE command and wait for completion
  for (unsigned int uTry = 0; uTry < uMaxTry; ++uTry)
  {
    write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_CLOSE);
    if (status(the_socket) == W5100_SOCK_CLOSED)
    {
      // Clear any previous event flag
      set_flags(the_socket, 0xFF);
      return rc_ok;
    }
  }
  return rc_close_failed;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::send(socket_e the_socket, uint8_t * the_buffer, uint16_t the_size)
{
  // Check preconditions: socket status must be ESTABLISHED
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return 0;

  return prv_txData(the_socket, the_buffer, the_size);
 }

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::checkSendCompleted(socket_e the_socket)
{
  // Check for completion of data transmission (non blocking)

  // Check preconditions: socket status must be ESTABLISHED
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return rc_invalid_status;

  // Check status      
  uint8_t currFlags = flags(the_socket);
  if ((currFlags & W5100_IR_SEND_OK) && (txSizePending(the_socket) == 0))
    return rc_ok;

  if (currFlags & W5100_IR_TIMEOUT)
    return rc_send_timeout;
  
  return rc_send_pending;
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::waitSendCompleted(socket_e the_socket)
{
  // Wait for completion of data transmission (blocking)

  retcode_e rc;
  while ((rc = checkSendCompleted(the_socket)) == rc_send_pending);
  return rc;    
}

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::receive(socket_e the_socket, uint8_t * the_buffer, uint16_t the_size)
{
  // Check preconditions: socket status must be ESTABLISHED
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return 0;

  return prv_rxData(the_socket, the_buffer, the_size);
}
    
///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::checkReceivePending(socket_e the_socket)
{
  // Wait for received data (non blocking)
  
  // Check preconditions: socket status must be ESTABLISHED
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return rc_invalid_status;

  // Check status      
  uint8_t currFlags = flags(the_socket);
  if ((currFlags & W5100_IR_RECV) && (rxSizePending(the_socket) != 0))
    return rc_ok;

  return rc_no_data;
}

///////////////////////////////////////////////////////////////////////////////

W5100::retcode_e W5100::waitReceivePending(socket_e the_socket)
{
  // Wait for received data (blocking)

  retcode_e rc;
  while ((rc = checkReceivePending(the_socket)) == rc_no_data);
  return rc;    
}

///////////////////////////////////////////////////////////////////////////////
// Socket status inquiry functions

uint8_t W5100::status(socket_e the_socket)
{ return read_Sn_R8(the_socket, W5100_Sn_SR); }

///////////////////////////////////////////////////////////////////////////////

uint8_t W5100::flags(socket_e the_socket)
{ return read_Sn_R8(the_socket, W5100_Sn_IR); }

void W5100::set_flags (socket_e the_socket, uint8_t the_flags)
{ write_Sn_R8(the_socket, W5100_Sn_IR, the_flags); }
  
///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::txMemSize(socket_e the_socket)
{
  switch (the_socket)
  {
  case socket_0: return prv_txMemSize_S0();
  case socket_1: return prv_txMemSize_S1();
  case socket_2: return prv_txMemSize_S2();
  case socket_3: return prv_txMemSize_S3();
  default: return 0x0000;
  }
}

////////////////////////////////////////////////////////////////////////////////

uint16_t W5100::txMemBase(socket_e the_socket)
{
  switch (the_socket)
  {
  case socket_0: return prv_txMemBase_S0();
  case socket_1: return prv_txMemBase_S1();
  case socket_2: return prv_txMemBase_S2();
  case socket_3: return prv_txMemBase_S3();
  default: return 0xFFFF;
  }
}

////////////////////////////////////////////////////////////////////////////////

uint16_t W5100::txSizePending(socket_e the_socket)
{ return txMemSize(the_socket) - read_Sn_R16(the_socket, W5100_Sn_TX_FSR); }

////////////////////////////////////////////////////////////////////////////////

uint16_t W5100::rxMemSize(socket_e the_socket)
{
  switch (the_socket)
  {
  case socket_0: return prv_rxMemSize_S0();
  case socket_1: return prv_rxMemSize_S1();
  case socket_2: return prv_rxMemSize_S2();
  case socket_3: return prv_rxMemSize_S3();
  default: return 0x0000;
  }
}

////////////////////////////////////////////////////////////////////////////////

uint16_t W5100::rxMemBase(socket_e the_socket)
{
  switch (the_socket)
  {
  case socket_0: return prv_rxMemBase_S0();
  case socket_1: return prv_rxMemBase_S1();
  case socket_2: return prv_rxMemBase_S2();
  case socket_3: return prv_rxMemBase_S3();
  default: return 0xFFFF;
  }
}

////////////////////////////////////////////////////////////////////////////////

uint16_t W5100::rxSizePending(socket_e the_socket)
{ return read_Sn_R16(the_socket, W5100_Sn_RX_RSR); }

////////////////////////////////////////////////////////////////////////////////

bool W5100::isOpen(socket_e the_socket)
{ return !isClosed(the_socket); }

////////////////////////////////////////////////////////////////////////////////

bool W5100::isClosed(socket_e the_socket)
{ return status(the_socket) == W5100_SOCK_CLOSED; }

////////////////////////////////////////////////////////////////////////////////

bool W5100::isConnected(socket_e the_socket)
{ return (status(the_socket) == W5100_SOCK_ESTABLISHED); }

////////////////////////////////////////////////////////////////////////////////

bool W5100::canReceiveData(socket_e the_socket)
{
  // Check preconditions: socket status must be ESTABLISHED
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return false;
  
  // Note that this bit is automatically set to 1 in two cases:
  // a) when data are received
  // b) when more data are still available after having completed execution
  //    of a RECV command
  return (flags(the_socket) & W5100_IR_RECV);
}

////////////////////////////////////////////////////////////////////////////////

bool W5100::canTransmitData(socket_e the_socket)
{
  // Check preconditions: socket status must be ESTABLISHED
  if (status(the_socket) != W5100_SOCK_ESTABLISHED)
    return false;

  // Here we do not check TX_Sn_FSR because it must be checked during the send process  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Utility functions for reading/writing registers

void W5100::write_R8(uint16_t the_addr, uint8_t the_data)
{
  prv_setSS();  
  SPI.transfer(0xF0);
  SPI.transfer(the_addr >> 8);
  SPI.transfer(the_addr & 0xFF);
  SPI.transfer(the_data);
  prv_resetSS();
}

void W5100::write_R16(uint16_t the_addr, uint16_t the_data)
{
  write_R8(the_addr, the_data >> 8);
  write_R8(++the_addr, the_data & 0xFF);
}

void W5100::write_Sn_R8(socket_e the_socket, uint16_t the_addr, uint8_t the_data)
{
  the_addr &= 0x00FF;
  
  switch (the_socket)
  {
  case socket_0: write_R8(W5100_S0_MASK | the_addr, the_data); return;
  case socket_1: write_R8(W5100_S1_MASK | the_addr, the_data); return;
  case socket_2: write_R8(W5100_S2_MASK | the_addr, the_data); return;
  case socket_3: write_R8(W5100_S3_MASK | the_addr, the_data); return;
  default: return;
  }
}

void W5100::write_Sn_R16(socket_e the_socket, uint16_t the_addr, uint16_t the_data)
{
  the_addr &= 0x00FF;
  
  switch (the_socket)
  {
  case socket_0: write_R16(W5100_S0_MASK | the_addr, the_data); return;
  case socket_1: write_R16(W5100_S1_MASK | the_addr, the_data); return;
  case socket_2: write_R16(W5100_S2_MASK | the_addr, the_data); return;
  case socket_3: write_R16(W5100_S3_MASK | the_addr, the_data); return;
  default: return;
  }
}

///////////////////////////////////////////////////////////////////////////////

uint8_t W5100::read_R8(uint16_t the_addr)
{
  prv_setSS();  
  SPI.transfer(0x0F);
  SPI.transfer(the_addr >> 8);
  SPI.transfer(the_addr & 0xFF);
  uint8_t d8 = SPI.transfer(0);
  prv_resetSS();
  return d8;
}

uint16_t W5100::read_R16(uint16_t the_addr)
{
  uint16_t d16 = read_R8(the_addr);
  d16 = ((d16 << 8) & 0xFF00) + read_R8(++the_addr);
  return d16;
}

uint8_t W5100::read_Sn_R8(socket_e the_socket, uint16_t the_addr)
{
  the_addr &= 0x00FF;
  
  switch (the_socket)
  {
  case socket_0: return read_R8(W5100_S0_MASK | the_addr);
  case socket_1: return read_R8(W5100_S1_MASK | the_addr);
  case socket_2: return read_R8(W5100_S2_MASK | the_addr);
  case socket_3: return read_R8(W5100_S3_MASK | the_addr);
  default: return 0xFF;
  }
}

uint16_t W5100::read_Sn_R16(socket_e the_socket, uint16_t the_addr)
{
  the_addr &= 0x00FF;
  
  switch (the_socket)
  {
  case socket_0: return read_R16(W5100_S0_MASK | the_addr);
  case socket_1: return read_R16(W5100_S1_MASK | the_addr);
  case socket_2: return read_R16(W5100_S2_MASK | the_addr);
  case socket_3: return read_R16(W5100_S3_MASK | the_addr);
  default: return 0xFFFF;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Private member functions

uint16_t W5100::prv_txData(socket_e the_socket, uint8_t * the_buffer, uint16_t the_size)
{
  // This function writes data to tx memory and returns the amount of data acutally written
  
  // Compute write address limits
  uint16_t memBegin        = txMemBase(the_socket);
  uint16_t memEnd          = memBegin + txMemSize(the_socket);
  uint16_t writtenActually = 0;
    
  while (the_size)
  {
    // Check preconditions: data must be available and socket must be in correct state
    if (!canTransmitData(the_socket))
      return writtenActually;

    // Compute howmany bytes can be written
    uint16_t available = read_Sn_R16(the_socket, W5100_Sn_TX_FSR);
    if (available == 0) return writtenActually;
    
    uint16_t writeOfs   = read_Sn_R16(the_socket, W5100_Sn_TX_WR);
    uint16_t writeStart = memBegin + (writeOfs & (txMemSize(the_socket) - 1));
    uint16_t sizeToTop  = memEnd - writeStart;
    uint16_t canWrite   = (available > the_size ? the_size : available);
    canWrite = (sizeToTop > canWrite ? canWrite : sizeToTop);
    
    // Copy this portion of bytes from buffer to tx memory
    for (uint16_t u = 0; u < canWrite; ++u, ++the_buffer, ++writeStart)
      write_R8(writeStart, *the_buffer);
    
    // Update counters and pointers
    the_size        -= canWrite;
    writtenActually += canWrite;
    if (writeStart >= memEnd) writeStart = memBegin;
    
    // Signal completion of this portion of reading
    set_flags(the_socket, W5100_IR_SEND_OK);
    set_flags(the_socket, W5100_IR_TIMEOUT);
    write_Sn_R16(the_socket, W5100_Sn_TX_WR, writeOfs + canWrite);
    write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_SEND);
  }
  
  return writtenActually;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::prv_rxData(socket_e the_socket, uint8_t * the_buffer, uint16_t the_size)
{
  // This function reads data from rx memory and returns the amount of data acutally read
  
  // Compute read address limits
  uint16_t memBegin     = rxMemBase(the_socket);
  uint16_t memEnd       = memBegin + rxMemSize(the_socket);
  uint16_t readActually = 0;
    
  while (the_size)
  {
    // Check preconditions: data must be available and socket must be in correct state
    if (!canReceiveData(the_socket))
      return readActually;

    // Compute howmany bytes can be read
    uint16_t available = read_Sn_R16(the_socket, W5100_Sn_RX_RSR);
    uint16_t readOfs   = read_Sn_R16(the_socket, W5100_Sn_RX_RD);
    uint16_t readStart = memBegin + (readOfs & (rxMemSize(the_socket) - 1));
    uint16_t sizeToTop = memEnd - readStart;
    uint16_t canRead   = (available > the_size ? the_size : available);
    canRead = (sizeToTop > canRead ? canRead : sizeToTop);
    
    // Copy this portion of bytes from rx memory to buffer
    for (uint16_t u = 0; u < canRead; ++u, ++the_buffer, ++readStart)
      *the_buffer = read_R8(readStart);
    
    // Update counters and pointers
    the_size     -= canRead;
    readActually += canRead;
    if (readStart >= memEnd) readStart = memBegin;
    
    // Signal completion of this portion of reading
    set_flags(the_socket, W5100_IR_RECV);
    set_flags(the_socket, W5100_IR_TIMEOUT);
    write_Sn_R16(the_socket, W5100_Sn_RX_RD, readOfs + canRead);
    write_Sn_R8(the_socket, W5100_Sn_CR, W5100_COMMAND_RECV);
  }
  
  return readActually;
}  

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::prv_txMemSize_S0()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S0_TMSR_VAL(read_R8(W5100_TMSR));
  return memSize; 
}

uint16_t W5100::prv_txMemSize_S1()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S1_TMSR_VAL(read_R8(W5100_TMSR));
  return memSize; 
}

uint16_t W5100::prv_txMemSize_S2()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S2_TMSR_VAL(read_R8(W5100_TMSR));
  return memSize; 
}

uint16_t W5100::prv_txMemSize_S3()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S3_TMSR_VAL(read_R8(W5100_TMSR));
  return memSize; 
}

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::prv_txMemBase_S0()
{ return W5100_MEM_TX_BASE; }

uint16_t W5100::prv_txMemBase_S1()
{ 
  static uint16_t memBase = 0x0000;
  if (!memBase) memBase = prv_txMemBase_S0() + prv_txMemSize_S0();
  return memBase;
}

uint16_t W5100::prv_txMemBase_S2()
{ 
  static uint16_t memBase = 0x0000;
  if (!memBase) memBase = prv_txMemBase_S1() + prv_txMemSize_S1();
  return memBase;
}

uint16_t W5100::prv_txMemBase_S3()
{ 
  static uint16_t memBase = 0x0000;
  if (!memBase) memBase = prv_txMemBase_S2() + prv_txMemSize_S2();
  return memBase;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::prv_rxMemSize_S0()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S0_RMSR_VAL(read_R8(W5100_RMSR));
  return memSize; 
}

uint16_t W5100::prv_rxMemSize_S1()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S1_RMSR_VAL(read_R8(W5100_RMSR));
  return memSize; 
}

uint16_t W5100::prv_rxMemSize_S2()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S2_RMSR_VAL(read_R8(W5100_RMSR));
  return memSize; 
}

uint16_t W5100::prv_rxMemSize_S3()
{ 
  static uint16_t memSize = 0x0000;
  if (!memSize) memSize = oneKB << W5100_S3_RMSR_VAL(read_R8(W5100_RMSR));
  return memSize; 
}

///////////////////////////////////////////////////////////////////////////////

uint16_t W5100::prv_rxMemBase_S0()
{ return W5100_MEM_RX_BASE; }

uint16_t W5100::prv_rxMemBase_S1()
{
  static uint16_t memBase = 0x0000;
  if (!memBase) memBase = prv_rxMemBase_S0() + prv_rxMemSize_S0();
  return memBase;
}

uint16_t W5100::prv_rxMemBase_S2()
{
  static uint16_t memBase = 0x0000;
  if (!memBase) memBase = prv_rxMemBase_S1() + prv_rxMemSize_S1();
  return memBase;
}

uint16_t W5100::prv_rxMemBase_S3()
{
  static uint16_t memBase = 0x0000;
  if (!memBase) memBase = prv_rxMemBase_S2() + prv_rxMemSize_S2();
  return memBase;
}
  
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

W5100::mac_address_t::mac_address_t(uint16_t the_regAddr)
{
  my_addr[0] = W5100::read_R8(the_regAddr++);
  my_addr[1] = W5100::read_R8(the_regAddr++);
  my_addr[2] = W5100::read_R8(the_regAddr++);
  my_addr[3] = W5100::read_R8(the_regAddr++);
  my_addr[4] = W5100::read_R8(the_regAddr++);
  my_addr[5] = W5100::read_R8(the_regAddr  );
}

W5100::mac_address_t::mac_address_t(socket_e the_socket)
{
  switch (the_socket)
  {
  case socket_0: *this = mac_address_t(W5100_S0_DHAR0); break;
  case socket_1: *this = mac_address_t(W5100_S1_DHAR0); break;
  case socket_2: *this = mac_address_t(W5100_S2_DHAR0); break;
  case socket_3: *this = mac_address_t(W5100_S3_DHAR0); break;
  default      : *this = mac_address_t(0,0,0,0,0,0); break;
  }
}

W5100::mac_address_t::mac_address_t(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5)
{
  my_addr[0] = a0;
  my_addr[1] = a1;
  my_addr[2] = a2;
  my_addr[3] = a3;
  my_addr[4] = a4;
  my_addr[5] = a5;
}

W5100::mac_address_t::mac_address_t(const uint8_t * the_mac)
{
  my_addr[0] = the_mac[0];
  my_addr[1] = the_mac[1];
  my_addr[2] = the_mac[2];
  my_addr[3] = the_mac[3];
  my_addr[4] = the_mac[4];
  my_addr[5] = the_mac[5];
}

void W5100::mac_address_t::set(uint16_t the_regAddr) const
{
  write_R8(the_regAddr++, my_addr[0]);
  write_R8(the_regAddr++, my_addr[1]);
  write_R8(the_regAddr++, my_addr[2]);
  write_R8(the_regAddr++, my_addr[3]);
  write_R8(the_regAddr++, my_addr[4]);
  write_R8(the_regAddr++, my_addr[5]);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

W5100::ipv4_address_t::ipv4_address_t(uint16_t the_regAddr)
{
  my_addr[0] = W5100::read_R8(the_regAddr++);
  my_addr[1] = W5100::read_R8(the_regAddr++);
  my_addr[2] = W5100::read_R8(the_regAddr++);
  my_addr[3] = W5100::read_R8(the_regAddr++);
}

W5100::ipv4_address_t::ipv4_address_t(socket_e the_socket)
{
  switch (the_socket)
  {
  case socket_0: *this = ipv4_address_t(W5100_S0_DIPR0); break;
  case socket_1: *this = ipv4_address_t(W5100_S1_DIPR0); break;
  case socket_2: *this = ipv4_address_t(W5100_S2_DIPR0); break;
  case socket_3: *this = ipv4_address_t(W5100_S3_DIPR0); break;
  default      : *this = ipv4_address_t(0,0,0,0); break;
  }
}

W5100::ipv4_address_t::ipv4_address_t(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3)
{
  my_addr[0] = ip0;
  my_addr[1] = ip1;
  my_addr[2] = ip2;
  my_addr[3] = ip3;
}

W5100::ipv4_address_t::ipv4_address_t(const uint8_t * the_ip)
{
  my_addr[0] = the_ip[0];
  my_addr[1] = the_ip[1];
  my_addr[2] = the_ip[2];
  my_addr[3] = the_ip[3];
}

void W5100::ipv4_address_t::set(uint16_t the_regAddr) const
{
  write_R8(the_regAddr++, my_addr[0]);
  write_R8(the_regAddr++, my_addr[1]);
  write_R8(the_regAddr++, my_addr[2]);
  write_R8(the_regAddr++, my_addr[3]);
}

///////////////////////////////////////////////////////////////////////////////

