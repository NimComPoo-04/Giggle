#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_response.h"

/*
 * NOTE: for the perposes of prototyping HTTP/1.0 is used instead of the current
 * standard HTTP/1.1 asto not deal with multiple http requests over a single tcp connection.
 */

// TODO: we need a string manupulator instead of this hack here
static const char *status_phrase(int status)
{
	switch(status)
	{
		case 200: return "HTTP/1.0 200 OK\r\n\r\n";
		case 404: return "HTTP/1.0 404 Not Found\r\n\r\n";
		case 500: return "HTTP/1.0 500 Internal Server Error\r\n\r\n";
	}
	return "HTTP/1.0 501 Not Implemented\r\n\r\n";
}

/*
 * TODO: make the string not be allocated in the heap
 * that makes this dude slow and i don't like that
 */
char *http_response_gen(http_response_t *h)
{
	const char *header = status_phrase(h->status);
	const char *body = h->body; 

	int header_len = strlen(header);
	int body_len = strlen(body);

	char *response = malloc(header_len + body_len + 1);
	memcpy(response, header, header_len);
	strcat(response, body);

	return response;
}

/*
 * TODO: this is slower than my grandma plz help all the heap allocations don't help
 */
char *http_read_file(char *path)
{
	FILE *f = fopen(path, "r+");

	if(f == NULL) return NULL;

	fseek(f, 0, SEEK_END);
	int file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *contents = malloc(file_size + 1);

	fread(contents, file_size, 1, f);

	fclose(f);

	return contents;
}
