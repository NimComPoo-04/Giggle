#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "connection.h"
#include "http.h"

// TODO: DON'T USE THIS PLEASE
int stupid_hash_func(char *key);

void connection_handler(connection_t *c)
{
	char *req = http_request_read(c->fd, NULL);
	//printf(req);
	
	map_t m = http_request_parse(req);

	struct bkt_t *b = m.bkts[0];
	while(b)
	{
		printf("%s: %s\n", b->key, b->value);
		b = b->next;
	}

	map_destroy(&m);
	free(req);

	const char *body = "<h1>Hello, World!\n</h1>";

	map_t req_map = map_create(100, stupid_hash_func);
	map_add(&req_map, "Server", "Gigglelol");

	char *res = http_response_gen(200, &req_map , body);

	map_destroy(&req_map);

	send(c->fd, res, strlen(res), 0);

	printf("%s\n", res);

	free(res);

	shutdown(c->fd, SHUT_RDWR);
	close(c->fd);
}
