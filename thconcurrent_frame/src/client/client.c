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

	if (connect(client_sockd, (SKADDR *) &server_addr,
	            (socklen_t) sizeof (SKADDR)) != 0)
	{
		perror("connect error ");
		return -1;
	}

	printf("connect success!\n");
	char send_buf[BUFFER_SIZE];
	char *obuf = send_buf;
	//sprintf(send_buf, "CLISOCK[%d]CLIPID[%d] : hello, world!", client_sockd, (int) getpid());
	//sprintf(send_buf, "%hd%s%s", 1001, "jason", "123456");
	/*
	unsigned short type = 1001;
	unsigned short len = 0;
	unsigned short send_len = 0;
	memcpy(obuf, (char*) &type, sizeof (unsigned short));
	obuf += sizeof (unsigned short);
	send_len += sizeof (unsigned short);
	char* username = "jason";
	len = strlen(username) + 1;
	memcpy(obuf, (char*) &len, sizeof (unsigned short));
	send_len += sizeof (unsigned short);
	obuf += sizeof (unsigned short);
	memcpy(obuf, username, len);
	obuf += len;
	send_len += len;
	char* password = "123456";
	len = strlen(password) + 1;
	memcpy(obuf, (char*) &len, sizeof (unsigned short));
	obuf += sizeof (unsigned short);
	send_len += sizeof (unsigned short);
	memcpy(obuf, password, len);
	obuf += len;
	send_len += len;
	send(client_sockd, send_buf, send_len, 0);
	*/

	CMSG* cmsg = create_cmsg(1001);
	char* username = "jason";
	push_cmsg(cmsg, username, strlen(username)+1);
	char* password = "123456";
	push_cmsg(cmsg, password, strlen(password)+1);
	unsigned long uid = 123456789;
	//push_cmsg(cmsg, (char*) &uid, sizeof (uid));
	//printf("message type[%hu]\n", *(unsigned short*) cmsg->msg);
	send(client_sockd, cmsg->msg, cmsg->msg_len, 0);
	destroy_cmsg(cmsg);

	//printf("%s\n", send_buf);
	/*
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
	*/
	

	close(client_sockd);
	return 0;
}
