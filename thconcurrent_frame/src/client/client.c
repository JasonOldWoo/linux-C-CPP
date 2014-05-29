#include "common.h"
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage : client server_addr message\n");
		return -1;
	}
	srand(time(NULL));
	SKADDRIN server_addr;
	int client_sockd;

	memset(&server_addr, 0, sizeof (SKADDRIN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DEFAULT_PORT);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	if ((client_sockd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error ");
		return -1;
	}
	/*
	if ((send_len=sendto(client_sockd, send_buf,
	                     sizeof (send_buf), 0,
	                     (SKADDR *) &server_addr,
	                     (socklen_t) sizeof (SKADDR))) < 0)
	{
		perror("sendto error ");
		return -1;
	}
	sleep(1);
	printf("send over! send_len[%d], dest_addr=[%s]:[%u]\n", send_len,
	       inet_ntoa(server_addr.sin_addr), ntohl(server_addr.sin_addr.s_addr));
	*/

	if (connect(client_sockd, (SKADDR *) &server_addr,
	            (socklen_t) sizeof (SKADDR)) != 0)
	{
		perror("connect error ");
		return -1;
	}

	printf("connect success!\n");
	char send_buf[BUFFER_SIZE];
	sprintf(send_buf, "CLISOCK[%d]CLIPID[%d] : hello, world!", client_sockd, (int) getpid());
	printf("%s\n", send_buf);
	int i=0;
	for (; i<atoi(argv[2]); i++)
	{
		send(client_sockd, send_buf, strlen(send_buf)+1, 0);
		sprintf(send_buf, "CLISOCK[%d]CLIPID[%d] : [%d]", client_sockd, (int) getpid(), i+1);
		char recv_buf[BUFFER_SIZE];
		memset(recv_buf, 0, sizeof recv_buf);
		if (recv(client_sockd, recv_buf, BUFFER_SIZE, 0) < 0)
		{
			perror("recv error ");
			exit(-1);
		}
		printf("MESSAGE FROM SERVER:[%s]\n", recv_buf);
	}
	printf("Over!\n");

	close(client_sockd);
	return 0;
}
