#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "map.h"

#define BUFFSIZE (256)

/*
 * NOTE: recv is a blocking operatoin. if it don't recieve anything
 * it blocks the thing. so we exit as soon as we see that we have
 * recieved little less info than the buffer itself.
 */
char *http_request_read(int con_fd, int *zZzZ)
{
	int len = BUFFSIZE;
	char buffer[BUFFSIZE] = {0};

	char *header = NULL;
	int header_len = 0;

	while(len >= BUFFSIZE)
	{
		len = recv(con_fd, buffer, sizeof buffer, 0);
		if(len < 0)
		{
			printf("recv is doing something wierd\n");
			exit(1);
		}

		header = realloc(header, header_len + len + 1);
		//memset(header+header_len, 0, len);
		memcpy(header+header_len, buffer, len);
		header_len += len;
	}

	if(zZzZ != NULL)
		*zZzZ = header_len;

	return header;
}

static int stupid_hash_func(char *key)
{
	(void)key;
	return 0;
}

char *strcok(char *value, char *delim, char **save)
{
	if(value == NULL)
		value = *save;

	char *val = value;
	int dlen = strlen(delim);

	while(value++)
	{
		if(strncmp(delim, value, dlen) == 0)
		{
			memset(value, 0, dlen);
			value += dlen;
			break;
		}
	}

	*save = value;
	return val;
}

static char *strtrm(char *c)
{
	while(*c == ' ') c++;
	return c;
}

map_t http_request_parse(char *value)
{
	map_t m = map_create(100, stupid_hash_func);

	char *save = 0;

	map_add(&m, "__METHOD__", strcok(value, " ",    &save));
	map_add(&m, "__PATH__",   strcok(NULL,  " ",    &save));
	map_add(&m, "__PROTO__",  strcok(NULL,  "\r\n", &save));

	while(save[0] != '\r' || save[1] != '\n')
	{
		map_add(&m, strcok(NULL, ":", &save), strtrm(strcok(NULL, "\r\n", &save)));
	}

	map_add(&m, "__BODY__", save+2);

	return m;
}

static const char *status_code_gen(int status)
{
	if(status == 200)
		return "200 OK";
	return "404 Not Found";
}

char *http_response_gen(int status, map_t *m)
{
	(void)m;
	char *response = calloc(sizeof(char), 128);

	strcpy(response, "HTTP/1.0 ");
	strcat(response, status_code_gen(status));
	strcat(response, "\r\n\r\n");

	const char *hehboi = "<h1>Hello, World!<h1><p>No I am not crazy<br>Swear on god world say that on more time i will loose my shit!</p>";
	strcat(response, hehboi);

	return response;
}
