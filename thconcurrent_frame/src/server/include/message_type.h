#ifndef __MESSAGE_TYPE_H__
#define __MESSAGE_TYPE_H__

#include <stdarg.h>
#include <string.h>

typedef struct CommMessage
{
	unsigned short	msg_type;
	unsigned long	msg_len;
	char*		msg;
} CMSG;

void unpacket_msg(const char* msg, unsigned long msg_len, ...);

#endif	// message_type.h
