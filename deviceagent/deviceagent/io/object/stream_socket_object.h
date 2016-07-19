#ifndef __STREAM_SOCKET_OBJECT_H__
#define __STREAM_SOCKET_OBJECT_H__
#include "object_base.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <poll.h>

namespace zl_device_agent {

class stream_socket_object : public object_base {
public:
	typedef core_proactor::reactor_op reactor_op;

	stream_socket_object()
		: object_base()
	{object_base::init();}

	~stream_socket_object()
	{object_base::destroy();}

	int open();

	void close();

	int set_option(const int& name, const int& opt);

	int get_option(const int& name, int& opt);

	int set_nonblocking(bool nonblocking);

	int get_nonblocking(bool& nonblocking);

	bool is_open();

	int connect(sockaddr* addr);

	void async_connect();

	void async_write(cbuffer* buf, int flag);

protected:

	// TODO custom error wrapper
	int do_open();

	void do_close();

	void start_write_op(reactor_op* op, bool nonblocking, bool notask);

	void start_connect();

	/*
	void start_accept();
	*/

	void handle_write(int ec, int bytes);

	void handle_read(int ec, int bytes);
};
}	// namespace zl_device_agent
#endif	// __stream_socket_object_h__
