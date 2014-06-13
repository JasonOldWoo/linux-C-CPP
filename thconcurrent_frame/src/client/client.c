#include "common.h"
#include "message_type.h"
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
/*
	if (argc != 3)
	{
		printf("Usage : client server_addr message\n");
		return -1;
	}
	srand(time(NULL));
*/
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

	while (connect(client_sockd, (SKADDR *) &server_addr,
	            (socklen_t) sizeof (SKADDR)) != 0)
	{
		perror("connect error ");
		sleep(1);
	}

	printf("connect success!\n");
	char send_buf[BUFFER_SIZE];
	char *obuf = send_buf;

	CMSG* cmsg = create_cmsg(USER_LOGIN);
	char* username = "jason";
	push_cmsg(cmsg, username, strlen(username)+1);
	char* password = "123456";
	push_cmsg(cmsg, password, strlen(password)+1);
	unsigned long uid = 123456789;
	push_cmsg(cmsg, (char*) &uid, sizeof (uid));
	//end_cmsg(cmsg);
	//printf("message type[%hu]\n", *(unsigned short*) cmsg->msg);
	int i=0;
	for (i=0; i<100; i++)
	{
		if (send(client_sockd, cmsg->msg, cmsg->msg_pos, 0) < 0)
		{
			perror("send error ");
			close(client_sockd);
			if ((client_sockd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				perror("socket error ");
				return -1;
			}
			if (connect(client_sockd, (SKADDR *) &server_addr,
			            (socklen_t) sizeof (SKADDR)) != 0)
			{
				perror("connect error ");
				return -1;
			}
			sleep(1);
			continue ;
		}
		printf("send OK!\n");
		sleep(1);
	}
	printf("count[%d]\n", i);
	destroy_cmsg(cmsg);

	close(client_sockd);
	return 0;
}
