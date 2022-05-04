#include <stdio.h>
#include <signal.h>

#include "server.h"

#define PORT (8080)
#define BACKLOG (1)

static server_t sr = {0};

void exit_signal_handler(int wtf)
{
	if(wtf == SIGINT)
		sr.listning = 0;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	struct sigaction siga = {0};
	sigaction(SIGINT, NULL, &siga);
	siga.sa_handler = exit_signal_handler;
	sigaction(SIGINT, &siga, NULL);

	printf("Starting Server...\n");
	sr = server_create(PORT, BACKLOG);

	printf("Server Accepting Connections...\n");
	server_start(&sr);

	printf("Server Closing Connections...\n");
	server_destroy(&sr);

	return 0;
}
