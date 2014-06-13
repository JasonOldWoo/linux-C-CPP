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
		ssize_t recv_len = 0;
		if ((recv_len=recv(rmsg->cli_sockd, buf, BUFFER_SIZE, 0)) <= 0)
		{
			//printf("TID[%lu] : SOCK[%d] recv error\n", tid, rmsg->cli_sockd);
			break;
		}
		char* obuf = (char*) malloc(BUFFER_SIZE);
		//memset(obuf, '\0', BUFFER_SIZE);
		ssize_t olen = 0;
		printf("server_handlder() - recv_len=[%d]\n", recv_len);
		server_process(rmsg, buf, recv_len, &obuf, &olen);
		send(rmsg->cli_sockd, obuf, olen, 0);
		if (obuf != NULL)
			free(obuf);
		obuf = NULL;
		//printf("TID[%lu] : MESSAGE[%s]\n", tid, buf);
		//sprintf(buf, "TID[%lu]", tid);
		//send(rmsg->cli_sockd, buf, BUFFER_SIZE, 0);
	}
	close(rmsg->cli_sockd);
	//printf("TID[%lu] : SOCK[%d] closed....\n", tid, rmsg->cli_sockd);
	free(rmsg);

	return NULL;
}
