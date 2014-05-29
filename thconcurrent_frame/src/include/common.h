#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SKADDRIN struct sockaddr_in
#define SKADDR struct sockaddr
#define DEFAULT_PORT 8999
#define BUFFER_SIZE 2048
#define BACKLOG 10000

typedef struct RemoteMessage
{
	int cli_sockd;
	SKADDRIN cli_addr;
	char cli_msg[BUFFER_SIZE];
} RMSG;

#endif
