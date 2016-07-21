#include "stream_socket.h"

namespace zl_device_agent {

stream_socket::stream_socket()
	: interface_(0)
{
	init();
}

stream_socket::stream_socket(const remote_info& remote)
	: interface_(0),
	remote_(remote)
{
	init();
}

stream_socket::~stream_socket()
{
	deinit();
}

void stream_socket::init() {
	if (!interface_) {
		tcp_protocol p;
		interface_ = new socket_object(p);
	}
}

void stream_socket::deinit() {
	if (is_open()) {
		close();
	}

	if (interface_) {
		delete interface_;
		interface_ = 0;
	}
}

void stream_socket::open() {
	interface_->open();
}

void stream_socket::close() {
	interface_->close();
}

int stream_socket::set_option(const int& name, const int& opt) {
	return interface_->set_option(name, opt);
}

int stream_socket::get_option(const int& name, int& opt) {
	return interface_->get_option(name, opt);
}

bool stream_socket::is_open() {
	return interface_->is_open();
}

int stream_socket::connect(const remote_info* remote) {
	return interface_->connect(remote->host, remote->port);
}

int stream_socket::connect() {
	return interface_->connect(remote_.host, remote_.port);
}

void stream_socket::async_write(cbuffer* buf, int flag) {
	write_handler wh;
	wh.object = static_cast<cbase*>(this);
	wh.proc = (socket_write_op::handle_write)
		&stream_socket::handle_write;

	interface_->async_write(buf, wh, flag);
}

void stream_socket::async_write(cbuffer* buf, write_handler& handler,int flag) {
	interface_->async_write(buf, handler, flag);
}

void stream_socket::async_read(cbuffer* buf, int flag) {
	read_handler rh;
	rh.object = static_cast<cbase*>(this);
	rh.proc = (socket_read_op::handle_read)
	  &stream_socket::handle_read;

	interface_->async_read(buf, rh, flag);
}

void stream_socket::async_read(cbuffer* buf, read_handler& handler, int flag) {
	interface_->async_read(buf, handler, flag);
}

void stream_socket::set_write_handler(write_handler& h) {
	wh_ = h;
}

void stream_socket::set_read_handler(read_handler& h) {
	rh_ = h;
}

int stream_socket::run_task(int& ec) {
	return interface_->run(ec);
}

void stream_socket::cancel() {
	interface_->cancel();
}

void stream_socket::post(post_handler& h) {
	interface_->post(h);
}

void stream_socket::handle_write(int ec, int bytes) {
	(wh_.object->*wh_.proc)(ec, bytes);
}

void stream_socket::handle_read(int ec, int bytes) {
	(rh_.object->*rh_.proc)(ec, bytes);
}


}	// namespace zl_device_agent
