#ifndef __DEVICEAGENT_STREAM_SOCKET_H__
#define __DEVICEAGENT_STREAM_SOCKET_H__

#include <string>

#include "object/socket_object.h"
#include "object/protocol/tcp_protocol.h"

// TODO resolver service

namespace zl_device_agent {

struct remote_info {
	std::string host;
	int port;
};

class stream_socket  : public cbase {
public:
	stream_socket();

	stream_socket(const remote_info& remote);

	~stream_socket();

	void open();

	void close();

	int set_option(const int& name, const int& opt);

	int get_option(const int& name, int& opt);

	int set_nonblocking(bool nonblocking);

	int get_nonblocking(bool& nonblocking);

	bool is_open();

	int connect(const remote_info* remote);

	int connect();

	void async_write(cbuffer* buf, int flag = 0);

	void async_write(cbuffer* buf, write_handler& handler, int flag = 0);

	void async_read(cbuffer* buf, int flag = 0);

	void async_read(cbuffer* buf, read_handler& handler, int flag = 0);

	void set_write_handler(write_handler&);

	void set_read_handler(read_handler&);

	int run_task(int& ec);

	void cancel();

	void post(post_handler& h);

	inline core_proactor_ptr get_service_impl() {
		return interface_->get_service_impl();
	}

private:
	void init();

	void deinit();

	void handle_write(int ec, int bytes);

	void handle_read(int ec, int bytes);

private:
	socket_object* interface_;
	remote_info remote_;
	write_handler wh_;
	read_handler rh_;
};
}	// namespace zl_device_agent

#endif	// __deviceagent_stream_socket_h__
