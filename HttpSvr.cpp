///////////////////////////////////////////////////////////////////////////////
//
//  HttpSvr.cpp - Definition of class HttpSvr
//
//  ----------------------
//
//  HttpSvr is a small library for Arduino implementing a basic web server.
//  It uses Ethernet and SD library and provides a partial implementation of
//  HTTP/1.1 according to RFC 2616
//
//  ----------------------
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of either the GNU General Public License version 2
//  or the GNU Lesser General Public License version 2.1, both as
//  published by the Free Software Foundation.
//
///////////////////////////////////////////////////////////////////////////////

#include "HttpSvr.h"
#include "ClientProxy.h"
#include "utility/SdSvr.h"
#include "utility/crc16.h"
#include "utility/W5100.h"

#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

static const uint16_t  local_maxUrlLength        = 128;
static const uint16_t  local_maxFieldNameLength  =  64;
static const uint16_t  local_maxFieldValueLength = 256;

///////////////////////////////////////////////////////////////////////////////

static uint32_t local_boundedStrLen(const char * the_s, uint32_t the_maxLength)
{
  if (!the_s) return 0;

  for (uint32_t u = 0; u < the_maxLength; ++u)
    if (the_s[u] == 0) return u;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool local_isBoundary(const char *the_buffer, uint16_t the_bufferLen, uint16_t the_boundaryCRC)
{
  if (!the_buffer) return false;
  if (the_bufferLen < 4) return false;

  // The boundary delimiter line always starts with "\r\n--" (see RFC 2046 par.5.1.1)
  if ((the_buffer[0] != '\r') || (the_buffer[1] != '\n') || (the_buffer[2] != '-') || (the_buffer[3] != '-'))
    return false;
  the_buffer += 4;
  the_bufferLen -= 4;

  // Discard the last two hyphens, if any, to handle the last boundary delimiter line
  if ((the_buffer[the_bufferLen-2] == '-') && (the_buffer[the_bufferLen-1] == '-'))
    the_bufferLen -= 2;
    
  // Compare the crc16 of the candidate line with the given boundary's crc
  return (the_boundaryCRC == crcsum(the_buffer, the_bufferLen, CRC_INIT));
}

///////////////////////////////////////////////////////////////////////////////

bool local_skipBeyondBoundary(ClientProxy& the_client, uint16_t the_boundaryCRC, char *the_buffer, uint16_t the_bufferLen)
{
  // This function consumes all chars up to the boundary delimiter,
  // then consumes the boundary delimiter itself
  if (!the_buffer) return false;
  if (!the_bufferLen) return false;
  
  while (true)
  {
    uint16_t uRead = the_client.readToEOL(the_buffer, the_bufferLen);
    if (uRead == 0) return false;
    if (local_isBoundary(the_buffer, uRead, the_boundaryCRC)) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

HttpSvr::HttpSvr()
: my_sdSvr()
{ resetAllBindings(); }

HttpSvr::~HttpSvr()
{ terminate(); }

///////////////////////////////////////////////////////////////////////////////

void HttpSvr::begin_noDHCP(const uint8_t* the_macAddress, const IPAddress& the_ipAddress, uint16_t the_port)
{ 
  // Connect using fixed IP
  W5100::begin(W5100::mac_address_t(the_macAddress),
               W5100::ipv4_address_t(the_ipAddress[0],the_ipAddress[1],the_ipAddress[2],the_ipAddress[3]));
  
  for (uint8_t sn = W5100::socket_begin; sn < W5100::socket_end; ++sn)
    prv_resetSocket(W5100::socket_cast(sn), the_port);
}

void HttpSvr::begin_noDHCP(int the_sdPinSS, int the_sdPinCS, const uint8_t* the_macAddress, const IPAddress& the_ipAddress, uint16_t the_port)
{ my_sdSvr.begin(the_sdPinSS, the_sdPinCS); begin_noDHCP(the_macAddress, the_ipAddress, the_port); }

void HttpSvr::terminate()
{ 
  resetAllBindings();
  my_sdSvr.terminate();
  W5100::terminate();
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::bindUrl(const char * the_url, url_callback_t the_callback)
{
  if (!the_url) return false;
  if (!the_callback) return false;

  // Compute CRC16 of requested url. CRC16 is used as a hash function
  // for URLs and the hash value is stored instead of the URL itself
  uint32_t len = local_boundedStrLen(the_url, local_maxUrlLength);
  if (len == 0) return false;
  uint16_t crc = crcsum(the_url, len, CRC_INIT);
  
  // Find a place where to store bind info
  uint8_t u;
  for (u = 0; u < smy_resMap_size; ++u)
    if (my_resMap[u].crc == 0) break;
  if (u >= smy_resMap_size) return false;
  
  // Store bind info
  my_resMap[u].crc = crc;
  my_resMap[u].fn  = the_callback;
  return true;
}

bool HttpSvr::isUrlBound(const char * the_url)
{
  if (!the_url) return false;

  uint32_t len = local_boundedStrLen(the_url, local_maxUrlLength);
  if (len == 0) return false;
  uint16_t crc = crcsum(the_url, len, CRC_INIT);
  
  uint8_t u;
  for (u = 0; u < smy_resMap_size; ++u)
    if (my_resMap[u].crc == crc) return true;
  return false;
}

bool HttpSvr::resetUrlBinding(const char * the_url)
{
  if (!the_url) return false;

  uint32_t len = local_boundedStrLen(the_url, local_maxUrlLength);
  if (len == 0) return false;
  uint16_t crc = crcsum(the_url, len, CRC_INIT);
  
  // Find bind info for this URL
  uint8_t u;
  for (u = 0; u < smy_resMap_size; ++u)
    if (my_resMap[u].crc == crc) break;
  if (u >= smy_resMap_size) return false;
  
  // Reset bind info
  my_resMap[u] = res_fn_pair();
  return true;
}

void HttpSvr::resetAllBindings()
{
  for (uint8_t u = 0; u < smy_resMap_size; ++u)
    my_resMap[u] = res_fn_pair();
}

///////////////////////////////////////////////////////////////////////////////

ClientProxy HttpSvr::pollClient(http_e::poll_type the_pollType) const
{
  switch (the_pollType)
  {
  case http_e::poll_nonBlocking : return pollClient_nonBlk();
  case http_e::poll_blocking    : return pollClient_blk   (msTimeout_infinite);
  default                       : return ClientProxy();
  }
}

ClientProxy HttpSvr::pollClient_nonBlk() const
{ 
  ClientProxy aClient;
  
  for (uint8_t sn = W5100::socket_begin; sn < W5100::socket_end; ++sn)
  {
    switch (W5100::checkClientConn(W5100::socket_cast(sn)))
    {
    case W5100::rc_ok:
      aClient.setConnection(W5100::socket_cast(sn));
      return aClient;
      
    default:
      break;
    }
  }
  
  return aClient;
}

ClientProxy HttpSvr::pollClient_blk(unsigned long the_msTimeout) const
{
  unsigned long ulStartTime = millis();
  for (;;)
  {
    ClientProxy aClient = pollClient_nonBlk();
    if (aClient.isConnected()) return aClient;
  
    if (the_msTimeout == msTimeout_infinite) continue;
    if (millis() - ulStartTime >= the_msTimeout) break;
  }
  
  return ClientProxy();
}

///////////////////////////////////////////////////////////////////////////////

void HttpSvr::resetConnection(ClientProxy& the_client) const
{ 
  W5100::socket_e sn = the_client.socket();
  uint16_t        port = the_client.localPort();
  the_client.closeConnection();
  prv_resetSocket(sn, port);
}

///////////////////////////////////////////////////////////////////////////////

static ClientProxy clients[W5100::socket_end];

uint8_t HttpSvr::serveHttpConnections()
{
  // This function can be called from inside "loop" to serve client connections

  uint8_t uNewConn = 0;
  
  digitalWrite(W5100_DBG_PIN1, LOW);
  for (uint8_t sn = W5100::socket_begin; sn < W5100::socket_end; ++sn)
  {
    if (!clients[sn].isConnected())
    {
      if (W5100::isClosed(W5100::socket_cast(sn)))
      {
        // If the socket is closed, we must recover it
        resetConnection(clients[sn]);
      }
      else
      {
        // If this client is not currently connected, check if there is an incoming connection
        if (W5100::checkClientConn(W5100::socket_cast(sn)) == W5100::rc_ok)
        {
          clients[sn].setConnection(W5100::socket_cast(sn));
          clients[sn].triggerConnTimeout();
          uNewConn++;
        }
      }
    }
    else
    {
      digitalWrite(W5100_DBG_PIN1, HIGH);
      
      // If this client is already connected, serve its requests if any
      if (clients[sn].anyDataReceived())
      {
        // Some data are pending from this client: let's refresh its connection timeout
        // and serve its request

        char urlBuffer[local_maxUrlLength];
        
        clients[sn].triggerConnTimeout();
        if (!serveRequest_GETPOST(clients[sn], urlBuffer, sizeof(urlBuffer)))
        {
          // Something went wrong during service: reset this client's connection
          resetConnection(clients[sn]);
        }
      }
      else
      {
        // No data since last inquiry. Let's check for connection timeout
        if (clients[sn].connTimeoutExpired())
          resetConnection(clients[sn]);
      }
    }
  }
  
  return uNewConn;
}  

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::serveRequest_GET(ClientProxy& the_client, char * the_urlBuffer, uint16_t the_bufferLen)
{ 
  // This function reads the message start line and calls the resource provider
  // bound to the URI contained therein, if any. If no provider has been bound,
  // a resource corresponding to the request-URI is searched on the SD card.
  // If non is found, a 404 Not Found is sent in response.
  
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) return false;
  if (!the_bufferLen) return false;

  http_e::method aMethod;
  if (!prv_readRequestLine(the_client, aMethod, the_urlBuffer, the_bufferLen)) return false;
  if (!dispatchRequest_GET(the_client, aMethod, the_urlBuffer)) return false;
  return true;
}

bool HttpSvr::serveRequest_POST(ClientProxy& the_client, char * the_urlBuffer, uint16_t the_bufferLen)
{ 
  // This function reads the message start line and calls the resource provider
  // bound to the URI contained therein, if any. If no provider has been bound,
  // a resource corresponding to the request-URI is searched on the SD card.
  // If non is found, a 404 Not Found is sent in response.
  
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) return false;
  if (!the_bufferLen) return false;

  http_e::method aMethod;
  if (!prv_readRequestLine(the_client, aMethod, the_urlBuffer, the_bufferLen)) return false;
  if (!dispatchRequest_POST(the_client, aMethod, the_urlBuffer)) return false;
  return true;
}

bool HttpSvr::serveRequest_GETPOST(ClientProxy& the_client, char * the_urlBuffer, uint16_t the_bufferLen)
{ 
  // This function reads the message start line and calls the resource provider
  // bound to the URI contained therein, if any. If no provider has been bound,
  // a resource corresponding to the request-URI is searched on the SD card.
  // If non is found, a 404 Not Found is sent in response.  
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) return false;
  if (!the_bufferLen) return false;

  http_e::method aMethod;
  if (!prv_readRequestLine(the_client, aMethod, the_urlBuffer, the_bufferLen)) return false;
  if (!dispatchRequest_GETPOST(the_client, aMethod, the_urlBuffer)) return false;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::readRequestLine(ClientProxy& the_client, http_e::method& the_method, char * the_urlBuffer, uint16_t the_bufferLen) const
{ 
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) return false;
  if (!the_bufferLen) return false;
  
  return prv_readRequestLine(the_client, the_method, the_urlBuffer, the_bufferLen);
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::dispatchRequest_GET(ClientProxy& the_client, http_e::method the_method, const char * the_urlBuffer)
{
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) { sendResponseInternalServerError(the_client); return false; }
 
  switch (the_method)
  {
  case http_e::mthd_get : return prv_dispatchGET(the_client, the_urlBuffer);
  case http_e::mthd_head: sendResponseOk(the_client); return true;
  default               : sendResponseBadRequest(the_client); return false;
  }
}

bool HttpSvr::dispatchRequest_POST(ClientProxy& the_client, http_e::method the_method, const char * the_urlBuffer)
{
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) { sendResponseInternalServerError(the_client); return false; }
 
  switch (the_method)
  {
  case http_e::mthd_head: sendResponseOk(the_client); return true;
  case http_e::mthd_post: return prv_dispatchPOST(the_client, the_urlBuffer);
  default               : sendResponseBadRequest(the_client); return false;
  }
}

