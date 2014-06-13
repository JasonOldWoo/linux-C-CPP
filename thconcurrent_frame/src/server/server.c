#include "common.h"
#include "server_handler.h"
#include "comm.h"
#include "thread_pool.h"
#include <signal.h>
#include <setjmp.h>

int main(int argc, char *argv[])
{
	int server_sockd = mh_init_server_tcp_sock((short) DEFAULT_PORT, BACKLOG);
	if (server_sockd < 0)
	{
		printf("init sock error!\n");
		return -1;
	}

	printf("Start listening....\n");

	SKADDRIN client_addr;
	memset((void *) &client_addr, 0, sizeof (SKADDRIN));
	socklen_t sock_len = sizeof (SKADDR);

	int count = 0;
	int otid = 0;
	thpool_init(100);
	while (1)
	{
		int client_sockd = 0;
		if ((client_sockd=accept(server_sockd, (SKADDR *) &client_addr,
		                         &sock_len)) < 0)
		{
			perror("accept error ");
			close(server_sockd);
			close(client_sockd);
			server_sockd = mh_init_server_tcp_sock((short) DEFAULT_PORT, BACKLOG);
			//sleep(1);
			continue ;
		}
		//printf("[%d]CIP:[%s],PORT[%d]\n", ++count, inet_ntoa(client_addr.sin_addr),
		//       ntohs(client_addr.sin_port));

		// 给消息结构体赋值，从这里启动消息处理线程
		RMSG *rmsg = (RMSG *) malloc(sizeof (RMSG));
		memset(rmsg, 0, sizeof (RMSG));
		rmsg->cli_sockd = client_sockd;
		memcpy(&(rmsg->cli_addr), &client_addr, sizeof (client_addr));
		thpool_add_worker(server_handle, (void*) rmsg);
		//pthread_create(&tid, NULL, server_handle, (void *) rmsg);
	}
	close(server_sockd);
	return 0;
}
