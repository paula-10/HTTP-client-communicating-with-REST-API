#pragma once

#include <string>
enum DataType { HTML, FORM, JSON, FORM_DATA };
// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(const char *host, const char *url,
                          const char *query_params, char **headers,
                          char **header_names, int headers_count,
                          char **cookies, int cookies_count);

// computes and returns a POST request string (cookies can be NULL if not
// needed)
char *compute_post_request(const char *host, const char *url,
                           const char *content_type, char **headers,
                           char **header_names, int headers_count,
                           char **body_data, int body_data_fields_count,
                           char **cookies, int cookies_count);

char *compute_delete_request(const char *host, const char *url,
                             const char *query_params, char **headers,
                             char **header_names, int headers_count,
                             char **cookies, int cookies_count);
int getStatusCode(char *message);

std::string getSessionCookie(char *message);
