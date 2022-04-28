#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "connection.h"
#include "http_request.h"

void connection_handler(connection_t *c)
{
	char req[1024] = {0};

	recv(c->fd, req, sizeof req, 0);

	http_request_t h = http_request_parse(req);

	//printf("%s\n", req);

	printf("METHOD: %s\n", h.method);
	printf("PATH: %s\n", h.path);
	printf("PROTO: %s\n", h.proto);

	for(int i = 0; i < h.fields_current; i++)
	{
		printf("( %s : %s )\n", h.field_key[i], h.field_value[i]);
	}

	char res_path[512] = {0};

	strcpy(res_path, "public");
	strcat(res_path, h.path);

	char res[1024] = "HTTP/1.0 404 Not Found\r\nServer: Giggle\r\n\r\n";

	strcat(res, "<h1>REQUEST: ");
	strcat(res, res_path);
	strcat(res, "</h1>");

	send(c->fd, res, strlen(res), 0);

	shutdown(c->fd, SHUT_RDWR);
	close(c->fd);
}
