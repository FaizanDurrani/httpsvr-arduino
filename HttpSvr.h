///////////////////////////////////////////////////////////////////////////////
//
//  HttpSvr.h - Declaration of class HttpSvr
//  ECA 31Mar2013
//
//  ----------------------
//
//  HttpSvr is a small library for Arduino that implements a basic web server.
//  It uses Ethernet and SD libraries and provides a partial implementation of
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

#ifndef HTTPSVR_H
#define HTTPSVR_H

#include <Arduino.h>
#include <IPAddress.h>

#include "ClientProxy.h"
#include "utility/SdSvr.h"

///////////////////////////////////////////////////////////////////////////////
// Some useful definitions of strings

#define HttpSvr_SERVERNAME   "HttpSvr"
#define HttpSvr_VERSION      "0.0.1"
#define HttpSvr_HTTP_VERSION "HTTP/1.1"
#define HttpSvr_CRLF         "\r\n"
#define HttpSvr_SP           " "
#define HttpSvr_COLON        ":"
#define HttpSvr_SLASH        "/"

///////////////////////////////////////////////////////////////////////////////
// Definition of strings used in HTTP protocol

//Status-Codes (see RFC 2616 par. 6.1 and 6.1.1)
#define HttpSvr_SC_100   "100" // "Continue"                        
#define HttpSvr_SC_101   "101" // "Switching Protocols"             
#define HttpSvr_SC_200   "200" // "OK"                              
#define HttpSvr_SC_201   "201" // "Created"                         
#define HttpSvr_SC_202   "202" // "Accepted"                        
#define HttpSvr_SC_203   "203" // "Non-Authoritative Information"   
#define HttpSvr_SC_204   "204" // "No Content"                      
#define HttpSvr_SC_205   "205" // "Reset Content"                   
#define HttpSvr_SC_206   "206" // "Partial Content"                 
#define HttpSvr_SC_300   "300" // "Multiple Choices"                
#define HttpSvr_SC_301   "301" // "Moved Permanently"               
#define HttpSvr_SC_302   "302" // "Found"                           
#define HttpSvr_SC_303   "303" // "See Other"                       
#define HttpSvr_SC_304   "304" // "Not Modified"                    
#define HttpSvr_SC_305   "305" // "Use Proxy"                       
#define HttpSvr_SC_307   "307" // "Temporary Redirect"              
#define HttpSvr_SC_400   "400" // "Bad Request"                     
#define HttpSvr_SC_401   "401" // "Unauthorized"                    
#define HttpSvr_SC_402   "402" // "Payment Required"                
#define HttpSvr_SC_403   "403" // "Forbidden"                       
#define HttpSvr_SC_404   "404" // "Not Found"                       
#define HttpSvr_SC_405   "405" // "Method Not Allowed"              
#define HttpSvr_SC_406   "406" // "Not Acceptable"                  
#define HttpSvr_SC_407   "407" // "Proxy Authentication Required"   
#define HttpSvr_SC_408   "408" // "Rerquest Time-out"               
#define HttpSvr_SC_409   "409" // "Conflict"                        
#define HttpSvr_SC_410   "410" // "Gone"                            
#define HttpSvr_SC_411   "411" // "Length Required"                 
#define HttpSvr_SC_412   "412" // "Precondition Failed"             
#define HttpSvr_SC_413   "413" // "Request Entity Too Large"        
#define HttpSvr_SC_414   "414" // "Request-URI too Large"           
#define HttpSvr_SC_415   "415" // "Unsopprted Media Type"           
#define HttpSvr_SC_416   "416" // "Requested range not satisfiable" 
#define HttpSvr_SC_417   "417" // "Expectation Failed"              
#define HttpSvr_SC_500   "500" // "Internal Server Error"           
#define HttpSvr_SC_501   "501" // "Not Implemented"                 
#define HttpSvr_SC_502   "502" // "Bad Gateway"                     
#define HttpSvr_SC_503   "503" // "Service Unavailable"             
#define HttpSvr_SC_504   "504" // "Gateway Time-out"                
#define HttpSvr_SC_505   "505" // "HTTP Version not supported"      

