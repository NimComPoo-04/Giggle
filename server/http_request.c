#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_request.h"

/* 
 * TODO: this is strtok but the delim is treated as a single flat matcher
 * NOTE: this should be replaced with a regex parser.
 *
 * TODO: Implement a regex engine to deal with this instead of strcok
 */

// XXX: like the name ;)
static char *strcok(char *orig, char *delim, char **save)
{
	if(orig == NULL)
		orig = *save;

	char *stale_orig = orig;

	/* this is the length of the delimiter would be handy to send
	 * in the length from the caller but this is more convinient and
	 * the delimiter is going to be small anyways */

	int dlen = strlen(delim);

	while(orig[0] != 0)	
	{
		// FIXME: here is a buffer overflow bug. when the orig is at the end
		// the strncmp accesses memory that is out of bounds.
		if(strncmp(orig, delim, dlen) == 0)
		{
			memset(orig, 0, dlen);
			orig += dlen;
			break;
		}
		orig++;
	}

	*save = orig;
	return stale_orig;
}

// NOTE: this only trims the first part not the ending
static char *strtrm(char *x)
{
	while(*x == ' ') x++;
	return x;
}

http_request_t http_request_parse(char *value)
{
	http_request_t h = {0};
	char *cptr = 0;

	h.method = strcok(value, " ", &cptr);
	h.path = strcok(NULL, " ", &cptr);
	h.proto = strcok(NULL, "\r\n", &cptr);  

	while(strncmp(cptr, "\r\n", 2) != 0)
	{
		h.field_key[h.fields_current] = strcok(NULL, ":", &cptr);

		h.field_value[h.fields_current] = strcok(NULL, "\r\n", &cptr);
		h.field_value[h.fields_current] = strtrm(h.field_value[h.fields_current]);

		h.fields_current++;
	}

	return h;
}
