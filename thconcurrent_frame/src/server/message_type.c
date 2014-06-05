#include "message_type.h"

void unpacket_message(const char*msg, unsigned long len, ...)
{
	char *tmp;
	unsigned short tmp_len;
	unsigned long length = 0;
	va_list arg_ptr;

	int pc = 0;
	int i = 0;

	va_start(arg_ptr, len);
	while (1)
	{
		if (length >= len)
			break;
		tmp_len = *((unsigned short*) msg);
		msg = msg + sizeof (unsigned short);
		length += 2;
		tmp = va_arg(arg_ptr, char*);
		memcpy(tmp, msg, tmp_len);
		msg = msg + tmp_len;
		length += tmp_len;
	}
	va_end(arg_ptr);
}
