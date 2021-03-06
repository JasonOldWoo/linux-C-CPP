#include "stream_socket_object.h"

namespace zl_device_agent {

int stream_socket_object::open() {
	return do_open();
}

void stream_socket_object::close() {
	do_close();
}

bool stream_socket_object::is_open() {
	return (descriptor_ != invalid_socket);
}

int stream_socket_object::connect(sockaddr* addr) {
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
			//std::cout << __func__ << " -- ok" << std::endl;
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

void stream_socket_object::async_write(cbuffer* buf, int flag) {
	write_handler wh;
	wh.object = static_cast<cbase*>(this);
	wh.proc = (stream_socket_write_op::handle_write)
		&stream_socket_object::handle_write;

	stream_socket_write_op* p =
		new stream_socket_write_op(descriptor_, wh, buf, flag);

	start_write_op(p, true, (buf->len == buf->bytes_trans));
}

void stream_socket_object::async_read(cbuffer* buf, int flag) {
	read_handler rh;
	rh.object = static_cast<cbase*>(this);
	rh.proc = (stream_socket_read_op::handle_read)
	  &stream_socket_object::handle_read;

	stream_socket_read_op* p =
	  new stream_socket_read_op(descriptor_, rh, buf, flag);

	start_read_op(p, true, (buf->len == buf->bytes_trans));
}

int stream_socket_object::do_open() {
	if (is_open()) {
		return -1;
	}

	int err = ::socket(AF_INET, SOCK_STREAM, 0);

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

	//std::cout << __func__ << " -- ok" << std::endl;
	return 0;
}

void stream_socket_object::do_close() {
	if (!is_open()) {
	  std::cerr << __func__ << " -- already closed" << std::endl;
	  return ;
	}

	proactor_->detach(*this, true);
	::close(descriptor_);
	descriptor_ = invalid_socket;
	//std::cout << __func__ << " -- ok" << std::endl;
}

void stream_socket_object::start_write_op(reactor_op* op,
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

void stream_socket_object::start_read_op(reactor_op* op,
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
}

int stream_socket_object::set_option(const int& name, const int& opt) {
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

int stream_socket_object::get_option(const int& name, int& opt) {
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

int stream_socket_object::set_nonblocking(bool nonblocking) {
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

int stream_socket_object::get_nonblocking(bool& nonblocking) {
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

void stream_socket_object::handle_write(int ec, int bytes) {
	(wh_.object->*wh_.proc)(ec, bytes);
}

void stream_socket_object::handle_read(int ec, int bytes) {
	(rh_.object->*rh_.proc)(ec, bytes);
}

}	// namespace zl_device_agent
