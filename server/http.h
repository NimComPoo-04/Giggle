#ifndef _HTTP_H_
#define _HTTP_H_

#include "map.h"

/*
 * __METHOD__ : for http method
 * __PATH__ : for the requested document
 * __PROTO__: the version of protocol we are going with
 *
 * the rest should make sense lol
 */

char *http_request_read(int con_fd, int *len);
map_t http_request_parse(char *value);

char *http_response_gen(int status, map_t *field);

#endif
