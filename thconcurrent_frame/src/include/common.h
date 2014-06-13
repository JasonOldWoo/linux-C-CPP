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
#define DEFAULT_PORT 2280 
#define BUFFER_SIZE 2048
#define BACKLOG 10000

#define LOGIN 1
#define LOGOUT 2
#define P2PTRANS 3
#define GETALLUSER  4

typedef struct RemoteMessage
{
	int cli_sockd;
	SKADDRIN cli_addr;
	char cli_msg[BUFFER_SIZE];
} RMSG;


#define USER_LOGIN 1002

#endif
