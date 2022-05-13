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
static void exec_lua(lua_State *l, char *path, size_t *len, int *tf, http_request_t *req, http_response_t *res);

void connection_handler(connection_t *c)
{
	char lennum[32] = {0}; // this is a little helper lol

	// NOTE: there could be edge cases here that idk about
	// this needs some testing.
	size_t raw_request_len = 0;
	char *raw_request = http_request_read(c->fd, &raw_request_len);
	http_request_t request = http_request_parse(raw_request);

	http_response_t response = {0};

	response.Protocol = "HTTP/1.0";
	response.headers = map_create(10, map_default_hash);
	map_add(&response.headers, "Server", "Giggle");

	// FIXME: This is done for the sole perpose that lua has a garbage collector
	// and there is literally noway to pull values out of it after it is closed
	// so we extending the scope of lua here lol. find a better way to do this.
	lua_State *L = NULL;

	// TODO: don't let this be hard coded for the love of god
	char *public_dir = "public";

	char path[64] = {0};
	strcpy(path, public_dir);
	strcat(path, request.URI);

	char *extention = strchr(path, '.');

	if(extention == NULL)
	{
		strcpy(path, map_get(c->routes, request.URI));
		if(path == NULL) strcpy(path, "public/404.html");
		extention = strchr(path, '.');
	}
	if(strcmp(extention, ".lua") == 0)
	{
		int status = 0;
		size_t len = 0;

		L = luaL_newstate();

		exec_lua(L, path, &len, &status, &request, &response);

		sprintf(lennum, "%ld", len);

		response.Status = http_response_status_str(status);
		map_add(&response.headers, "Content-Length", lennum);
		map_add(&response.headers, "Content-Type", "text/html; charset=utf-8");
		response.body_len = len;
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

			char *type = map_get(c->mime_types, extention);
			if(type) map_add(&response.headers, "Content-Type", type);

			response.body = data;
			response.body_len = len;
		}
		else
		{
			int status = 501;
			size_t len = 27;
			char *data = strdup("<h1>Unsupported Method</h1>");

			sprintf(lennum, "%ld", len);
			response.Status = http_response_status_str(status);
			map_add(&response.headers, "Content-Length", lennum);
			map_add(&response.headers, "Content-Type", "text/html; charset=utf-8");

			response.body = data;
			response.body_len = len;
		}
	}

	size_t raw_response_len = 0;
	char *raw_response = http_response_gen(&response, &raw_response_len);

	int prnt = send(c->fd, raw_response, raw_response_len, 0);
	if(prnt == -1) printf("%s\n", strerror(errno));

	// freeing all the memory so that the OS remains happy lol
	map_destroy(&request.headers);
	map_destroy(&request.body_fields);
	map_destroy(&response.headers);

	free(response.body);
	free(raw_response);
	free(raw_request);

	if(L) lua_close(L);

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

	char *file = malloc(file_stat.st_size * sizeof(char));

	FILE *fd = fopen(path, "rb");
	fread(file, sizeof(char), file_stat.st_size, fd);
	fclose(fd);

	//file[file_stat.st_size] = 0;

	*tf = 200;
	*len = file_stat.st_size;
	return file;
}

/* :: LUA code executions ahead lol */

struct http_lua
{
	FILE *stream;
	char *body;
	size_t len;

	int tf;
	http_request_t *req;
	http_response_t *res;
};

static int http_print_lua(lua_State *L);
static int http_status_lua(lua_State *L);
static int http_header_lua(lua_State *L);
static int http_getreq_lua(lua_State *L);
static int http_getform_lua(lua_State *L);

static void exec_lua(lua_State *L, char *path, size_t *len, int *tf, http_request_t *req, http_response_t *res)
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
			res->body = strdup("<h1>Not Found</h1>");
		}
		else exit(69);
	}

	// if the file is found but its not regular then 403
	if((file_stat.st_mode & S_IFMT) != S_IFREG)
	{
		*tf = 403;
		*len = 25;
		res->body = strdup("<h1>Forbidden Access</h1>");
		return;
	}

	struct http_lua htl = {0};
	htl.stream = open_memstream(&htl.body, &htl.len);
	htl.req = req;
	htl.res = res;

	lua_pushlightuserdata(L, &htl);
	lua_setglobal(L, "__HTTP__");

	lua_register(L, "http_print", http_print_lua);
	lua_register(L, "http_status", http_status_lua);
	lua_register(L, "http_header", http_header_lua);
	lua_register(L, "http_getreq", http_getreq_lua);
	lua_register(L, "http_getform", http_getform_lua);

	if(luaL_dofile(L, path) != LUA_OK)
	{
		if(lua_isstring(L, -1))
		{
			printf("%s\n", lua_tostring(L, -1));
		}
		lua_pop(L, 1);
	}

	fclose(htl.stream);

	htl.res->body = htl.body;
	*len = htl.len;
	*tf = htl.tf;
}

