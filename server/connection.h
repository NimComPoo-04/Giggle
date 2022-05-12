#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <ctype.h>

#include "map.h"

typedef struct
{
	int fd;
	map_t *mime_types;
} connection_t;

void connection_handler(connection_t *);

#endif
