#ifndef __DEVICEAGENT_SOCKET_OBJECT_H__
#define __DEVICEAGENT_SOCKET_OBJECT_H__
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <poll.h>

#include "object_base.h"
#include "protocol/protocol.h"

namespace zl_device_agent {

class socket_object : public object_base {
public:
	typedef core_proactor::op_poll op_poll;

	socket_object(protocol& p)
		: object_base(), protocol_(p)
	{}

	socket_object(protocol* pp)
		: object_base(), protocol_(*pp)
	{}

	~socket_object()
	{}

	int open();

	void close();

	int set_option(const int& name, const int& opt);

	int get_option(const int& name, int& opt);

	int set_nonblocking(bool nonblocking);

	int get_nonblocking(bool& nonblocking);

	bool is_open();

	int connect(const std::string& remote, int port);

	int connect(const char* remote_cstr, int port);

	/*
	void async_connect();
	*/

	void async_write(cbuffer* buf, write_handler& handler, int flag);

	void async_read(cbuffer* buf, read_handler& handler, int flag);

	void async_write_to(cbuffer* buf, const sockaddr& addr, write_handler& handler, int flag);

	void async_read_from(cbuffer* buf, const sockaddr& addr, read_handler& handler, int flag);

protected:

	// TODO custom error wrapper
	int do_connect(sockaddr* addr);

	int do_open();

	void do_close();

	void start_write_op(op_poll* op, bool nonblocking, bool notask);

	void start_read_op(op_poll* op, bool nonblocking, bool notask);

	void start_connect();

	/*
	void start_accept();

	void handle_write(int ec, int bytes);

	void handle_read(int ec, int bytes);
	*/

private:
	protocol protocol_;
};
}	// namespace zl_device_agent
#endif	// __deviceagent_socket_object_h__
