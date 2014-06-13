#ifndef __MESSAGE_TYPE_H__
#define __MESSAGE_TYPE_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CMSG_BUF_SIZE 8192

// 二进制流数据，请勿独立操作该结构体
typedef struct CommMessage
{
	unsigned short	msg_type;
	unsigned short	buf_len;
	unsigned short	msg_pos;
	char*		msg;
} CMSG;

// 创建并初始化一个CMSG
CMSG* create_cmsg(unsigned short type);

// 销毁一个CMSG
void destroy_cmsg(CMSG* cmsg);

// 向CMSG中添加数据
int push_cmsg(CMSG* cmsg, char *msg, unsigned short msg_len);

// 对CMSG的消息部分进行解析，传递给自定的参数
int unpacket_message(const char* msg, unsigned long msg_len, ...);

#endif	// message_type.h
