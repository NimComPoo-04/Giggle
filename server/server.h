#ifndef _SERVER_H_
#define _SERVER_H_

#include <pthread.h>

typedef struct
{
	int fd;			// server file descriptor
	int listning;		// listing or not dude
	pthread_mutex_t mutex;	// (-_-) duh...
} server_t;

server_t server_create(int port, int backport);
void server_start(server_t *s);
void server_destroy(server_t *s);

int server_accept_connection(server_t *s);

#endif