// Reason-Phrases (see RFC 2616 par. 6.1 and 6.1.1)
#define HttpSvr_RP_100   "Continue"                        
#define HttpSvr_RP_101   "Switching Protocols"             
#define HttpSvr_RP_200   "OK"                              
#define HttpSvr_RP_201   "Created"                         
#define HttpSvr_RP_202   "Accepted"                        
#define HttpSvr_RP_203   "Non-Authoritative Information"   
#define HttpSvr_RP_204   "No Content"                      
#define HttpSvr_RP_205   "Reset Content"                   
#define HttpSvr_RP_206   "Partial Content"                 
#define HttpSvr_RP_300   "Multiple Choices"                
#define HttpSvr_RP_301   "Moved Permanently"               
#define HttpSvr_RP_302   "Found"                           
#define HttpSvr_RP_303   "See Other"                       
#define HttpSvr_RP_304   "Not Modified"                    
#define HttpSvr_RP_305   "Use Proxy"                       
#define HttpSvr_RP_307   "Temporary Redirect"              
#define HttpSvr_RP_400   "Bad Request"                     
#define HttpSvr_RP_401   "Unauthorized"                    
#define HttpSvr_RP_402   "Payment Required"                
#define HttpSvr_RP_403   "Forbidden"                       
#define HttpSvr_RP_404   "Not Found"                       
#define HttpSvr_RP_405   "Method Not Allowed"              
#define HttpSvr_RP_406   "Not Acceptable"                  
#define HttpSvr_RP_407   "Proxy Authentication Required"   
#define HttpSvr_RP_408   "Rerquest Time-out"               
#define HttpSvr_RP_409   "Conflict"                        
#define HttpSvr_RP_410   "Gone"                            
#define HttpSvr_RP_411   "Length Required"                 
#define HttpSvr_RP_412   "Precondition Failed"             
#define HttpSvr_RP_413   "Request Entity Too Large"        
#define HttpSvr_RP_414   "Request-URI too Large"           
#define HttpSvr_RP_415   "Unsopprted Media Type"           
#define HttpSvr_RP_416   "Requested range not satisfiable" 
#define HttpSvr_RP_417   "Expectation Failed"              
#define HttpSvr_RP_500   "Internal Server Error"           
#define HttpSvr_RP_501   "Not Implemented"                 
#define HttpSvr_RP_502   "Bad Gateway"                     
#define HttpSvr_RP_503   "Service Unavailable"             
#define HttpSvr_RP_504   "Gateway Time-out"                
#define HttpSvr_RP_505   "HTTP Version not supported"      

// Request Methods (see RFC 2616 par. 5.1.1)
#define HttpSvr_OPTIONS  "OPTIONS"
#define HttpSvr_GET      "GET"
#define HttpSvr_HEAD     "HEAD"
#define HttpSvr_POST     "POST"
#define HttpSvr_PUT      "PUT"
#define HttpSvr_DELETE   "DELETE"
#define HttpSvr_TRACE    "TRACE"
#define HttpSvr_CONNECT  "CONNECT"
  
// General headers (see RFC 2616 par. 4.5)
#define HttpSvr_cache_control       "Cache-Control"
#define HttpSvr_connection          "Connection"
#define HttpSvr_date                "Date"
#define HttpSvr_prama               "Pragma"
#define HttpSvr_trailer             "Trailer"
#define HttpSvr_transfer_encoding   "Transfer-Encoding"
#define HttpSvr_upgrade             "Upgrade"
#define HttpSvr_via                 "Via"
#define HttpSvr_warning             "Warning"

