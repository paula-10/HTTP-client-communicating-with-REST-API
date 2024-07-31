#include "requests.hpp"

#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include <iostream>
#include <string>
#include <sstream>

#include "helpers.hpp"
char *compute_get_request(const char *host, const char *url,
                          const char *query_params, char **headers,
                          char **header_names, int headers_count,
                          char **cookies, int cookies_count) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL, request params (if any) and protocol
  // type
  if (query_params != NULL) {
    sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
  } else {
    sprintf(line, "GET %s HTTP/1.1", url);
  }

  compute_message(message, line);

  // Step 2: add the host
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  // Step 3 (optional): add headers and/or cookies, according to the protocol
  // forma
  if (headers != NULL) {
    for (int i = 0; i < headers_count; i++) {
      sprintf(line, "%s: %s", header_names[i], headers[i]);
      compute_message(message, line);
    }
  }

  if (cookies != NULL) {
    strcpy(line, "Cookie: ");
    for (int i = 0; i < cookies_count; i++) {
      strcat(line, cookies[i]);
      if (i != cookies_count - 1) strcat(line, "; ");
    }
    compute_message(message, line);
  }
  // Step 4: add final new line
  compute_message(message, "");

  free(line);
  return message;
}

char *compute_post_request(const char *host, const char *url,
                           const char *content_type, char **headers,
                           char **header_names, int headers_count,
                           char **body_data, int body_data_fields_count,
                           char **cookies, int cookies_count) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));
  char *body_data_buffer = (char *)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL and protocol type
  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  // Step 2: add the host
  sprintf(line, "Host: %s", host);
  compute_message(message, line);
  /* Step 3: add necessary headers (Content-Type and Content-Length are
     mandatory) in order to write Content-Length you must first compute the
     message size
  */
  sprintf(line, "Content-Type: %s", content_type);
  compute_message(message, line);
  int len = body_data_fields_count - 1;
  for (int i = 0; i < body_data_fields_count; i++) {
    len += strlen(body_data[i]);
  }
  sprintf(line, "Content-Length: %d", len);
  compute_message(message, line);

  // Add headers
  if (headers != NULL) {
    for (int i = 0; i < headers_count; i++) {
      sprintf(line, "%s: %s", header_names[i], headers[i]);
      compute_message(message, line);
    }
  }
  // Step 4 (optional): add cookies
  if (cookies != NULL) {
    strcpy(line, "Cookie: ");
    for (int i = 0; i < cookies_count; i++) {
      strcat(line, cookies[i]);
      if (i != cookies_count - 1) strcat(line, "; ");
    }
    compute_message(message, line);
  }
  // Step 5: add new line at end of header
  compute_message(message, "");

  // Step 6: add the actual payload data
  memset(line, 0, LINELEN);
  for (int i = 0; i < body_data_fields_count; i++) {
    strcat(body_data_buffer, body_data[i]);
    if (i != body_data_fields_count - 1) strcat(body_data_buffer, "&");
  }
  compute_message(message, body_data_buffer);

  free(line);
  free(body_data_buffer);
  return message;
}

char *compute_delete_request(const char *host, const char *url,
                             const char *query_params, char **headers,
                             char **header_names, int headers_count,
                             char **cookies, int cookies_count) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL, request params (if any) and protocol
  // type
  if (query_params != NULL) {
    sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
  } else {
    sprintf(line, "DELETE %s HTTP/1.1", url);
  }

  compute_message(message, line);

  // Step 2: add the host
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  // Step 3 (optional): add headers and/or cookies, according to the protocol
  // forma
  if (headers != NULL) {
    for (int i = 0; i < headers_count; i++) {
      sprintf(line, "%s: %s", header_names[i], headers[i]);
      compute_message(message, line);
    }
  }

  if (cookies != NULL) {
    strcpy(line, "Cookie: ");
    for (int i = 0; i < cookies_count; i++) {
      strcat(line, cookies[i]);
      if (i != cookies_count - 1) strcat(line, "; ");
    }
    compute_message(message, line);
  }
  // Step 4: add final new line
  compute_message(message, "");

  free(line);
  return message;
}

int getStatusCode(char *message) {
  
  std::stringstream ss(message);
  std::string httpVersion, statusCode;
  ss >> httpVersion >> statusCode;
  return std::stoi(statusCode);
}

std::string getSessionCookie(char *message) {
  
  std::string str(message);
  size_t startPos = str.find("Set-Cookie: ");
  if (startPos == std::string::npos)
    return "";
  startPos += strlen("Set-Cookie: ");
  size_t endPos = str.find(';', startPos);
  return str.substr(startPos, endPos - startPos);
}