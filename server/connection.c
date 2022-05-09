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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "connection.h"
#include "http.h"

static char *load_file(char *path, size_t *len, int *tf);
static char *exec_luascript(char *path, size_t *len, int *tf, http_request_t *request);

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
		int status = 0;
		size_t len = 0;
		char *data = 0;

		data = exec_luascript(path, &len, &status, &request);

		sprintf(lennum, "%ld", len);
		response.Status = http_response_status_str(status);
		map_add(&response.headers, "Content-Length", lennum);
		map_add(&response.headers, "Content-Type", "text/html");

		response.body = data;
	}
	else
	{
		if(strcmp(request.Method, "GET") == 0)
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
	}

	size_t raw_response_len = 0;
	char *raw_response = http_response_gen(&response, &raw_response_len);

	send(c->fd, raw_response, raw_response_len, 0);

	// freeing all the memory so that the OS remains happy lol
	map_destroy(&request.headers);
	map_destroy(&request.body_fields);
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

static int internal_luafunc_http_print(lua_State *l);

// TODO: this is atrociously slow lol
// improve it... IMPROVE IT!!!
static char *exec_luascript(char *path, size_t *len, int *tf, http_request_t *request)
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

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// NOTE : the next part is like writing assembly so
	// commenting each line is a must lol

	// registering __HTTP_REQUEST__ table
	lua_newtable(L);	// create's table

	lua_pushstring(L, "Method");
	lua_pushstring(L, request->Method);
	lua_settable(L, -3);

	lua_pushstring(L, "URI");
	lua_pushstring(L, request->URI);
	lua_settable(L, -3);

	lua_pushstring(L, "Protocol");
	lua_pushstring(L, request->Protocol);
	lua_settable(L, -3);

	for(int i = 0; i < request->headers.length; i++)
	{
		struct bkt_t *b = request->headers.bkts[i];
		while(b)
		{
			lua_pushstring(L, b->key);	// pushes key onto stack
			lua_pushstring(L, b->value);	// pushes value onto stack
			lua_settable(L, -3);		// sets table[key] to value
			b = b->next;
		}
	}

	lua_pushstring(L, "Body");	// name of the table lol
	lua_newtable(L);		// create's table
	for(int i = 0; i < request->body_fields.length; i++)
	{
		struct bkt_t *b = request->body_fields.bkts[i];
		while(b)
		{
			lua_pushstring(L, b->key);	// pushes key onto stack
			lua_pushstring(L, b->value);	// pushes value onto stack
			lua_settable(L, -3);		// sets table[key] to value
			b = b->next;
		}
	}
	lua_settable(L, -3);		// sets table[key] to value

	lua_setglobal(L, "__HTTP_REQUEST__");	// sets it a global name

	lua_pushstring(L, "");
	lua_setglobal(L, "__HTTP_RESPONSE__");	// sets it a global name

	lua_register(L, "http_print", internal_luafunc_http_print);

	if(luaL_dofile(L, path) != LUA_OK)
	{
		if(lua_isstring(L, -1))
			printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	// just the body of the response
	// the rest would be generated in the server
	
	char *body = NULL;

	lua_getglobal(L, "__HTTP_RESPONSE__");
	if(lua_isstring(L, -1))
	{
		body = strdup(lua_tolstring(L, -1, len)); // cool
	}

	lua_close(L);

	*tf = 200;
	return body;
}

static int internal_luafunc_http_print(lua_State *L)
{
	if(!lua_isstring(L, 1))
	{
		lua_pushliteral(L, "incorrect argument");
		lua_error(L);
	}
	const char *arg1 = lua_tostring(L, 1);

	lua_getglobal(L, "__HTTP_RESPONSE__");
	lua_pushstring(L, arg1);

	lua_concat(L, 2);
	lua_setglobal(L, "__HTTP_RESPONSE__");

	return 0;
}
