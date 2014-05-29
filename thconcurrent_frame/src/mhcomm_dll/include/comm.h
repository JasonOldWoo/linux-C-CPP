#ifndef __COMM_H__
#define __COMM_H__

#include "common.h"
#define COMM_PORT 9142

int mh_sendto(const char *dest_addr, const char *send_buf, socklen_t send_len);

int mh_init_server_sock(short port, int type);

int mh_init_server_tcp_sock(short port, int backlog);

#endif // __COMM_H__
