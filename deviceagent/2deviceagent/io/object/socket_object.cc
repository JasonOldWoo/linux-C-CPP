#include <string.h>
#include <arpa/inet.h>
#include "socket_object.h"

namespace zl_device_agent {

int socket_object::open() {
	return do_open();
}

void socket_object::close() {
	do_close();
}

bool socket_object::is_open() {
	return (descriptor_ != invalid_socket);
}

int socket_object::connect(const std::string& remote, int port) {
	sockaddr_in addr;
	memset(&addr, 0x0, sizeof (sockaddr_in));
	addr.sin_family = protocol_.family;
	addr.sin_port = htons(port);
	inet_aton(remote.c_str(), &addr.sin_addr);

	return do_connect((sockaddr*) &addr);
}

int socket_object::connect(const char* remote_cstr, int port) {
	sockaddr_in addr;
	memset(&addr, 0x0, sizeof (sockaddr_in));
	addr.sin_family = protocol_.family;
	addr.sin_port = htons(port);
	inet_aton(remote_cstr, &addr.sin_addr);

	return do_connect((sockaddr*) &addr);
}

int socket_object::do_connect(sockaddr* addr) {
	if (!is_open()) {
		return -1;
	}

	std::size_t addr_len = sizeof (sockaddr);
	int result = ::connect(descriptor_, addr, addr_len);

	if (!result) {
		pollfd fds;
		fds.fd = descriptor_;
		fds.events = POLLOUT;
		fds.revents = 0;
		result = ::poll(&fds, 1, -1);
		if (result >= 0) {
			return 0;
		} else {
			std::cerr << __func__ << " -- ::poll failed" << std::endl;
			return errno;
		}
	} else {
		std::cerr << __func__ << " -- ::connect failed" << std::endl;
		return errno;
	}
}

void socket_object::async_write(cbuffer* buf, write_handler& handler, int flag) {
	socket_write_op* p =
		new socket_write_op(descriptor_, handler, buf, flag);

	start_write_op(p, true, (buf->get_total() == buf->get_trans()));
}

void socket_object::async_read(cbuffer* buf, read_handler& handler, int flag) {
	socket_read_op* p =
	  new socket_read_op(descriptor_, handler, buf, flag);

	start_read_op(p, true, (buf->get_total() == buf->get_trans()));
}

void socket_object::async_write_to(cbuffer* buf, const sockaddr& addr, write_handler& handler, int flag) {
	socket_write_op* p =
	  new socket_write_op(descriptor_, handler, buf, addr, flag);

	start_write_op(p, true, (buf->get_total() == buf->get_trans()));
}

void socket_object::async_read_from(cbuffer* buf, const sockaddr& addr, read_handler& handler, int flag) {
		socket_read_op* p =
		new socket_read_op(descriptor_, handler, buf, addr, flag);

	start_read_op(p, true, (buf->get_total() == buf->get_trans()));
}

int socket_object::do_open() {
	if (is_open()) {
		return -1;
	}

	int err = ::socket(protocol_.family, protocol_.level, 0);

	if (err < 0) {
		descriptor_ = invalid_socket;
		std::cerr << __func__ << " -- ::socket failed" << std::endl;
		return errno;
	} else {
		descriptor_ = err;
	}

	if ((err = proactor_->attach(*this))) {
		std::cerr << __func__ << " -- attach failed" << std::endl;
		return err;
	}
	std::cout << this << " attach, dd_: " << descriptor_data_ << std::endl;

	return 0;
}

void socket_object::do_close() {
	if (!is_open()) {
	  std::cerr << __func__ << " -- already closed" << std::endl;
	  return ;
	}

	proactor_->detach(*this, true);
	::close(descriptor_);
	descriptor_ = invalid_socket;
	std::cout << this << " detach, dd_: " << descriptor_data_ << std::endl;
}

void socket_object::start_write_op(op_poll* op,
	bool nonblocking, bool notask) {
	if (!notask) {
		bool sock_internal_blocking = false;
		int err = get_nonblocking(sock_internal_blocking);
		if ((!err && sock_internal_blocking)
			|| !(op->ec_ = set_nonblocking(true))) {
			proactor_->start_write_op(op, *this, nonblocking);
			return ;
		}
	}

	proactor_->post_immediate_completion(op);
}

void socket_object::start_read_op(op_poll* op,
	bool nonblocking, bool notask) {
	if (!notask) {
		bool sock_internal_blocking = false;
		int err = get_nonblocking(sock_internal_blocking);
		if ((!err && sock_internal_blocking)
			|| !(op->ec_ = set_nonblocking(true))) {
		  proactor_->start_read_op(op, *this, nonblocking);
		  return ;
		}
	}

	proactor_->post_immediate_completion(op);
}

int socket_object::set_option(const int& name, const int& opt) {
	if (!is_open()) {
		return -1;
	}

	const char* opt_ptr = reinterpret_cast<const char*>(&opt);
	int err = ::setsockopt(descriptor_, SOL_SOCKET, name,
		opt_ptr, sizeof (opt));

	if (!err) {
		return 0;
	} else {
		return errno;
	}
}

int socket_object::get_option(const int& name, int& opt) {
	if (!is_open()) {
		return -1;
	}

	char* opt_ptr = reinterpret_cast<char*>(&opt);
	socklen_t len = 0;
	int err = ::getsockopt(descriptor_, SOL_SOCKET, name,
		opt_ptr, &len);

	if (!err) {
		return 0;
	} else {
		return errno;
	}
}

int socket_object::set_nonblocking(bool nonblocking) {
	if (!is_open()) {
		return -1;
	}

	int result = ::fcntl(descriptor_, F_GETFL, 0);
	if (result >= 0) {
		int flag = nonblocking ?
			(result | O_NONBLOCK) : (result & ~O_NONBLOCK);
		result = ::fcntl(descriptor_, F_SETFL, flag);
	}

	if (result < 0) {
		return errno;
	} else {
		return 0;
	}
}

int socket_object::get_nonblocking(bool& nonblocking) {
	if (!is_open()) {
		return -1;
	}

	int result = ::fcntl(descriptor_, F_GETFL, 0);
	if (result >= 0) {
		nonblocking = (result & O_NONBLOCK);
		return 0;
	} else {
		return errno;
	}
}
}	// namespace zl_device_agent
