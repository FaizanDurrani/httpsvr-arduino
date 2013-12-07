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
#define HTTPMEGA_APPVER  "0.6.1"

////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

// The following #includes should not be due. However, they must be added here
#include <SD.h>
#include <SPI.h>
#include <HttpSvr.h>
#include <LiquidCrystal.h>
#include "LcdUtils.h"

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

void writeFreeMem()
{ 
  writeLine1(String("FreeBytes:") + String(memFree()));
}

////////////////////////////////////////////////////////////////////////////////
// IP printing utility
static void local_writeIPdigit(char * s, uint8_t b)
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

////////////////////////////////////////////////////////////////////////////////
// Definition of connection parameters - Change to your fit your configuration
static const uint8_t   HTTPMEGA_MAC_ADDRESS[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x42, 0xC6 };
static const IPAddress HTTPMEGA_STATIC_IP  (192, 168, 0, 27);
static const int       HTTPMEGA_TCP_PORT = 80;

static const int       HTTPMEGA_CS_PIN = 4;  // On the Ethernet Shield, CS is pin 4.
static const int       HTTPMEGA_SS_PIN = 53; // SS pin is 10 on most Arduino boards, 53 on the Mega

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
// Configuration of HTTP server object
void configHttpServer()
{
  // Bind resource providers
  HTTPMEGA_httpSvr.bindUrl("/"            , &rpRoot        );
  HTTPMEGA_httpSvr.bindUrl("/digitalRead" , &rpDigitalRead );
  HTTPMEGA_httpSvr.bindUrl("/digitalWrite", &rpDigitalWrite);

  // Start the server, specifying the SS and CS pins for SD card
  HTTPMEGA_httpSvr.begin_noDHCP(HTTPMEGA_SS_PIN,
                               HTTPMEGA_CS_PIN,
                               HTTPMEGA_MAC_ADDRESS,
                               HTTPMEGA_STATIC_IP,
                               HTTPMEGA_TCP_PORT);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static const uint8_t DBG_PIN = 9;

void setup()
{
  pinMode(DBG_PIN, OUTPUT);
  pinMode(W5100_DBG_PIN0, OUTPUT);
  pinMode(W5100_DBG_PIN1, OUTPUT);
  digitalWrite(DBG_PIN, HIGH);
  digitalWrite(W5100_DBG_PIN0, HIGH);
  digitalWrite(W5100_DBG_PIN1, HIGH);
  
  // Initialize LCD
  configDisplay();
  writeBanner(HTTPMEGA_APPNAME, HTTPMEGA_APPVER);
  delay(2000);
  
  // Initialize HttpSvr
  digitalWrite(W5100_DBG_PIN0, LOW);
  digitalWrite(W5100_DBG_PIN1, LOW);
  writeLine1("Connecting...");
  configHttpServer();
  writeIP(HTTPMEGA_httpSvr.localIpAddr());
  delay(2000);
  
  // Terminate configuration with a short delay
  clearDisplay();
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
  
  // Print some diagnostics
  writeFreeMem();  
  writeLine2(String("Conn:") + String(uTotConn));
}

////////////////////////////////////////////////////////////////////////////////

