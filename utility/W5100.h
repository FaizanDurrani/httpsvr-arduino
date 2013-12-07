////////////////////////////////////////////////////////////////////////////////
//
//  W5100.h - Definition of W5100 driver
//  ECA 26Oct2013
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef W5100_H
#define W5100_H

#include <Arduino.h>
#include <SPI.h>
#include "W5100Defs.h"

////////////////////////////////////////////////////////////////////////////////

class W5100
{
public:
  /////////////////////////////////////////////////////////
  // Socket numbers
  enum socket_e
  {
    socket_undefined = -1,
    socket_begin = 0,
    socket_0 = socket_begin,
    socket_1,
    socket_2,
    socket_3,
    socket_end
  };
  
  template<typename T>
  static inline socket_e socket_cast(T u)
  { return static_cast<socket_e>(u); }
  
  /////////////////////////////////////////////////////////
  // Return codes
  enum retcode_e
  {
    rc_ok                 = 0,
    rc_invalid_status     ,
    rc_invalid_socket     ,
    rc_invalid_port       ,
    rc_open_failed        ,
    rc_listen_failed      ,
    rc_connect_failed     ,
    rc_connect_timeout    ,
    rc_disconnect_failed  ,
    rc_disconnect_timeout ,
    rc_not_connected      ,
    rc_close_failed       ,
    rc_no_data            ,
    rc_send_pending       ,
    rc_send_timeout       ,
    rc_unknown
  };
  
  /////////////////////////////////////////////////////////
  // Utility class for manipulation of MAC address
  class mac_address_t
  {
  public:
    // This constructor reads a mac address from W5100 registers.
    // By default, it reads the SHAR (Source Hardware Address);
    // Passing Sn_DHAR0 as parameter, it reads the Socket n's Destination Hardware Address
    explicit mac_address_t(uint16_t the_regAddr = W5100_SHAR0);
    explicit mac_address_t(socket_e the_socket);

    // These constructors store a mac address in the member variables,
    // but do not write it to W5100. Use method "set" to write it.
    mac_address_t(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5);
    explicit mac_address_t(const uint8_t * the_mac);
    
  public:
    // This method writes the mac address to W5100 registers.
    // By default, it writes the SHAR (Source Hardware Address);
    // Passing Sn_DHAR0 as parameter, it writes the Socket n's Destination Hardware Address
    void              set(uint16_t the_regAddr = W5100_SHAR0) const;

    // These methods return each single component of the MAC address
    // stored in the member variable
    uint8_t           a0 () const { return my_addr[0]; }
    uint8_t           a1 () const { return my_addr[1]; }
    uint8_t           a2 () const { return my_addr[2]; }
    uint8_t           a3 () const { return my_addr[3]; }
    uint8_t           a4 () const { return my_addr[4]; }
    uint8_t           a5 () const { return my_addr[5]; }
    
  private:
    uint8_t           my_addr[6];
  };
  
  /////////////////////////////////////////////////////////
  // Utility class for manipulation of IP address (IPV4)
  class ipv4_address_t
  {
  public:
    // This constructor reads an IP address from W5100 registers.
    // By default, it reads the SIPR (Source IP Address);
    // Passing Sn_DIPR0 as parameter, it reads the Socket n's Destination IP Address
    explicit ipv4_address_t(uint16_t the_regAddr = W5100_SIPR0);
    explicit ipv4_address_t(socket_e the_socket);

    // This constructors store an IP address in the member variable,
    // but do not write it to W5100. Use method "set" to write it.
    // The second form can be used directly with a IPAddress.raw_address()
    ipv4_address_t(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3);
    explicit ipv4_address_t(const uint8_t * the_ip);
    
  public:
    // This method writes the IP address to W5100 registers.
    // By default, it writes the SIPR (Source IP Address);
    // Passing Sn_DIPR0 as parameter, it writes the Socket n's Destination IP Address
    void              set(uint16_t the_regAddr = W5100_SIPR0) const;

    // These methods return each single component of the IP address
    // stored in the member variable
    uint8_t           ip0 () const { return my_addr[0]; }
    uint8_t           ip1 () const { return my_addr[1]; }
    uint8_t           ip2 () const { return my_addr[2]; }
    uint8_t           ip3 () const { return my_addr[3]; }
    
  private:
    uint8_t           my_addr[4];
  };
  