bool HttpSvr::dispatchRequest_GETPOST(ClientProxy& the_client, http_e::method the_method, const char * the_urlBuffer)
{
  if (!the_client.isConnected()) return false;
  if (!the_urlBuffer) { sendResponseInternalServerError(the_client); return false; }
 
  switch (the_method)
  {
  case http_e::mthd_get : return prv_dispatchGET(the_client, the_urlBuffer);
  case http_e::mthd_head: sendResponseOk(the_client); return true;
  case http_e::mthd_post: return prv_dispatchPOST(the_client, the_urlBuffer);
  default               : sendResponseBadRequest(the_client); return false;
  }
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::readNextHeader(ClientProxy& the_client,
                             char *the_fieldName , uint16_t the_fieldNameLen,
                             char *the_fieldValue, uint16_t the_fieldValueLen) const
{
  // This function reads the next header in HTTP message, if any, and returns it
  // in the two buffers fieldName and fieldValue.
  // If either fieldName of fieldValue is null, or their lengths are zero,
  // the relevant header part is read but discarded, i.e. not stored in buffer.
  // If there are no more headers (i.e. an empty line, see RFC 2616 par. 4.1),
  // the function returns TRUE with both fieldName and fieldValue empty.
  // The function returns FALSE if an error occurs.
  if (!the_client.isConnected()) return false;

  if (the_fieldName  && the_fieldNameLen) the_fieldName [0] = 0;
  if (the_fieldValue && the_fieldValueLen) the_fieldValue[0] = 0;

  // According to the behavior of skipToNextLine, a CRLF pair is still to be consumed
  // when a new line is being read. This allows checking strongly the beginning of line
  if (!the_client.readCRLF()) return false;

  // Lines starting with SP or HT are to be considered as continuation of
  // previous header's field value (see RFC2616 par. 4.2)
  uint8_t ch;
  while (true)
  {
    if (!the_client.peekByte(ch)) return false;
    if ((ch != ' ') || (ch != '\t')) break;
    the_client.skipToNextLine();
    if (!the_client.readCRLF()) return false;
  }

  // Read field name
  uint16_t u = 0;
  while (u < the_fieldNameLen-1)
  {
    if (!the_client.readByte(ch)) return false;
    if (ch == '\r') { the_client.unreadByte(ch); break; }
    if (ch == ':') break;
    if (the_fieldName && the_fieldNameLen) the_fieldName[u++] = ch;
  }
  if (the_fieldName && the_fieldNameLen) the_fieldName[u] = 0;
  if ((ch == '\r') && (u != 0)) return false; // \r  is allowed only if the line is empty
  if ((ch != ':' ) && (u != 0)) return false; // ':' is the only delimiter allowed if the line is not empty

  // Read field value
  u = 0;
  the_client.skipAllLWS();
  while (u < the_fieldValueLen-1)
  {
    if (!the_client.readByte(ch)) return false;
    if (ch == '\r') { the_client.unreadByte(ch); break; }
    if (the_fieldValue && the_fieldValueLen) the_fieldValue[u++] = ch;
  }
  if (the_fieldValue && the_fieldValueLen) the_fieldValue[u] = 0;

  return true;
}

bool HttpSvr::skipHeaders(ClientProxy& the_client) const
{
  // This function skips all headers and goes to the header delimiter
  // (empty line or end of message).
  // It returns TRUE if the end of headers is successfully reached,
  // FALSE otherwise.
  char sFieldName[local_maxFieldNameLength];
  while (readNextHeader(the_client, sFieldName, sizeof(sFieldName), 0, 0))
    if (sFieldName[0] == 0) return true;
  return false;
}

uint16_t HttpSvr::skipToBody(ClientProxy& the_client) const
{
  // This function reads all headers and discards all, and goes to the
  // beginning of body, if any. It returns the value of "Content-Length",
  // if present, so as to allow reading the message body correctly.
  char sFieldName [local_maxFieldNameLength ];
  char sFieldValue[local_maxFieldValueLength];
  uint16_t uBodyLength = 0;
    
  // Consume all headers
  bool bOk;
  while ((bOk = readNextHeader(the_client, sFieldName, sizeof(sFieldName), sFieldValue, sizeof(sFieldValue))))
  {
    if (!sFieldName[0]) break;
    if (!strcasecmp(sFieldName, HttpSvr_content_length))
    {
      // A "Content-Length" header has been found, so we must remember its value
      // to skip the message body following headers
      uBodyLength = atoi(sFieldValue);
    }
  }
  
  // If everything is ok, we must also consume the empty line at the end
  // of headers (headers delimiter).
  if (bOk) the_client.readCRLF();
  return uBodyLength;
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::sendResFile(ClientProxy& the_client, const char * the_urlBuffer)
{
  if (!the_urlBuffer) { sendResponseInternalServerError(the_client); return false; }
  
  if (my_sdSvr.resFileExists(the_urlBuffer))
  {
    if (!my_sdSvr.openResFile(the_urlBuffer)) { sendResponseInternalServerError(the_client); return false; }

    // Send a response header with content length
    sendResponseOkWithContent(the_client, my_sdSvr.resFileSize());
    
    // Send resource as message body
    static const uint16_t uResBufferSize = 256;
    uint8_t resBuffer[uResBufferSize];
    uint16_t uRead = 0;
    while(uRead = my_sdSvr.readResFileBuffer(resBuffer, uResBufferSize))
    {
      if (the_client.writeBuffer(resBuffer, uRead) != uRead)
      {
        my_sdSvr.closeCurrentResFile();
        return false;
      }
    }
    my_sdSvr.closeCurrentResFile();
    return true;
  }    

  // If the page does not exist, send a 404 Not Found
  sendResponseNotFound(the_client);
  return false;
}


///////////////////////////////////////////////////////////////////////////////

const char * HttpSvr::uriFindEndOfPath(const char * the_URI) const
{
  if (!the_URI) return static_cast<char *>(0);
  // The path component terminates at end of string, or at start of fragment ('#'),
  // or at start of query ('?') (see RFC 3986 par. 3)
  while ((*the_URI != 0) && (*the_URI != '#') && (*the_URI != '?'))
    ++the_URI;
  return the_URI;
}

const char * HttpSvr::uriFindStartOfQuery(const char * the_URI) const
{
  // This function returns the pointer to the first character of the
  // URI's query component, if any, i.e. the first char after '?'.
  // If a query component is not found, 0 is returned.
  if (!the_URI) return static_cast<char *>(0);
  while (*the_URI != 0)
    if (*the_URI++ == '?') return the_URI;
  return 0;
}

const char * HttpSvr::uriExtractFirstQueryNVP(const char * the_URI, char * the_name, uint16_t the_nameLen, char * the_value, uint16_t the_valueLen) const
{ 
  // This function extracts the first Name-Value pair (NVP) from an URI's query, if any, and returns
  // a pointer to the first char after the end of the extracted NVP.
  // If a NVP is not found, 0 is returned.
  return uriExtractNextQueryNVP(uriFindStartOfQuery(the_URI), the_name, the_nameLen, the_value, the_valueLen);
}

const char * HttpSvr::uriExtractNextQueryNVP(const char * the_URI, char * the_name, uint16_t the_nameLen, char * the_value, uint16_t the_valueLen) const
{
  // This function extracts the next Name-Value pair (NVP) from the string passed, if any, and returns
  // a pointer to the first char after the end of the extracted NVP.
  // If a NVP is not found, 0 is returned.
  if (!the_URI || !the_name || !the_value || !the_nameLen || !the_valueLen)
    return static_cast<char *>(0);
  
  the_name[0] = 0;
  the_value[0] = 0;
  
  uint16_t u;

  // Read name
  for (u = 0; u < the_nameLen-1; ++u, ++the_URI)
  {
    if ((*the_URI == 0) || (*the_URI == '?')) { the_name[u] = 0; return static_cast<char *>(0); }
    if (*the_URI == '=') break;
    the_name[u] = *the_URI;
  }
  the_name[u] = 0;
  if (*the_URI != 0) ++the_URI;

  // Read value
  for (u = 0; u < the_valueLen-1; ++u, ++the_URI)
  {
    if ((*the_URI == 0) || (*the_URI == '?')) break;
    if (*the_URI == '&') break;
    the_value[u] = *the_URI;
  }
  the_value[u] = 0;
  if (*the_URI != 0) ++the_URI;

  return the_URI;
}

const char * HttpSvr::uriFindStartOfFragment(const char * the_URI) const
{
  // This function returns the pointer to the first character of the
  // URI's fragment component, if any, i.e. the first char after '#'.
  // If a fragment component is not found, 0 is returned.
  if (!the_URI) return static_cast<char *>(0);
  while (*the_URI != 0)
    if (*the_URI++ == '#') return the_URI;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::sendResponse(ClientProxy& the_client, const char * the_str) const
{ return prv_sendString(the_client, the_str); }

bool HttpSvr::sendResponseOk(ClientProxy& the_client) const
{ 
  // 200 OK
  static const char * msg = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_200 HttpSvr_SP HttpSvr_RP_200 HttpSvr_CRLF
                            HttpSvr_header_server HttpSvr_CRLF
                            HttpSvr_CRLF;
  return prv_sendString(the_client, msg); 
}

bool HttpSvr::sendResponseOkWithContent(ClientProxy& the_client, uint32_t the_size) const
{
  if (the_size == 0) return sendResponseOk(the_client);
  
  // 200 OK
  static const char * msg01 = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_200 HttpSvr_SP HttpSvr_RP_200 HttpSvr_CRLF;
  static const char * msg02 = HttpSvr_header_server HttpSvr_CRLF;
  static const char * msg03 = HttpSvr_header_content_type_html HttpSvr_CRLF;
  static const char * msg04 = HttpSvr_header_content_length;
  static const uint16_t msg04_len = strlen(msg04);
  
  // Send start line
  if (!prv_sendString(the_client, msg01)) return false;
  
  // Send headers: "Server: xxx"...
  if (!prv_sendString(the_client, msg02)) return false;

  // ..."Content-Type: text/html"
  if (!prv_sendString(the_client, msg03)) return false;

  // ..."Content-Length: xxx"
  char sContentLengthHeader[msg04_len + 16];
  strncpy(sContentLengthHeader, msg04, msg04_len);
  ltoa(the_size, &sContentLengthHeader[msg04_len], 10);
    strcat(sContentLengthHeader, HttpSvr_CRLF);
  if (!prv_sendString(the_client, sContentLengthHeader)) return false;
  
  // ...and an emtpy line (end of headers)
  return prv_sendString(the_client, HttpSvr_CRLF);
}

bool HttpSvr::sendResponseBadRequest(ClientProxy& the_client) const
{
  // 400 Bad Request
  static const char * msg = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_400 HttpSvr_SP HttpSvr_RP_400 HttpSvr_CRLF
                            HttpSvr_header_server HttpSvr_CRLF
                            HttpSvr_CRLF;
  return prv_sendString(the_client, msg); 
}

bool HttpSvr::sendResponseNotFound(ClientProxy& the_client) const
{
  // 404 Not Found
  static const char * msg = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_404 HttpSvr_SP HttpSvr_RP_404 HttpSvr_CRLF
                            HttpSvr_header_server HttpSvr_CRLF
                            HttpSvr_CRLF;
  return prv_sendString(the_client, msg); 
}

bool HttpSvr::sendResponseMethodNotAllowed(ClientProxy& the_client) const
{
  // 405 Method Not Allowed
  static const char * msg = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_405 HttpSvr_SP HttpSvr_RP_405 HttpSvr_CRLF
                            HttpSvr_header_server HttpSvr_CRLF
                            HttpSvr_CRLF;
  return prv_sendString(the_client, msg); 
}

bool HttpSvr::sendResponseRequestUriTooLarge(ClientProxy& the_client) const
{
  // 414 Request-URI Too Large
  static const char * msg = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_414 HttpSvr_SP HttpSvr_RP_414 HttpSvr_CRLF
                            HttpSvr_header_server HttpSvr_CRLF
                            HttpSvr_CRLF;
  return prv_sendString(the_client, msg); 
}

bool HttpSvr::sendResponseInternalServerError(ClientProxy& the_client) const
{
  // 500 Internal Server Error
  static const char * msg = HttpSvr_HTTP_VERSION HttpSvr_SP HttpSvr_SC_500 HttpSvr_SP HttpSvr_RP_500 HttpSvr_CRLF
                            HttpSvr_header_server HttpSvr_CRLF
                            HttpSvr_CRLF;
  return prv_sendString(the_client, msg); 
}

///////////////////////////////////////////////////////////////////////////////

IPAddress HttpSvr::localIpAddr() const
{
  W5100::ipv4_address_t ipAddr;
  return IPAddress(ipAddr.ip0(), ipAddr.ip1(), ipAddr.ip2(), ipAddr.ip3());
}

///////////////////////////////////////////////////////////////////////////////

void HttpSvr::prv_resetSocket(W5100::socket_e the_sn, uint16_t the_port) const
{
  if (the_sn == W5100::socket_undefined) return;
  
  // Modify default initialization of W5100 sockets to prevent buffer overflow
  // in receive operation.
  W5100::write_Sn_R8(the_sn, W5100_Sn_MR, W5100::read_Sn_R8(the_sn, W5100_Sn_MR) | W5100_ND);

  // Open socket and set it in listen mode
  W5100::open(the_sn, the_port);
  W5100::listen(the_sn);
}
  
///////////////////////////////////////////////////////////////////////////////

static http_e::method local_encodeMethod(const char *the_sMethod)
{
  // Deduce method by string match
  if (!strcmp(HttpSvr_OPTIONS, the_sMethod)) return http_e::mthd_options;
  if (!strcmp(HttpSvr_GET    , the_sMethod)) return http_e::mthd_get    ;
  if (!strcmp(HttpSvr_HEAD   , the_sMethod)) return http_e::mthd_head   ;
  if (!strcmp(HttpSvr_POST   , the_sMethod)) return http_e::mthd_post   ;
  if (!strcmp(HttpSvr_PUT    , the_sMethod)) return http_e::mthd_put    ;
  if (!strcmp(HttpSvr_DELETE , the_sMethod)) return http_e::mthd_delete ;
  if (!strcmp(HttpSvr_TRACE  , the_sMethod)) return http_e::mthd_trace  ;
  if (!strcmp(HttpSvr_CONNECT, the_sMethod)) return http_e::mthd_connect;

  return http_e::mthd_undefined;
}

bool HttpSvr::prv_readRequestLine(ClientProxy& the_client, http_e::method& the_method, char * the_urlBuffer, uint16_t the_bufferLen) const
{
  // This function reads Method and Request-URI from Request-Line
  // according to RFC 2616 par. 5.1 and returns TRUE if successful
  
  // Skip empty lines if any (see RFC 2616 par. 4.1)
  if (!the_client.skipAllCRLF()) return false;
  
  // Read and encode method
  char sMethod[8];
  if (!the_client.readToken(sMethod, sizeof(sMethod))) return false;
  the_method = local_encodeMethod(sMethod);

  // Read URL
  if (!the_client.skipAllLWS()) return false;
  if (the_client.readToken(the_urlBuffer, the_bufferLen) == 0) return false;

  // Everything seems ok so far. Skip the remaining part of line
  return the_client.skipToNextLine();
}

///////////////////////////////////////////////////////////////////////////////

uint8_t HttpSvr::prv_boundResIdx(ClientProxy& the_client, const char * the_urlBuffer)
{
  // Isolate the absolute path from entire URI
  // (see RFC 2616 par. 3.2.1 and 5.1.2, and RFC 3986 par. 3)
  uint32_t uLen = 0;
  for (uLen = 0; true; ++uLen)
    if ((the_urlBuffer[uLen] == 0) || (the_urlBuffer[uLen] == '?' || (the_urlBuffer[uLen] == '#')))
      break;
  if (uLen == 0) { sendResponseBadRequest(the_client); return static_cast<uint8_t>(-1); }
  uint16_t crc = crcsum(the_urlBuffer, uLen, CRC_INIT);
  
  // Find stored bind info
  uint8_t u;
  for (u = 0; u < smy_resMap_size; ++u)
  {
    if (my_resMap[u].crc == crc) break;
    if (my_resMap[u].crc == 0) break;
  }
  return u;
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::prv_dispatchGET(ClientProxy& the_client, const char * the_urlBuffer)
{
  // Consume headers and body
  uint16_t uBodyLength = skipToBody(the_client);
  while (uBodyLength-->0)
  {
    uint8_t ch;
    if (!the_client.readByte(ch))
    { sendResponseBadRequest(the_client); return false; }
  }

  // Find stored bind info
  uint8_t u = prv_boundResIdx(the_client, the_urlBuffer);

  // If there is no provider for this resource, try to find the requested
  // resource as a file in SD card
  if ((u >= smy_resMap_size) || (my_resMap[u].crc == 0))
    return sendResFile(the_client, the_urlBuffer);
  
  // If a provider has been found, call it
  if (my_resMap[u].fn) return (my_resMap[u].fn)(the_client, http_e::mthd_get, the_urlBuffer);
  
  sendResponseNotFound(the_client);
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool HttpSvr::prv_dispatchPOST(ClientProxy& the_client, const char * the_urlBuffer)
{
  // Find stored bind info
  uint8_t u = prv_boundResIdx(the_client, the_urlBuffer);

  // If there is no provider for this resource, assume that this is a file upload to SD
  if ((u >= smy_resMap_size) || (my_resMap[u].crc == 0))
  {
    char sFieldName [local_maxFieldNameLength ];
    char sFieldValue[local_maxFieldValueLength];
    
    // Find the "Content-Type" header and check if it is "form/multipart"
    bool bOk;
    while ((bOk = readNextHeader(the_client, sFieldName, sizeof(sFieldName), sFieldValue, sizeof(sFieldValue))))
    {
      if (!sFieldName[0]) break;
      if (!strcasecmp(sFieldName, HttpSvr_content_type)) break;
    }
    if (!bOk)           { sendResponseBadRequest(the_client); return false; }
    if (!sFieldName[0]) { sendResponseBadRequest(the_client); return false; }

    // Is content type a "multipart/form-data" ?
    static const char * sMultipart = "multipart/form-data";
    if (!strstr(sFieldValue, sMultipart)) { sendResponseBadRequest(the_client); return false; }

    // Extract boundary and compute its crc16 (hash code to store for compare instead of full boundary)
    static const char * sBoundaryName= "boundary=";
    char * sBoundaryStart = strstr(sFieldValue, sBoundaryName);
    if (!sBoundaryStart) { sendResponseBadRequest(the_client); return false; }
    sBoundaryStart += strlen(sBoundaryName);
    char *sBoundaryEnd;
    for (sBoundaryEnd = sBoundaryStart; *sBoundaryEnd; ++sBoundaryEnd)
      if ((*sBoundaryEnd == ' ') || (*sBoundaryEnd == '\r') || (*sBoundaryEnd == '\n')) break;
    *sBoundaryEnd = 0;
    uint16_t crcBoundary = crcsum(sBoundaryStart, sBoundaryEnd - sBoundaryStart, CRC_INIT);
    
    // Now skip any other header and goto message body
    if (!skipHeaders(the_client)) { sendResponseBadRequest(the_client); return false; }

    // Skip all body sub-parts except the one with header containing the filename
    char * sFilenameStart = 0;
    char * sFilenameEnd   = 0;
    while (true)
    {
      // Start reading body parts
      if (!local_skipBeyondBoundary(the_client, crcBoundary, sFieldValue, sizeof(sFieldValue)))
      { sendResponseBadRequest(the_client); return false; }

      // Read subpart headers and look for one like:
      // Content-Disposition: form-data; [...] filename:"originalFileName.ext"; [...] CRLF
      while ((bOk = readNextHeader(the_client, sFieldName, sizeof(sFieldName), sFieldValue, sizeof(sFieldValue))))
      {
        if (!sFieldName[0]) break;
        if (!strcasecmp(sFieldName, "Content-Disposition")) break;
      }
      if (!bOk) { sendResponseBadRequest(the_client); return false; } // Read failed
      if (!sFieldName[0]) continue; // Content-Disposition not found - skip this body sub-part
      
      // Look for filename
      if ((sFilenameStart = strstr(sFieldValue, "filename=")) != 0) break;
    }

    // Trim filename at trailing double quote
    if (!sFilenameStart) { sendResponseBadRequest(the_client); return false; }
    for (; *sFilenameStart; ++sFilenameStart)
      if (*sFilenameStart == '"') break;
    if (*sFilenameStart != '"') { sendResponseBadRequest(the_client); return false; }
    ++sFilenameStart;
    for (sFilenameEnd = sFilenameStart; *sFilenameEnd; ++sFilenameEnd)
      if (*sFilenameEnd == '"') break;
    *sFilenameEnd = 0;

    // Skip to end of headers for this sub-part (first empty line following headers)
    // (consume also the empty line - header delimiter)
    if (!skipHeaders(the_client)) { sendResponseBadRequest(the_client); return false; }
    if (!the_client.readCRLF()) { sendResponseBadRequest(the_client); return false; }

    // Open file for writing on SD card
    static char * sLocalName = "upload.txt";
    if (SD.exists(sLocalName)) SD.remove(sLocalName);
    File aFile = SD.open(sLocalName, FILE_WRITE);
    if (!aFile) { sendResponseInternalServerError(the_client); return false; }

    // Read body until the boundary delimiter or the end delimiter is found
    uint32_t uRead;
    uint32_t uTotRead = 0;
    while ((uRead = the_client.readToEOL(sFieldValue, sizeof(sFieldValue))) > 0)
    {
      if (local_isBoundary(sFieldValue, uRead, crcBoundary)) break;
      aFile.write((uint8_t*)(sFieldValue), uRead);
      uTotRead += uRead;
    }
    aFile.close();

    char sTotRead[16];
    ltoa(uTotRead, sTotRead, 10);
    sendResponseOkWithContent(the_client, strlen(sTotRead));
    sendResponse(the_client, sTotRead);
    return true;
  }
  
  // If a provider has been found, call it
  if (my_resMap[u].fn) return (my_resMap[u].fn)(the_client, http_e::mthd_post, the_urlBuffer);
  
  sendResponseNotFound(the_client);
  return false;
}

///////////////////////////////////////////////////////////////////////////////

static const uint8_t msg_size = 32;

bool HttpSvr::prv_sendString(ClientProxy& the_client, const char * the_str) const
{
  uint8_t msg[msg_size];

  if (!the_str || !the_str[0]) return false;
  
  uint32_t u = 0;
  while (*the_str)
  {
    msg[u++] = *the_str++;
    if (u >= msg_size) { the_client.writeBuffer(msg, msg_size); u = 0; }
  }
  if (u > 0) the_client.writeBuffer(msg, u);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

