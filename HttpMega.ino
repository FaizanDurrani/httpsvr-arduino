////////////////////////////////////////////////////////////////////////////////
//
// HttpMega - A development, test and sample application for the HttpSvr library
// ECA 07Dec2013
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#define HTTPMEGA_APPNAME "HTTPMEGA"
#define HTTPMEGA_APPVER  "0.8.0"

////////////////////////////////////////////////////////////////////////////////

// If you want to use LCD display, define HTTP_USE_LCD.
// On UNO, leave it undefined to reduce program and data memory usage
//#undef  HTTPSVR_USE_LCD
#define HTTPSVR_USE_LCD

// Definitions of pins connected to diagnostic LEDs
#define DBG_PIN                     9  // GREEN  - Customize this according to your configuration
#define W5100_DBG_PIN0              22 // RED    - Customize this according to your configuration
#define W5100_DBG_PIN1              23 // YELLOW - Customize this according to your configuration

////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

// The following #includes should not be due. However, they must be added here
#include <SD.h>
#include <SPI.h>
#include <HttpSvr.h>
#include <UdpPeer.h>

#ifdef HTTPSVR_USE_LCD
  #include <LiquidCrystal.h>
  #include "LcdUtils.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Memory monitoring
extern int    __bss_end;
extern void * __brkval ;

int memFree()
{
  int freeValue;
  if ((int)__brkval == 0) freeValue = ((int)&__bss_end);
  else                    freeValue = ((int)&freeValue) - ((int)&__brkval);
  return freeValue;
}

#ifdef HTTPSVR_USE_LCD
void writeFreeMem()
{ 
  writeLine1(String("FreeBytes:") + String(memFree()));
}
#endif

////////////////////////////////////////////////////////////////////////////////
// IP printing utility
#ifdef HTTPSVR_USE_LCD
void local_writeIPdigit(char * s, uint8_t b)
{
  char *p = s;
  if      (b >= 200) {*p++ = '2'; b -= 200; }
  else if (b >= 100) {*p++ = '1'; b -= 100; }
  else               {*p++ = '0'; }
  *p++ = '0' + (b / 10);
  *p++ = '0' + (b % 10);
  *p=0;
}