// Request headers (see RFC 2616 par. 5.3)
#define HttpSvr_accept              "Accept"
#define HttpSvr_accept_charset      "Accept-Charset"
#define HttpSvr_accept_encoding     "Accept-Encoding"
#define HttpSvr_accept_language     "Accept-Language"
#define HttpSvr_authorization       "Authorization"
#define HttpSvr_expect              "Expect"
#define HttpSvr_from                "From"
#define HttpSvr_host                "Host"
#define HttpSvr_if_match            "If-Match"
#define HttpSvr_if_modified_since   "If-Modified-Since"
#define HttpSvr_if_none_match       "If-None-Match"
#define HttpSvr_if_range            "If-Range"
#define HttpSvr_if_unmodified_since "If-Unmodified-Since"
#define HttpSvr_max_forwards        "Max-Forwards"
#define HttpSvr_proxy_authorization "Proxy-Authorization"
#define HttpSvr_range               "Range"
#define HttpSvr_referer             "Referer"
#define HttpSvr_te                  "TE"
#define HttpSvr_user_agent          "User-Agent"

// Response headers (see RFC 2616 par. 6.2)
#define HttpSvr_accept_ranges       "Accept-Ranges"
#define HttpSvr_age                 "Age"
#define HttpSvr_etag                "ETag"
#define HttpSvr_location            "Location"
#define HttpSvr_proxy_authenticate  "Proxy-Authenticate"
#define HttpSvr_retry_after         "Retry-After"
#define HttpSvr_server              "Server"
#define HttpSvr_vary                "Vary"
#define HttpSvr_www_authenticate    "WWW-Authenticate"

// Entity headers (see RFC 2616 par. 7.1)
#define HttpSvr_allow               "Allow"
#define HttpSvr_content_encoding    "Content-Encoding"
#define HttpSvr_content_language    "Content-Language"
#define HttpSvr_content_length      "Content-Length"
#define HttpSvr_content_location    "Content-Location"
#define HttpSvr_content_md5         "Content-MD5"
#define HttpSvr_content_range       "Content-Range"
#define HttpSvr_content_type        "Content-Type"
#define HttpSvr_expires             "Expires"
#define HttpSvr_last_modified       "Last-Modified"

///////////////////////////////////////////////////////////////////////////////
// Precompiled message headers

#define HttpSvr_header_server              HttpSvr_server HttpSvr_COLON HttpSvr_SP HttpSvr_SERVERNAME HttpSvr_SLASH HttpSvr_VERSION // "Server: HttpSvr/x.y.z"
#define HttpSvr_header_content_length      HttpSvr_content_length HttpSvr_COLON HttpSvr_SP // "Content-Length: "
#define HttpSvr_header_content_type_html   HttpSvr_content_type HttpSvr_COLON HttpSvr_SP "text/html"// "Content-Type: text/html"

///////////////////////////////////////////////////////////////////////////////
// The following struct contains all the enums used in the library.

struct http_e
{
  // Polling Type - The method used when listening to client connection.
  // It can be either:
  // * non-blocking, meaning that the poll function returns even if no client connection is available,
  // * blocking, meaning that the poll function does not return until a client connection is available,
  //   or a timeout expires. The timeout can also be infinite.
  enum poll_type
  { 
    poll_nonBlocking,         // Non-blocking poll
    poll_blocking             // Blocking poll (with timeout)
  };

  // Method - The HTTP request method (see RFC 2616 par. 5.1.1)
  // The method is the very first item of a HTTP message, i.e. the first token in the
  // first line, that is called the "Request Line". Examples of request lines are:
  //
  //  GET /favicon.ico HTTP/1.1    <== method is "GET"
  //   ...
  //
  //  POST /ex/fup.cgi HTTP/1.1    <== method is "POST"
  //   ...
  enum method   
  {
    mthd_undefined,
    
    // Request Method (see RFC 2616 par. 5.1.1)
    mthd_options,             // "OPTIONS"
    mthd_get,                 // "GET"
    mthd_head,                // "HEAD"
    mthd_post,                // "POST"
    mthd_put,                 // "PUT"
    mthd_delete,              // "DELETE"
    mthd_trace,               // "TRACE"
    mthd_connect              // "CONNECT"
  };
  
