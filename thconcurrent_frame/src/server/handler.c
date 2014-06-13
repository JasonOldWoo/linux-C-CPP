#include "handler.h"
#include "message_type.h"
#include "common.h"


int server_process(const RMSG* rmsg, const char* ibuf, const ssize_t ilen, char** obufp, ssize_t* olenp)
{
	if (NULL == ibuf || NULL==obufp || NULL == olenp)
	{
		printf("server_process() - arg error!\n");
		return -1;
	}

	unsigned short type = *(unsigned short*) ibuf;
	printf("server_process() - type=%hd\n", type);
	ssize_t len = ilen + sizeof (short);
	const char* buf = ibuf + sizeof (short);
	printf("buf[%p]\n", buf);
	int ret = -1;
	switch (type)
	{
	case USER_LOGIN:
	{
		printf("server_process() - user login\n");
		ret = user_login(buf, len, obufp, olenp);
		break;
	}
	default:
	{
		ret = -1;
		printf("server_process() - Unknow type[%hd]\n", type);
		break;
	}
	}
	return ret;
}

int user_login(const char* ibuf, const ssize_t ilen, char** obufp, ssize_t* olenp)
{
	char username[129];
	char password[129];
	unsigned long uid = 0;
	unpacket_message(ibuf, ilen, username, password, (char*) &uid, NULL);
	printf("user_login() - username[%s], password[%s], uid[%lu]\n", username, password, uid);
}
