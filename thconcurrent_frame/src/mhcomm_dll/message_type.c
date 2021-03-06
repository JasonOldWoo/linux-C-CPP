#include "message_type.h"
#include <setjmp.h>

int unpacket_message(const char*msg, unsigned long len, ...)
{
	char *tmp;
	unsigned short tmp_len;
	unsigned long length = 0;
	va_list arg_ptr;

	int pc = 0;
	int i = 0;

	va_start(arg_ptr, len);
	while (length < len)
	{
		printf("msg[%p]\n", msg);
		tmp_len = *((unsigned short*) msg);
		if (0 == tmp_len)
			break;
		msg = msg + sizeof (unsigned short);
		length += 2;
		tmp = va_arg(arg_ptr, char*);
		if (NULL == tmp)
			break;
		printf("tmp[%p], tmp_len[%hu]\n", tmp, tmp_len);
		memcpy(tmp, msg, tmp_len);
		msg = msg + tmp_len;
		length += tmp_len;
	}
	va_end(arg_ptr);
	return 0;
}


// CMSG务必以该方式创建
CMSG* create_cmsg(unsigned short type)
{
	CMSG* cmsg = (CMSG*) malloc(sizeof (CMSG));
	cmsg->msg_type = type;
	cmsg->msg = (char*) malloc(MAX_CMSG_BUF_SIZE);
	memcpy(cmsg->msg, (char*) &type, sizeof (type));
	cmsg->msg_pos = sizeof (type);
	cmsg->buf_len = MAX_CMSG_BUF_SIZE;
	return cmsg;
}

// CMSG务必以该形式销毁
void destroy_cmsg(CMSG* cmsg)
{
	if (NULL == cmsg)
		return ;
	if (cmsg->msg != NULL)
	{
		free(cmsg->msg);
		cmsg->msg = NULL;
	}
	free(cmsg);
}

// 向CMSG里添加数据
int push_cmsg(CMSG* cmsg, char* msg, unsigned short msg_len)
{
	if (NULL == cmsg)
		return -1;
	if (msg_len + cmsg->msg_pos > cmsg->buf_len)
	{
		int tmp_len = cmsg->buf_len;
		while (msg_len + cmsg->msg_pos > tmp_len)
			tmp_len += MAX_CMSG_BUF_SIZE;
		char *msg_buf = (char*) malloc(tmp_len);
		memcpy(msg_buf, cmsg->msg, cmsg->msg_pos);
		if (NULL != cmsg->msg)
		{
			free(cmsg->msg);
			cmsg->msg = NULL;
		}
		cmsg->msg = msg_buf;
		cmsg->buf_len = tmp_len;
	}
	/*
	char *msg_buf = (char*) malloc(cmsg->msg_len + msg_len + sizeof (msg_len));
	memcpy(msg_buf, cmsg->msg, cmsg->msg_len);
	if (NULL != cmsg->msg)
	{
		free(cmsg->msg);
		cmsg->msg = NULL;
	}
	cmsg->msg = msg_buf;
	*/

	memcpy(cmsg->msg+cmsg->msg_pos, (char*) &msg_len, sizeof (unsigned short));
	cmsg->msg_pos += sizeof (unsigned short);
	memcpy(cmsg->msg+cmsg->msg_pos, msg, msg_len);
	cmsg->msg_pos += msg_len;
	
	return 0;
}

int end_cmsg(CMSG* cmsg)
{
	if (NULL == cmsg)
		return -1;
	if (1 + cmsg->msg_pos > cmsg->buf_len)
	{
		char *msg_buf = (char*) malloc(1 + cmsg->msg_pos);
		memcpy(msg_buf, cmsg->msg, cmsg->msg_pos);
		if (NULL != cmsg->msg)
		{
			free(cmsg->msg);
			cmsg->msg = NULL;
		}
		cmsg->msg = msg_buf;
		cmsg->buf_len = 1 + cmsg->msg_pos;
	}

	// set stop bit
	// char stop_bit = 'S';
	short stop_bit = 0;
	memcpy(cmsg->msg+cmsg->msg_pos, (char*) &stop_bit, sizeof (stop_bit));
	cmsg->msg_pos += sizeof (stop_bit);
}
