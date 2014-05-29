#include "server_handler.h"
#include "comm.h"

void *server_handle(void *args)
{
	RMSG *rmsg = (RMSG *) args;
	unsigned long tid = pthread_self();
	//printf("TID[%lu] : SOCK[%d], CIP[%s], PORT[%d]\n",	\
	       tid,	\
	       rmsg->cli_sockd,	\
	       inet_ntoa(rmsg->cli_addr.sin_addr),	\
	       ntohs(rmsg->cli_addr.sin_port));

	while(1)
	{
		char buf[BUFFER_SIZE] = "";
		if (recv(rmsg->cli_sockd, buf, BUFFER_SIZE, 0) <= 0)
		{
			//printf("TID[%lu] : SOCK[%d] recv error\n", tid, rmsg->cli_sockd);
			break;
		}
		printf("TID[%lu] : MESSAGE[%s]\n", tid, buf);
		sprintf(buf, "TID[%lu]", tid);
		send(rmsg->cli_sockd, buf, BUFFER_SIZE, 0);
	}
	close(rmsg->cli_sockd);
	//printf("TID[%lu] : SOCK[%d] closed....\n", tid, rmsg->cli_sockd);
	free(rmsg);

	return NULL;
}