  // Header field names
  // Headers are the part of a HTTP message immediately following the request line.
  // Headers are a sequence of non-empty text lines, each terminated by a CRLF.
  // The header part in a HTTP message terminates with an empty line and is followed
  // by the message body.
  // Each header line has the form "field-name : field-value CRLF". Examples are:
  //
  //  Host: cgi-lib.berkeley.edu
  //  Connection: keep-alive
  //  Content-Length: 209435
  //  Cache-Control: max-age=0
  //  Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
  //  Origin: http://cgi-lib.berkeley.edu
  //  Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryU5xhPrJdAidAnjCU
  //  Referer: http://cgi-lib.berkeley.edu/ex/fup.html
  //
  // Headers are semantically grouped in four categories: general, request, response and entity.
  // All categories share the same form.
  enum msg_header  
  {
    hd_undefined,
    
    // General headers (see RFC 2616 par. 4.5)
    genhd_cache_control,      // "Cache-Control"
    genhd_connection,         // "Connection"
    genhd_date,               // "Date"
    genhd_pragma,             // "Pragma"
    genhd_trailer,            // "Trailer"
    genhd_transfer_encoding,  // "Transfer-Encoding"
    genhd_upgrade,            // "Upgrade"
    genhd_via,                // "Via"
    genhd_warning,            // "Warning"
    
    // Request headers (see RFC 2616 par. 5.3)
    reqhd_accept,             // "Accept"
    reqhd_accept_charset,     // "Accept-Charset"
    reqhd_accept_encoding,    // "Accept-Encoding"
    reqhd_accept_language,    // "Accept-Language"
    reqhd_authorization,      // "Authorization"
    reqhd_expect,             // "Expect"
    reqhd_from,               // "From"
    reqhd_host,               // "Host"
    reqhd_if_match,           // "If-Match"
    reqhd_if_modified_since,  // "If-Modified-Since"
    reqhd_if_none_match,      // "If-None-Match"
    reqhd_if_range,           // "If-Range"
    reqhd_if_unmodified_since,// "If-Unmodified-Since"
    reqhd_max_forwards,       // "Max-Forwards"
    reqhd_proxy_authorization,// "Proxy-Authorization"
    reqhd_range,              // "Range"
    reqhd_referer,            // "Referer"
    reqhd_te,                 // "TE"
    reqhd_user_agent,         // "User-Agent"
    
    // Response headers (see RFC 2616 par. 6.2)
    rsphd_accept_ranges,      // "Accept-Ranges"
    rsphd_age,                // "Age"
    rsphd_etag,               // "ETag"
    rsphd_location,           // "Location"
    rsphd_proxy_authenticate, // "Proxy-Authenticate"
    rsphd_retry_after,        // "Retry-After"
    rsphd_server,             // "Server"
    rsphd_vary,               // "Vary"
    rsphd_www_authenticate,   // "WWW-Authenticate"
    
    // Entity headers (see RFC 2616 par. 7.1)
    enthd_allow,              // "Allow"
    enthd_content_encoding,   // "Content-Encoding"
    enthd_content_language,   // "Content-Language"
    enthd_content_length,     // "Content-Length"
    enthd_content_location,   // "Content-Location"
    enthd_content_md5,        // "Content-MD5"
    enthd_content_range,      // "Content-Range"
    enthd_content_type,       // "Content-Type"
    enthd_expires,            // "Expires"
    enthd_last_modified       // "Last-Modified"
  };
};

///////////////////////////////////////////////////////////////////////////////
// The class implementing the HTTP server

class HttpSvr
{
public:
  // HttpSvr uses SD card and the SD card library. You can choose to either let HttpSvr
  // initialize the SD library and relevant object for you, or initialize it by yourself,
  // before using it in HttpSvr.

  // The constructor and destructor methods.
  HttpSvr();
  virtual ~HttpSvr();

  // This version of method begin initializes Ethernet but not SD card.
  // It should be used in case SD card is initialized somewhere else.
  // If "_wDHCP", DHCP is used to obtain the IP address and, if initialization fails,
  // the IP address defaults to the value provided in the relevant parameter;
  // If "_noDHCP", the IP address is set to the value provided in the relevant parameter;
  // Parameter "the_port" lets you specify which is the TCP port number used by HttpSvr
  // to listen for clients. Usually it is port 80.
  /* void begin_wDHCP (const uint8_t* the_macAddress, const IPAddress& the_ipAddress, uint16_t the_port); */
  void begin_noDHCP(const uint8_t* the_macAddress, const IPAddress& the_ipAddress, uint16_t the_port);
  
