#ifndef __DEVICEAGENT_TCP_H__
#define __DEVICEAGENT_TCP_H__
#pragma once

#include "../basedef.h"
#include "async_io.hpp"
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/fcntl.h>

namespace zl_device_agent {
#define invalid_socket (-1)
class tcp_op : op_poll {
}
template <typename config>
class tcp {
public:
	typedef async_io::io<config> asio;
	typedef typename config::poll::poll_type poll_type;
	typedef typename poll_type::handle handle;

	explicit tcp(asio* io)
		: asio_(io)
	{
	}

	void open() {
		sock_ = do_open();
		if (sock_ != invalid_socket) {
			set_nonblocking(sock_);
		}
	}

	void async_connect() {
	}

	void async_send() {
	}

	void async_recv() {
	}

protected:
	int do_open() {
		if (is_open(sock_)) {
			return -1;
		}

		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (0 == sock) {
			sock = invalid_socket;
		}

		return sock;
	}

private:
	bool is_open(handle& sock) {
		return sock != invalid_socket;
	}

	int set_nonblocking(handle& sock) {
		int opts = 0;

		opts = fcntl(sock, F_GETFL);
		if (opts < 0) {
			return -1;
		}

		opts |= O_NONBLOCK;
		if (fcntl(sock, F_SETFL, opts) < 0) {
			return -1;
		}

		return 0;
	}

private:
	poll_type poller_;
	asio* asio_;
	handle sock_;
};
}	// zl_device_agent

#endif
