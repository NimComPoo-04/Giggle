#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "connection.h"

char *read_http_request(int fd, size_t *plen)
{
	char *buff = NULL;
	size_t len = 0;

	FILE *stream = open_memstream(&buff, &len);

	char buffer[256] = {0};
	size_t amt_read = 256;

	while(amt_read >= sizeof buffer)
	{
		amt_read = recv(fd, buffer, sizeof buffer, 0);
		fwrite(buffer, sizeof(char), amt_read, stream);
	}

	fclose(stream);

	if(plen) *plen = len;

	return buff;
}

/*
 * The lua script would write to a global value called
 * __HTTP_REQUEST__ the whole request.
 * __HTTP_RESPONSE__ the whole response. yea that would defenitely be fun
 * innit lol.
 *
 * This crap feels like a total HACK lol XD
 * who cares this is fun lol.
 */

void connection_handler(connection_t *c)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	int keep_alive = 1;

	printf("[Connected] :: %d\n", c->fd);

	while(keep_alive)
	{
		char *request = read_http_request(c->fd, NULL);
		const char *response = NULL;
		size_t len_response = 0;

		// starting a new lua vm

		lua_pushstring(L, request);
		lua_setglobal(L, "__HTTP_REQUEST__");

		if(luaL_dofile(L, "glue.lua") != LUA_OK)
		{
			if(lua_isstring(L, -1))
			{
				printf("%s\n", lua_tostring(L, -1));
			}
			lua_pop(L, 1);
		}

		lua_getglobal(L, "__HTTP_RESPONSE__");
		if(lua_isstring(L, -1))
		{
			response = lua_tolstring(L, lua_gettop(L), &len_response );
			lua_pop(L, 1);
		}

		lua_getglobal(L, "__KEEP_ALIVE__");
		if(lua_isinteger(L, -1))
		{
			keep_alive = lua_tointeger(L, lua_gettop(L));
			lua_pop(L, 1);
		}

		send(c->fd, response, len_response, 0);

		free(request);
	}

	printf("[Disconnected] :: %d\n", c->fd);

	// killing the vm because I can lol
	lua_close(L);

	shutdown(c->fd, SHUT_RDWR);
	close(c->fd);
}