void writeIP(const IPAddress& the_ip)
{
  writeLine1("IP Address:");
  char sIP[16];
  memset(sIP, ' ', sizeof(sIP));
  for (int i = 0; i < 4; ++i)
  {
    local_writeIPdigit(&sIP[4*i], the_ip[i]);
    if (i<3) sIP[4*i+3] = '.';
  }
  sIP[sizeof(sIP)-1] = 0;
  writeLine2(sIP);
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Definition of connection parameters - Change to your fit your configuration
static const uint8_t   HTTPMEGA_MAC_ADDRESS[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x42, 0xC6 };

static const IPAddress HTTPMEGA_STATIC_IP   ( 10, 42,  0, 27);
static const IPAddress HTTPMEGA_GATEWAY_IP  ( 10, 42,  0,  1); // Set to (0,0,0,0) if not needed
static const IPAddress HTTPMEGA_SUBNET_MASK (255,255,255,  0); // Set to (0,0,0,0) if not needed
static const int       HTTPMEGA_TCP_PORT = 80;
static const int       HTTPMEGA_UDP_PORT = 8888;

static const int       HTTPMEGA_CS_PIN = 4;  // On the Ethernet Shield, CS is pin 4.
static const int       HTTPMEGA_SS_PIN = 53; // SS pin is 10 on most Arduino boards, 53 on the Mega

////////////////////////////////////////////////////////////////////////////////
// NTP-related definitions
static const int       NTP_PORT = 123;

static const IPAddress NTP_SERVER_IP (132, 163, 4, 101);   // time-a.timefreq.bldrdoc.gov NTP server
// static const IPAddress NTP_SERVER_IP(132, 163, 4, 102);    // time-b.timefreq.bldrdoc.gov NTP server
// static const IPAddress NTP_SERVER_IP(132, 163, 4, 103);    // time-c.timefreq.bldrdoc.gov NTP server

const unsigned int     NTP_PACKET_SIZE = 48;               // NTP time stamp is in the first 48 bytes of the message
byte                   NTP_packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming and outgoing packets 

////////////////////////////////////////////////////////////////////////////////
// Definition of the HTTP Server Object
HttpSvr HTTPMEGA_httpSvr;

////////////////////////////////////////////////////////////////////////////////
// Resource Provider for "/"
bool rpRoot(ClientProxy& the_client, http_e::method the_method, const char * the_url)
{
  // In this resource provider we just redirect to "/www/index.htm"
  return HTTPMEGA_httpSvr.sendResFile(the_client, "/www/index.htm");
}

////////////////////////////////////////////////////////////////////////////////
// Resource Provider for "/memInfo"
bool rpMemInfo(ClientProxy& the_client, http_e::method the_method, const char * the_url)
{
  static const uint16_t uBufferLen = 32;
  uint8_t sBuffer[uBufferLen];

  // Skip all headers and goto message body
  uint16_t uBodyLen = HTTPMEGA_httpSvr.skipToBody(the_client);
  if (uBodyLen < 5) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  if (uBodyLen >= uBufferLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }

  // Read message body. We expect a string like 'info=mem'
  // Remember that, while reading the body, we MUST ALWAYS read EXACTLY
  // the number of bytes returned by skipToBody.
  uint16_t uRead = the_client.readBuffer(sBuffer, uBodyLen);
  if (uRead != uBodyLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  sBuffer[uBodyLen] = 0;

  // Compose the response string
  memset(sBuffer, 0, uBufferLen);
  ltoa(memFree(), (char *)sBuffer, 10);
  HTTPMEGA_httpSvr.sendResponseOkWithContent(the_client, strlen((const char *)sBuffer));
  return HTTPMEGA_httpSvr.sendResponse(the_client, (const char *)sBuffer);
}

////////////////////////////////////////////////////////////////////////////////
// Resource Provider for "/timeInfo"

static String local_paddedString(uint32_t u)
{
  if (u < 10) return String("0") + String(u, DEC);
  else return String(u, DEC);
}

////////////////////////////////////////////////////////////////////////////////

void sendNtpRequest(UdpPeer& the_udp, byte *the_buffer, unsigned int the_bufferSize)
{
  // Clear buffer
  memset(the_buffer, 0, the_bufferSize);
  
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  the_buffer[ 0] = 0b11100011;      // LI, Version, Mode
  the_buffer[ 1] = 0;               // Stratum, or type of clock
  the_buffer[ 2] = 6;               // Polling Interval
  the_buffer[ 3] = 0xEC;            // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  the_buffer[12] = 49; 
  the_buffer[13] = 0x4E;
  the_buffer[14] = 49;
  the_buffer[15] = 52;

  // TODO : UDP header? double check if it must be prepended here or not
  
  // All NTP fields have been given values. We can send a packet requesting a timestamp: 		   
  the_udp.writeBuffer(the_buffer, the_bufferSize);
}

////////////////////////////////////////////////////////////////////////////////

static String HTTPMEGA_sNtpTime = "n.a.";

String getNtpTime()
{
  // Clear buffer
  memset(NTP_packetBuffer, 0, sizeof(NTP_packetBuffer));
  
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  NTP_packetBuffer[ 0] = 0b11100011;      // LI, Version, Mode
  NTP_packetBuffer[ 1] = 0;               // Stratum, or type of clock
  NTP_packetBuffer[ 2] = 6;               // Polling Interval
  NTP_packetBuffer[ 3] = 0xEC;            // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  NTP_packetBuffer[12] = 49; 
  NTP_packetBuffer[13] = 0x4E;
  NTP_packetBuffer[14] = 49;
  NTP_packetBuffer[15] = 52;

  // All NTP fields have been given values. We can send a packet requesting a timestamp: 		   
  UdpPeer udp;
  udp.open(HTTPMEGA_UDP_PORT, NTP_SERVER_IP, NTP_PORT);
  udp.writeBuffer(NTP_packetBuffer, sizeof(NTP_packetBuffer));
  delay(1000);

  // Start reading the UDP datagram header (8 bytes)
  // TODO : Keep length and possibily checksum for flushing input buffer and checking data
  // TODO : Embed in UdpPeer a function for reading and validating header
  uint8_t udpHeader[8];
  if (udp.readBuffer(udpHeader, sizeof(udpHeader)) != sizeof(udpHeader))
    return "Bad Datagram!";

  // Check response. If valid, read into local buffer
  if (udp.readBuffer(NTP_packetBuffer, sizeof(NTP_packetBuffer)) != sizeof(NTP_packetBuffer))
    return "No Time!";
  
  // The timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, extract the two words, then combine the four bytes
  // (two words) into a long integer. This is NTP time, in seconds since Jan 1 1900.
  uint32_t highWord = word(NTP_packetBuffer[40], NTP_packetBuffer[41]);
  uint32_t lowWord  = word(NTP_packetBuffer[42], NTP_packetBuffer[43]);  
  uint32_t secsSince1900 = highWord << 16 | lowWord;  

  // Now we can convert the time info and generate the string

  // First generate Unix Time, i.e. the time in seconds since Jan 1 1970.
  // Being NTP time based on Jan 01 1900, we must subtract the number of
  // seconds elapsed between the two dates.
  // According to the Time protocol in RFC 868 it is 2208988800L.
  const uint32_t secsSeventyYears = 2208988800UL;     
  uint32_t epoch = secsSince1900 - secsSeventyYears;  

  // Now we build UTC time, i.e. the time at Greenwich Meridian (GMT)
  uint32_t HH = (epoch  % 86400L) / 3600;    // 86400 secs/day, 3600 secs/hour
  uint32_t MM = (epoch % 3600) / 60;         // 60 secs/min
  uint32_t SS = (epoch % 60);

  const char am[] = "AM";
  const char pm[] = "PM";
  const char *ampm = am;
  if (HH > 12) { ampm = pm; HH = HH % 12; }

  return local_paddedString(HH) + ":" + local_paddedString(MM) + ":" + local_paddedString(SS) + " " + ampm;
}

///////////////////////////////////////////////////////////////////////////////

bool rpTimeInfo(ClientProxy& the_client, http_e::method the_method, const char * the_url)
{
  static const uint16_t uBufferLen = 32;
  uint8_t sBuffer[uBufferLen];

  // Skip all headers and goto message body
  uint16_t uBodyLen = HTTPMEGA_httpSvr.skipToBody(the_client);
  if (uBodyLen < 5) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  if (uBodyLen >= uBufferLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }

  // Read message body. We expect a string like 'info=time'
  // Remember that, while reading the body, we MUST ALWAYS read EXACTLY
  // the number of bytes returned by skipToBody.
  uint16_t uRead = the_client.readBuffer(sBuffer, uBodyLen);
  if (uRead != uBodyLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  sBuffer[uBodyLen] = 0;

  // Compose the response string
  HTTPMEGA_sNtpTime.toCharArray((char *)sBuffer, uBufferLen);
  HTTPMEGA_httpSvr.sendResponseOkWithContent(the_client, strlen((const char *)sBuffer));
  return HTTPMEGA_httpSvr.sendResponse(the_client, (const char *)sBuffer);
}

////////////////////////////////////////////////////////////////////////////////
// Resource Provider for "/digitalRead"
bool rpDigitalRead(ClientProxy& the_client, http_e::method the_method, const char * the_url)
{
  static const uint16_t uBufferLen = 32;
  uint8_t sBuffer[uBufferLen];

  // Skip all headers and goto message body
  uint16_t uBodyLen = HTTPMEGA_httpSvr.skipToBody(the_client);
  if (uBodyLen < 5) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  if (uBodyLen >= uBufferLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }

  // Read message body. We expect a string like 'name=XX' where XX
  // is the pin number.
  // Remember that, while reading the body, we MUST ALWAYS read EXACTLY
  // the number of bytes returned by skipToBody.
  uint16_t uRead = the_client.readBuffer(sBuffer, uBodyLen);
  if (uRead != uBodyLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  sBuffer[uBodyLen] = 0;

  // Read pin number
  void *pv = sBuffer + 5;
  uint8_t pinId = atoi(static_cast<char *>(pv));

  // Compose the response string
  bool bValue = digitalRead(pinId);
  HTTPMEGA_httpSvr.sendResponseOkWithContent(the_client, 1);
  return HTTPMEGA_httpSvr.sendResponse(the_client, (bValue ? "1" : "0"));
}

////////////////////////////////////////////////////////////////////////////////
// Resource Provider for "/digitalWrite"
bool rpDigitalWrite(ClientProxy& the_client, http_e::method the_method, const char * the_url)
{
  static const uint16_t uBufferLen = 32;
  uint8_t sBuffer[uBufferLen];

  // Skip all headers and goto message body
  // Skip all headers and goto message body
  uint16_t uBodyLen = HTTPMEGA_httpSvr.skipToBody(the_client);
  if (uBodyLen < 5) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  if (uBodyLen >= uBufferLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }

  // Read message body. We expect a string like 'name=XX&value=Y' where XX
  // is the pin number and Y is "true" or "false".
  // Remember that, while reading the body, we MUST ALWAYS read EXACTLY
  // the number of bytes returned by skipToBody.
  uint16_t uRead = the_client.readBuffer(sBuffer, uBodyLen);
  if (uRead != uBodyLen) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  sBuffer[uBodyLen] = 0;

  // Read pin number and value
  char *ss= static_cast<char *>(static_cast<void *>(sBuffer+5));
  uint8_t pinId = atoi(ss);

  while ((*ss) && (*ss != '=')) ss++;
  if (!*ss) { HTTPMEGA_httpSvr.sendResponseBadRequest(the_client); return false; }
  bool bValue = (*(++ss) == 't');

  // Set digital pin value
  digitalWrite(pinId, bValue);
  
  // Compose the response string
  bValue = digitalRead(pinId);
  HTTPMEGA_httpSvr.sendResponseOkWithContent(the_client, 1);
  return HTTPMEGA_httpSvr.sendResponse(the_client, (bValue ? "1" : "0"));
}

////////////////////////////////////////////////////////////////////////////////
// Configuration of WizNet device

void configWiz(const uint8_t *  the_macAddr,        // Local MAC address
               const IPAddress& the_srcIpAddr,      // Local IP
               const IPAddress& the_subnetMask,     // Subnet Mask
               const IPAddress& the_gatewayIpAddr)  // Gateway IP
{
  // Initialize W5100
  W5100::begin(W5100::mac_address_t(the_macAddr), the_srcIpAddr, the_subnetMask, the_gatewayIpAddr);
               
  // Modify default initialization of W5100 sockets to prevent buffer overflow
  // in receive operation.
  for (uint8_t sn = W5100::socket_begin; sn < W5100::socket_end; ++sn)
    W5100::write_Sn_R8(W5100::socket_cast(sn), W5100_Sn_MR, W5100::read_Sn_R8(W5100::socket_cast(sn), W5100_Sn_MR) | W5100_ND);
}

////////////////////////////////////////////////////////////////////////////////
// Configuration of HTTP server object

void configHttpServer()
{
  // Bind resource providers
  HTTPMEGA_httpSvr.bindUrl("/"            , &rpRoot        );
  HTTPMEGA_httpSvr.bindUrl("/memInfo"     , &rpMemInfo     );
  HTTPMEGA_httpSvr.bindUrl("/timeInfo"    , &rpTimeInfo    );
  HTTPMEGA_httpSvr.bindUrl("/digitalRead" , &rpDigitalRead );
  HTTPMEGA_httpSvr.bindUrl("/digitalWrite", &rpDigitalWrite);

  // Start the server, specifying the SS and CS pins for SD card
  HTTPMEGA_httpSvr.begin_noDHCP(HTTPMEGA_SS_PIN,
                               HTTPMEGA_CS_PIN,                               
                               HTTPMEGA_TCP_PORT);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(DBG_PIN, OUTPUT);
  pinMode(W5100_DBG_PIN0, OUTPUT);
  pinMode(W5100_DBG_PIN1, OUTPUT);
  digitalWrite(DBG_PIN, HIGH);
  digitalWrite(W5100_DBG_PIN0, HIGH);
  digitalWrite(W5100_DBG_PIN1, HIGH);
  
#ifdef HTTPSVR_USE_LCD
  // Initialize LCD
  configDisplay();
  writeBanner(HTTPMEGA_APPNAME, HTTPMEGA_APPVER);
#endif
  
  // Start Ethernet
  configWiz(HTTPMEGA_MAC_ADDRESS,  // Local MAC address
            HTTPMEGA_STATIC_IP,    // Local IP
            HTTPMEGA_SUBNET_MASK,  // Subnet mask
            HTTPMEGA_GATEWAY_IP);  // Gateway IP

  delay(2000);
  
  // Initialize HttpSvr
  digitalWrite(W5100_DBG_PIN0, LOW);
  digitalWrite(W5100_DBG_PIN1, LOW);
  HTTPMEGA_sNtpTime = getNtpTime();
#ifdef HTTPSVR_USE_LCD
  writeLine1("Connecting...");
#endif

  configHttpServer();
  
#ifdef HTTPSVR_USE_LCD
  writeIP(HTTPMEGA_httpSvr.localIpAddr());
#endif

  // Terminate configuration with a short delay
  delay(2000);
  
#ifdef HTTPSVR_USE_LCD
  clearDisplay();
#endif
}

////////////////////////////////////////////////////////////////////////////////

static uint8_t uTotConn = 0;

void loop()
{
  // Blink led
  digitalWrite(DBG_PIN, !digitalRead(DBG_PIN));
  delay(250);

  // Check if there is an incoming client request
  uTotConn += HTTPMEGA_httpSvr.serveHttpConnections();
  
#ifdef HTTPSVR_USE_LCD
  // Print some diagnostics
  writeFreeMem();  
  writeLine2(String("Conn:") + String(uTotConn));
#endif
}

////////////////////////////////////////////////////////////////////////////////

