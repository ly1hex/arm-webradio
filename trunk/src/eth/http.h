#ifndef _HTTP_H_
#define _HTTP_H_


//----- DEFINES -----
#define HTTP_SERVERPORT                (80)

#define HTTP_HTML_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Content-Type: text/html\r\n" \
  "Content-Length:       \r\n" \
  "\r\n"

#define HTTP_XML_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Content-Type: text/xml\r\n" \
  "Content-Length:       \r\n" \
  "\r\n"

#define HTTP_CSS_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Content-Type: text/css\r\n" \
  "Content-Length:       \r\n" \
  "\r\n"

#define HTTP_ICON_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "Content-Type: image/x-icon\r\n" \
  "Content-Length:       \r\n" \
  "\r\n"

#define HTTP_200_HEADER "" \
  "HTTP/1.0 200 OK\r\n" \
  "Server: "APPNAME"\r\n" \
  "\r\n"

#define HTTP_400_HEADER "" \
  "HTTP/1.0 400 Bad request\r\n" \
  "Server: "APPNAME"\r\n" \
  "\r\n"

#define HTTP_404_HEADER "" \
  "HTTP/1.0 404 Not found\r\n" \
  "Server: "APPNAME"\r\n" \
  "\r\n"


//----- PROTOTYPES -----
unsigned int                           http_tcpapp(unsigned int index, const unsigned char *rx, unsigned int rx_len, unsigned char *tx);


#endif //_HTTP_H_