  // This version of method begin initializes SD card and Ethernet.
  // Here the SS and CS pin numbers are required for initialization of SD card library.
  // On the Ethernet Shield, CS is pin 4; SS pin is 10 on most Arduino boards, 53 on the Mega.
  // See relevant documentation for other boards.
  // See above for details on other parameters and the meaning of "_wDHCP" vs. "_noDHCP".
  /* void begin_wDHCP (int the_sdPinSS, int the_sdPinCS, const uint8_t* the_macAddress, const IPAddress& the_ipAddress, uint16_t the_port); */
  void begin_noDHCP(int the_sdPinSS, int the_sdPinCS, const uint8_t* the_macAddress, const IPAddress& the_ipAddress, uint16_t the_port);
  
  // The function to be called on exit.                   
  void terminate();

public:
  // Resource Binding
  // Resources are the basic object of a HTTP request: any HTTP request message is basically
  // aimed to obtain something in response - a resource.
  // Resources are identified by a Unified Resource Identifier (URI), that is a string
  // whose format is described in detail in RFC 3986. A special case of URI is the URL, the Unified
  // Resource Locator, that is a resource identifier that indicates a location where the resource
  // can be found. URLs are widely used in the web browser's address bar, and are those strings
  // usually starting with "http://www.something.org/..."
  // In HttpSvr, each URL can be bound to a callback function that will be in charge of providing
  // the corresponding resource. This gives means for dynamic building of the resource when required,
  // e.g. reporting the current status of sensors or other information.
  // Resources that are not bound to any callback are searched as static HTML pages in the SD card's
  // file system.
  typedef bool (*url_callback_t)(ClientProxy&, http_e::method, const char *);
  bool            bindUrl               (const char * the_url, url_callback_t the_callback);
  bool            isUrlBound            (const char * the_url);
  bool            resetUrlBinding       (const char * the_url);
  void            resetAllBindings      ();
  
public:
  // Client connection management
  // HttpSvr waits for connections from clients. It can wait in several ways:
  // - it can wait forever, until a client connects; this way is called "blocking".
  // - it can just look for a client connection at a given moment and immediately
  //   return in any case; if a client is trying to connect, it will be served, otherwise
  //   HttpSvr just reports that no client is connecting. This way is called "non-blocking".
  // - it can wait for a given time interval. If a connection occurs in this interval, HttpSvr
  //   function returns with a valid client; if the time elapses without any connection, HttpSvr
  //   function returns reporting no connection.
  // Each function returns an ClientProxy whose method "isConnected" evaluates to "true"
  // if a connection has been detected, otherwise it evaluates to "false", meaning that it can
  // be tested in a statement like:
  //    ClientProxy aClient = pollClient_nonBlk();
  //    if (aClient.isConnected())
  //      // do things with aClient - it is connected
  static const unsigned long msTimeout_infinite = static_cast<unsigned long>(-1);
  ClientProxy      pollClient            (http_e::poll_type the_bt = http_e::poll_nonBlocking) const;
  ClientProxy      pollClient_nonBlk     () const;
  ClientProxy      pollClient_blk        (unsigned long the_msTimeout) const;
  void             resetConnection       (ClientProxy&) const;

