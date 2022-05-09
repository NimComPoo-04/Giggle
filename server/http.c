#include <sys/socket.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

#define BUFFSIZE (256)

// FIXME: this read will fail if a bissare coincedence happens where the size of
// the request is exactly 256 bytes long no more no less
char *http_request_read(int con_fd, size_t *length)
{
	char buffer[BUFFSIZE] = {0};
	int len = BUFFSIZE;

	char *value = 0;
	FILE *stream = open_memstream(&value, length);

	while(len == BUFFSIZE)
	{
		len = recv(con_fd, buffer, sizeof buffer, 0);
		if(len < 0)
		{
			fprintf(stderr, "recv doing weird stuff: %d: %s\n", errno, strerror(errno));
			exit(1);
		}
		fwrite(buffer, sizeof(char), len, stream);
	}
	fclose(stream);

	return value;
}

// NOTE: this comparisn function does case insensetive comparisn
// TODO: this slower than my grandma plz make it faster
static int strnzcmp(char *c, char *d, size_t l)
{
	for(size_t i = 0; i < l; i++)
		if(c[i] != d[i])
			return 1;
	return 0;
}

// NOTE: does what strtok does except the delim is a flat pattern?.
static char *strcok(char *orig, char *delim, char **save)
{
	if(orig == NULL) orig = *save;
	char *dorig = orig;

	int dlen = strlen(delim);

	while(*orig != 0)
	{
		if(strnzcmp(orig, delim, dlen) == 0)
		{
			memset(orig, 0, dlen);
			orig += dlen;
			break;
		}
		orig++;
	}
	*save = orig;
	return dorig;
}

// NOTE: removes spaces from the top of stuff lol
static char *strtrm(char *orig)
{
	while(*orig == ' ') orig++;
	return orig;
}

// NOTE: parses the request into a map
// FIXME: crashes when the map does weird stuff.
http_request_t http_request_parse(char *req)
{
	http_request_t hrt = {0};

	char *sav = 0;

	hrt.Method = strcok(req, " ", &sav);
	hrt.URI = strcok(NULL, " ", &sav);
	hrt.Protocol = strcok(NULL, "\r\n", &sav);
	hrt.headers = map_create(10, map_default_hash);

	while(strncmp(sav, "\r\n", 2))
	{
		char *field_key = strtrm(strcok(NULL, ":", &sav));
		char *field_value = strtrm(strcok(NULL, "\r\n", &sav));

		map_add(&hrt.headers, field_key, field_value);
	}

	hrt.body = sav+2;
	hrt.body_fields = http_request_parse_body(&hrt);

	return hrt;
}

// TODO: get a better name for a url parser.
map_t http_request_parse_body(http_request_t *h)
{
	map_t vars = map_create(10, map_default_hash);
	char *sav = h->body;

	while(*sav != 0)
	{
		char *k1 = strcok(NULL, "=", &sav);
		char *k2 = strcok(NULL, "&", &sav);

		map_add(&vars, k1, k2);
	}

	return vars;
}

// TODO: make this dude portable. >:E
// windows went to get milk and never returned
char *http_response_gen(http_response_t *h, size_t *len)
{
	char *value = 0;
	FILE *stream = open_memstream(&value, len);

	fprintf(stream, "%s %s\r\n", h->Protocol, h->Status);
	
	for(int i = 0; i < h->headers.length; i++)
	{
		struct bkt_t *b = h->headers.bkts[i];
		while(b)
		{
			fprintf(stream, "%s: %s\r\n", b->key, b->value);
			b = b->next;
		}
	}

	fprintf(stream, "\r\n%s", h->body);

	fclose(stream);

	return value;
}

// TODO: implement atleast the ones in http/1.0 lol
char *http_response_status_str(int status)
{
	switch(status)
	{
		case 200: return "200 Success";
		case 403: return "403 Forbidden Access";
		case 404: return "404 Not Found";
		case 501: return "501 Unsupported Method";
	}

	return "500 Internal Server Error";
}
