#include <stdio.h>

#include "server.h"

#define PORT (8080)
#define BACKLOG (1)

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	printf("Starting Server...\n");
	server_t sr = server_create(PORT, BACKLOG);

	printf("Server Accepting Connections...\n");
	server_start(&sr);

	printf("Server Closing Connections...\n");
	server_destroy(&sr);
	
	return 0;
}