  /////////////////////////////////////////////////////////
  
public:
  ~W5100();
  
public:
  // Initialization and termination
  static void         begin               (const mac_address_t& the_macAddr, const ipv4_address_t& the_ipAddr);
  static void         terminate           ();

public:
  // Socket command functions
  static retcode_e    open                (socket_e the_socket, uint16_t the_port);
  static retcode_e    listen              (socket_e the_socket);
  static retcode_e    connect             (socket_e the_socket, const ipv4_address_t& the_ipAddr, uint16_t the_port);
  static retcode_e    disconnect          (socket_e the_socket);
  static retcode_e    checkClientConn     (socket_e the_socket);
  static retcode_e    waitClientConn      (socket_e the_socket);
  static retcode_e    close               (socket_e the_socket);
  static uint16_t     send                (socket_e the_socket, uint8_t * the_buffer, uint16_t the_size);
  static retcode_e    checkSendCompleted  (socket_e the_socket);
  static retcode_e    waitSendCompleted   (socket_e the_socket);
  static uint16_t     receive             (socket_e the_socket, uint8_t * the_buffer, uint16_t the_size);
  static retcode_e    checkReceivePending (socket_e the_socket);
  static retcode_e    waitReceivePending  (socket_e the_socket);
  
  // Socket status inquiry functions
  static uint8_t      status              (socket_e the_socket);

  static uint8_t      flags               (socket_e the_socket);
  static void         set_flags           (socket_e the_socket, uint8_t the_flags);
  
  static uint16_t     txMemSize           (socket_e the_socket);
  static uint16_t     txMemBase           (socket_e the_socket);
  static uint16_t     txSizePending       (socket_e the_socket);
  static uint16_t     rxMemSize           (socket_e the_socket);
  static uint16_t     rxMemBase           (socket_e the_socket);
  static uint16_t     rxSizePending       (socket_e the_socket);

  static bool         isOpen              (socket_e the_socket);
  static bool         isClosed            (socket_e the_socket);
  static bool         isConnected         (socket_e the_socket);
  static bool         canReceiveData      (socket_e the_socket);
  static bool         canTransmitData     (socket_e the_socket);

public:  
  // Utility functions for reading/writing registers
  static void         write_R8            (uint16_t the_addr, uint8_t the_data );
  static void         write_R16           (uint16_t the_addr, uint16_t the_data);
  static void         write_Sn_R8         (socket_e the_socket, uint16_t the_addr, uint8_t the_data );
  static void         write_Sn_R16        (socket_e the_socket, uint16_t the_addr, uint16_t the_data);

  static uint8_t      read_R8             (uint16_t the_addr);
  static uint16_t     read_R16            (uint16_t the_addr);
  static uint8_t      read_Sn_R8          (socket_e the_socket, uint16_t the_addr);
  static uint16_t     read_Sn_R16         (socket_e the_socket, uint16_t the_addr);

private:  
  static uint16_t     prv_txData      (socket_e the_socket, uint8_t * the_buffer, uint16_t the_size);
  static uint16_t     prv_rxData      (socket_e the_socket, uint8_t * the_buffer, uint16_t the_size);

  static uint16_t     prv_txMemSize_S0();
  static uint16_t     prv_txMemSize_S1();
  static uint16_t     prv_txMemSize_S2();
  static uint16_t     prv_txMemSize_S3();
  
  static uint16_t     prv_txMemBase_S0();
  static uint16_t     prv_txMemBase_S1();
  static uint16_t     prv_txMemBase_S2();
  static uint16_t     prv_txMemBase_S3();
  
  static uint16_t     prv_rxMemSize   (socket_e the_socket);
  static uint16_t     prv_rxMemSize_S0();
  static uint16_t     prv_rxMemSize_S1();
  static uint16_t     prv_rxMemSize_S2();
  static uint16_t     prv_rxMemSize_S3();
  
  static uint16_t     prv_rxMemBase   (socket_e the_socket);
  static uint16_t     prv_rxMemBase_S0();
  static uint16_t     prv_rxMemBase_S1();
  static uint16_t     prv_rxMemBase_S2();
  static uint16_t     prv_rxMemBase_S3();
  
private:
  W5100(); // An object of this class cannot be instantiated

  // Initialization and utility functions for SPI  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  inline static void  prv_initSS    ()      { DDRB  |=  _BV(4); };
  inline static void  prv_setSS     ()      { PORTB &= ~_BV(4); };
  inline static void  prv_resetSS   ()      { PORTB |=  _BV(4); };
#elif defined(__AVR_ATmega32U4__)
  inline static void  prv_initSS    ()      { DDRB  |=  _BV(6); };
  inline static void  prv_setSS     ()      { PORTB &= ~_BV(6); };
  inline static void  prv_resetSS   ()      { PORTB |=  _BV(6); }; 
#elif defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB162__)
  inline static void  prv_initSS    ()      { DDRB  |=  _BV(0); };
  inline static void  prv_setSS     ()      { PORTB &= ~_BV(0); };
  inline static void  prv_resetSS   ()      { PORTB |=  _BV(0); }; 
#else
  inline static void  prv_initSS    ()      { DDRB  |=  _BV(2); };
  inline static void  prv_setSS     ()      { PORTB &= ~_BV(2); };
  inline static void  prv_resetSS   ()      { PORTB |=  _BV(2); };
#endif

friend class mac_address_t;
friend class ip_address_t;
};

////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef W5100_H

