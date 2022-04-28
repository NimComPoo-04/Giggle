#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

typedef struct
{
	int status;
	char *body;
} http_response_t;

char *http_response_gen(http_response_t *h);
char *http_read_file(char *path);

#endif