// TODO: separate out the error handling code from the main functional
// code. stuff looks really cluttered with all the error handling
static int http_print_lua(lua_State *L)
{
	int n = lua_gettop(L);	// getting the number of arguments by studying the stack
	// checking argument list and its type
	if(n != 1) luaL_error(L, "Function requires string as argument");
	if(!lua_isstring(L, 1)) luaL_argerror(L, 1, "The type of argument is not string");

	lua_getglobal(L, "__HTTP__");

	if(!lua_isuserdata(L, -1))
		luaL_error(L, "__HTTP__ is not found");

	struct http_lua *htl = lua_touserdata(L, -1);

	if(htl == NULL)
		luaL_error(L, "__HTTP__ was nil");

	fprintf(htl->stream, "%s\n", lua_tostring(L, 1));

	lua_pop(L, 1); // to remove global from stack

	return 0; // we returning no arguments lol
}

static int http_status_lua(lua_State *L)
{
	int n = lua_gettop(L);	// getting the number of arguments by studying the stack
	// checking argument list and its type
	if(n != 1) luaL_error(L, "Function requires integer as argument");
	if(!lua_isinteger(L, 1)) luaL_argerror(L, 1, "The type of argument is not integer");

	lua_getglobal(L, "__HTTP__");

	if(!lua_isuserdata(L, -1))
		luaL_error(L, "__HTTP__ is not found");

	struct http_lua *htl = lua_touserdata(L, -1);

	if(htl == NULL)
		luaL_error(L, "__HTTP__ was nil");

	htl->tf = lua_tointeger(L, 1);

	lua_pop(L, 1); // to remove global from stack

	return 0;
}

static int http_header_lua(lua_State *L)
{
	int n = lua_gettop(L);	// getting the number of arguments by studying the stack
	// checking argument list and its type
	if(n != 2) luaL_error(L, "Function requires string, string as arguments");
	if(!lua_isstring(L, 2)) luaL_argerror(L, 2, "The type of argument is not string");
	if(!lua_isstring(L, 1)) luaL_argerror(L, 1, "The type of argument is not string");

	lua_getglobal(L, "__HTTP__");

	if(!lua_isuserdata(L, -1))
		luaL_error(L, "__HTTP__ is not found");

	struct http_lua *htl = lua_touserdata(L, -1);

	if(htl == NULL)
		luaL_error(L, "__HTTP__ was nil");

	// TODO: this is a bit clunky... update the map structure to accomodate this edgecase
	// FIXME: this is some undefined teretory const is discarded
	if(map_add(&htl->res->headers, (char *)lua_tostring(L, 2), (char *)lua_tostring(L, 1)))
		map_set(&htl->res->headers, (char *)lua_tostring(L, 2), (char *)lua_tostring(L, 1));

	lua_pop(L, 1); // to remove global from stack

	return 0; // we returning no arguments lol
}

static int http_getreq_lua(lua_State *L)
{
	int n = lua_gettop(L);	// getting the number of arguments by studying the stack
	// checking argument list and its type
	if(n != 1) luaL_error(L, "Function requires string as argument");
	if(!lua_isstring(L, 1)) luaL_argerror(L, 1, "The type of argument is not string");

	lua_getglobal(L, "__HTTP__");

	if(!lua_isuserdata(L, -1))
		luaL_error(L, "__HTTP__ is not found");

	struct http_lua *htl = lua_touserdata(L, -1);

	if(htl == NULL)
		luaL_error(L, "__HTTP__ was nil");

	// FIXME: This is some undefined behaviour teretory here. it won't crash because map's current
	// behaviour but this is very bad needs to update
	char *key = (char *)lua_tostring(L, 1);

	const char *value = map_get(&htl->req->headers, key);

	lua_pop(L, 1); // to remove global from stack

	lua_pushstring(L, value ? value : NULL);

	return 1;
}

// TODO: code duplication do something about it!!
static int http_getform_lua(lua_State *L)
{
	int n = lua_gettop(L);	// getting the number of arguments by studying the stack
	// checking argument list and its type
	if(n != 1) luaL_error(L, "Function requires string as argument");
	if(!lua_isstring(L, 1)) luaL_argerror(L, 1, "The type of argument is not string");

	lua_getglobal(L, "__HTTP__");

	if(!lua_isuserdata(L, -1))
		luaL_error(L, "__HTTP__ is not found");

	struct http_lua *htl = lua_touserdata(L, -1);

	if(htl == NULL)
		luaL_error(L, "__HTTP__ was nil");

	// FIXME: This is some undefined behaviour teretory here. it won't crash because map's current
	// behaviour but this is very bad needs to update
	char *key = (char *)lua_tostring(L, 1);

	const char *value = map_get(&htl->req->body_fields, key);

	lua_pop(L, 1); // to remove global from stack

	lua_pushstring(L, value ? value : "");

	return 1;
}
