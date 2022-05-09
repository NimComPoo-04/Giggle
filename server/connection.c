#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *load_file(char *path, size_t *len, int *tf);

void connection_handler(connection_t *c)
{
	char lennum[32] = {0}; // this is a little helper lol

	size_t raw_request_len = 0;
	char *raw_request = http_request_read(c->fd, &raw_request_len);
	http_request_t request = http_request_parse(raw_request);

	http_response_t response = {0};

	response.Protocol = "HTTP/1.0";
	response.headers = map_create(10, map_default_hash);
	map_add(&response.headers, "Server", "Giggle");


	// TODO: don't let this be hard coded for the love of god
	char *public_dir = "public";

	char path[64] = {0};
	strcpy(path, public_dir);
	strcat(path, request.URI);
	char *extention = strchr(path, '.');

	if(strcmp(request.Method, "GET") == 0)
	{
		if(extention == NULL)
		{
			// TODO: action routes here

			int status = 404;
			size_t len = 18;
			char *data = strdup("<h1>Not Found</h1>");

			sprintf(lennum, "%ld", len);
			response.Status = http_response_status_str(status);
			map_add(&response.headers, "Content-Length", lennum);
			map_add(&response.headers, "Content-Type", "text/html");

			response.body = data;
		}
		else if(strcmp(extention, ".lua") == 0)
		{
			// TODO: integrate lua

			int status = 404;
			size_t len = 18;
			char *data = strdup("<h1>Not Found</h1>");

			sprintf(lennum, "%ld", len);
			response.Status = http_response_status_str(status);
			map_add(&response.headers, "Content-Length", lennum);
			map_add(&response.headers, "Content-Type", "text/html");

			response.body = data;
		}
		else
		{
			int status = 0;
			size_t len = 0;
			char *data = NULL;

			data = load_file(path, &len, &status);

			sprintf(lennum, "%ld", len);

			response.Status = http_response_status_str(status);
			map_add(&response.headers, "Content-Length", lennum);
			map_add(&response.headers, "Content-Type", "text/html");

			response.body = data;
		}
	}
	else
	{
			int status = 501;
			size_t len = 27;
			char *data = strdup("<h1>Unsupported Method</h1>");

			sprintf(lennum, "%ld", len);
			response.Status = http_response_status_str(status);
			map_add(&response.headers, "Content-Length", lennum);
			map_add(&response.headers, "Content-Type", "text/html");

			response.body = data;
	}

	size_t raw_response_len = 0;
	char *raw_response = http_response_gen(&response, &raw_response_len);

	send(c->fd, raw_response, raw_response_len, 0);

	// freeing all the memory so that the OS remains happy lol
	map_destroy(&request.headers);
	map_destroy(&response.headers);
	free(response.body);
	free(raw_response);
	free(raw_request);

	shutdown(c->fd, SHUT_RDWR);
	close(c->fd);
}

// NOTE: loads file from paths and also handles the err code
// TODO: make it protable
static char *load_file(char *path, size_t *len, int *tf)
{
	struct stat file_stat = {0};
	int status = stat(path, &file_stat);

	// if the file is not found we do 404
	if(status < 0)
	{
		if(errno == ENOENT)
		{
			*tf = 404;
			*len = 18;
			return strdup("<h1>Not Found</h1>");
		}
		else exit(69);
	}

	// if the file is found but its not regular then 403
	if((file_stat.st_mode & S_IFMT) != S_IFREG)
	{
		*tf = 403;
		*len = 25;
		return strdup("<h1>Forbidden Access</h1>");
	}

	char *file = malloc((file_stat.st_size+1) * sizeof(char));

	FILE *fd = fopen(path, "r");
	fread(file, sizeof(char), file_stat.st_size, fd);
	fclose(fd);

	file[file_stat.st_size] = 0;

	*tf = 200;
	*len = file_stat.st_size;
	return file;
}