  // The following top-level function can be used for extra-simple http connection management.
  // It can be used directly in the loop function and implements typical management of HTTP clients
  uint8_t          serveHttpConnections ();
  
public:
  // Request serving
  // Functions "serveRequest_XXX" are high-level entry points for serving a client request.
  // These functions read the message start line and call the resource provider (callback function)
  // bound to the URI contained therein, if any. If no resource provider has been bound,
  // a resource corresponding to the request-URI is searched on the SD card.
  // If non is found, a 404 Not Found is sent in response.
  // The other functions in this group are used inside serveRequest, but are made publicly available
  // as building blocks for alternative management of requests.
  // In "_GET" variant, only GET and HEAD request methods are served (this is the most commonly used)
  // In "_POST" variant, only POST and HEAD request methods are served (useful for upload and AJAX operation, but memory hungry)
  // In "_GETPOST" variant, GET, POST and HEAD request methods are served (useful for upload and AJAX operation, but memory hungry)
  // If you know in advance what kind of requests you are going to serve, call only the most specific function and do not
  // call the others. Doing this, you will allow the compiler wipe out functions that are not called, thus
  // reducing the code size.
  bool            serveRequest_GET       (ClientProxy&, char *, uint16_t);
  bool            serveRequest_POST      (ClientProxy&, char *, uint16_t);
  bool            serveRequest_GETPOST   (ClientProxy&, char *, uint16_t);
  bool            readRequestLine        (ClientProxy&, http_e::method&, char *, uint16_t) const;
  bool            dispatchRequest_GET    (ClientProxy&, http_e::method, const char *);
  bool            dispatchRequest_POST   (ClientProxy&, http_e::method, const char *);
  bool            dispatchRequest_GETPOST(ClientProxy&, http_e::method, const char *);
  bool            readNextHeader         (ClientProxy&, char *, uint16_t , char *, uint16_t) const;
  bool            skipHeaders            (ClientProxy&) const;
  uint16_t        skipToBody             (ClientProxy&) const;
  bool            sendResFile            (ClientProxy&, const char *);

  // Request-URI parse utilities - typically for internal use only
  const char *    uriFindEndOfPath        (const char *) const;
  const char *    uriFindStartOfQuery     (const char *) const;
  const char *    uriExtractFirstQueryNVP (const char *, char *, uint16_t, char *, uint16_t) const;
  const char *    uriExtractNextQueryNVP  (const char *, char *, uint16_t, char *, uint16_t) const;
  const char *    uriFindStartOfFragment  (const char *) const;
  
public:
  // Response generation utilities
  // These functions may be used as shortcuts for response generation, either for
  // errors or for success. Other fuctions can be added for generating other kinds of responses
  bool            sendResponse                    (ClientProxy&, const char *) const;
  bool            sendResponseOk                  (ClientProxy&) const;
  bool            sendResponseOkWithContent       (ClientProxy&, uint32_t) const;
  bool            sendResponseBadRequest          (ClientProxy&) const;
  bool            sendResponseNotFound            (ClientProxy&) const;
  bool            sendResponseMethodNotAllowed    (ClientProxy&) const;
  bool            sendResponseInternalServerError (ClientProxy&) const;
  bool            sendResponseRequestUriTooLarge  (ClientProxy&) const;
  
public:
  // Message analysis

  // TODO : Provide some utility for analyzing/serving AJAX requests
    
public:
  // Connection and status information
  IPAddress       localIpAddr           () const;

private:
  void            prv_resetSocket       (W5100::socket_e the_sn, uint16_t the_port) const;
  bool            prv_readRequestLine   (ClientProxy& the_client, http_e::method& the_method, char * the_urlBuffer, uint16_t the_bufferLen) const;
  uint8_t         prv_boundResIdx       (ClientProxy&, const char *);
  bool            prv_dispatchGET       (ClientProxy&, const char *);
  bool            prv_dispatchPOST      (ClientProxy&, const char *);
  bool            prv_sendString        (ClientProxy& the_client, const char * the_str) const;

private:
  struct res_fn_pair
  { 
    res_fn_pair() : crc(0), fn(0) {}
    res_fn_pair(uint16_t the_crc, url_callback_t the_fn) : crc(the_crc), fn(the_fn) {}
    ~res_fn_pair() {}
    
    uint16_t       crc;
    url_callback_t fn;
  };  
  
  static const uint8_t smy_resMap_size = 16;
  res_fn_pair          my_resMap[smy_resMap_size];
  SdSvr                my_sdSvr;
};

///////////////////////////////////////////////////////////////////////////////

#endif // #ifndef HTTPSVR_H

