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
#include "http_request.h"
#include "http_response.h"

#define RES404 "<h1>NOT FOUND</h1>"
void connection_handler(connection_t *c)
{
	char req[1024] = {0};

	recv(c->fd, req, sizeof req, 0);

	http_request_t h = http_request_parse(req);

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

	http_response_t config = {0};
	config.status = 200;

	struct stat file_info = {0}; 
	stat(res_path, &file_info);

	if(S_ISREG(file_info.st_mode))
		config.body = http_read_file(res_path);
	else
		config.body = RES404;

	char *res = http_response_gen(&config);

	send(c->fd, res, strlen(res), 0);

	if((const char *)config.body != (const char *)RES404)
		free(config.body);
	free(res);

	shutdown(c->fd, SHUT_RDWR);
	close(c->fd);
}
