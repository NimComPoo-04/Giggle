#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server.h"
#include "connection.h"
#include "tpool.h"

#include <assert.h>

/*
 * TODO; The error handling here is terrible 
 * Please do something about this for the love of everything that is good
 */

server_t server_create(int port, int backlog)
{
	server_t s = {0};

	int status = 0;
	int on = 1;

	status = (s.fd = socket(AF_INET, SOCK_STREAM, 0));
	if(status < 0)
	{
		printf("Could not create socket : %d\n", errno);
		exit(1);
	}

	// setting socket to be reusable because we will need to rebind 
	status = setsockopt(s.fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof on);
	if(status < 0)
	{
		printf("Could not set socket to reusable : %d\n", errno);
		exit(1);
	}

	struct sockaddr_in sa = {.sin_port = htons(port),
				 .sin_family = AF_INET,
				 .sin_addr.s_addr = INADDR_ANY};

	status = bind(s.fd, (const struct sockaddr *)&sa, sizeof sa);
	if(status < 0)
	{
		printf("Could not bind to port %d : %d\n", port, errno);
		exit(1);
	}

	status = listen(s.fd, backlog);
	if(status < 0)
	{
		printf("Could not listen on port %d with backlog %d\n", port, backlog);
		exit(1);
	}

	s.listning = 1;

	s.scheduler = tpool_create(1);

	s.mime_types = map_create(10, map_default_hash);

	map_add(&s.mime_types, ".html", "text/html; charset=utf-8");
	map_add(&s.mime_types, ".png", "image/png");
	map_add(&s.mime_types, ".jpeg", "image/jpeg");

	return s;
}

// NOTE: this lauches threads that handle connections
void server_start(server_t *s)
{
	connection_t con = {0}; 
	con.mime_types = &s->mime_types;

	// TODO: make this multi threaded
	while(s->listning)
	{
		con.fd= server_accept_connection(s);
		tpool_exec(s->scheduler, connection_handler, con);
	}
}

void server_destroy(server_t *s)
{
	map_destroy(&s->mime_types);
	tpool_destroy(s->scheduler);
	shutdown(s->fd, SHUT_RDWR);
	close(s->fd);
}

// NOTE: use this thing to accept connections
int server_accept_connection(server_t *s)
{
	int confd = 0;
	struct sockaddr_in sa = {0};
	socklen_t l = 0;

	confd = accept(s->fd, (struct sockaddr *)&sa, &l);

	if(confd < 0)
	{
		printf("Could not accpet connection : %d\n", errno);
		printf("%s\n", strerror(errno));
		exit(1);
	}

	return confd;
}
