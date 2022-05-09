#ifndef _HTTP_H_
#define _HTTP_H_

#include "map.h"
#include <ctype.h>

// TODO: The names here break convention by a long shot.
// rename stuff for more respectablility :)

typedef struct
{
	char *Method;
	char *Protocol;
	char *URI;
	map_t headers;
	char *body;
} http_request_t;

typedef struct
{
	char *Protocol;
	char *Status;
	map_t headers;
	char *body;
} http_response_t;

char *http_request_read(int con_fd, size_t *length);
http_request_t http_request_parse(char *req);
map_t http_request_parse_body(http_request_t *h);

char *http_response_gen(http_response_t *h, size_t *len);

char *http_response_status_str(int status);

#endif
