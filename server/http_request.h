#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

// FIXME: the specification says there can be 100 fields
#define FIELD_NUM (50)

typedef struct
{
	char *method;
	char *path;
	char *proto;

	//TODO: implement this as a hash map for performance
	//FIXME: this might muck up the stack on some machines
	char *field_key[FIELD_NUM];
	char *field_value[FIELD_NUM];
	int fields_current;
} http_request_t;

http_request_t http_request_parse(char *value);

#endif
