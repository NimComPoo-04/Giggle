#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <ctype.h>

#include "map.h"

typedef struct
{
	int fd;
} connection_t;

void connection_handler(connection_t *);

#endif
