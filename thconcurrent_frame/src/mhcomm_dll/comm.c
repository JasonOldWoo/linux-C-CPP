#include "comm.h"

int mh_sendto(const char *dest_ipaddr, const char *send_buf, socklen_t send_len)
{
	if (NULL==dest_ipaddr || NULL==send_buf || 0==send_len)
	{
		printf("mh_sendto() - Illegal dest_ipaddr/send_buf/send_len\n");
		return -1;
	}
	SKADDRIN dest_addr;
	memset(&dest_addr, 0, sizeof (SKADDRIN));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(COMM_PORT);
	dest_addr.sin_addr.s_addr = inet_addr(dest_ipaddr);

	int sockd = 0;
	if ((sockd=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("mh_sendto() - socket error ");
		return -1;
	}

	sendto(sockd, send_buf, send_len, 0,
	       (SKADDR *) &dest_addr, sizeof (SKADDR));
}

int mh_init_server_sock(short port, int type)
{
	SKADDRIN server_addr;
	int server_sockd;

	memset(&server_addr, 0, sizeof (SKADDRIN));
	server_addr.sin_family  = AF_INET;
	server_addr.sin_port =  htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((server_sockd=socket(AF_INET, type, 0)) < 0)
	{
		perror("socket error ");
		return -1;
	}

	int reuse = 1;
	if (setsockopt(server_sockd, SOL_SOCKET, SO_REUSEADDR,
	               &reuse, (socklen_t) sizeof (reuse)) < 0)
	{
		perror("setsockopt error ");
		return -1;
	}

	if (bind(server_sockd, (SKADDR *) &server_addr,
	         (socklen_t) sizeof (SKADDR)) < 0)
	{
		perror("bind error ");
		return -1;
	}
	return server_sockd;
}

int mh_init_server_tcp_sock(short port, int backlog)
{
	int server_sockd = 0;
	server_sockd = mh_init_server_sock(port, SOCK_STREAM);
	if (-1 == server_sockd)
		return -1;

	if (listen(server_sockd, backlog) < 0)
	{
		perror("listen error ");
		return -1;
	}
	return server_sockd;
}
