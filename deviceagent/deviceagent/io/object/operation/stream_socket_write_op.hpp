#include "../../../config/core.h"
#include "../../async/proactor.hpp"
#include "../../operation.hpp"
#include <sys/socket.h>
#include <sys/types.h>

namespace zl_device_agent {

// TODO include basic header file
typedef async_io::proactor<core_config> core_proactor;
typedef core_proactor* core_proactor_ptr;
typedef core_proactor::reactor_op reactor_op;

struct cbuffer {
	char* msg;
	std::size_t len;
	std::size_t bytes_trans;
	int state;
};

class stream_socket_write_op : public reactor_op {
public:

	typedef void (cbase::*handle_write)(int ec, int bytes);
	struct write_handler : public handler<handle_write> {
		write_handler() {}
		write_handler(cbase* o, handle_write h)
			: handler<handle_write>(o, h) {}
	};

	explicit stream_socket_write_op(socket_type sock, write_handler handler,
		cbuffer* buf, int flag)
		: reactor_op(static_cast<cbase*>(this),
			(do_work_func) &stream_socket_write_op::do_work,
			static_cast<cbase*>(this),
			(complete_func) &stream_socket_write_op::do_callback),
			sock_(sock), handler_(handler), buf_(buf), flag_(flag)
	{}

	bool do_work() {
		int bytes = ::send(sock_, buf_->msg + buf_->bytes_trans,
				buf_->len - buf_->bytes_trans, flag_);
		buf_->state = bytes;
		if (bytes < 0) {
			return false;
		} else {
			buf_->bytes_trans += bytes;
			return true;
		}
	}

	void do_callback(const int&, std::size_t, bool destroy) {
		write_handler handler(handler_.object, handler_.proc);
		cbuffer* buf = buf_;
		stream_socket_write_op* p = this;
		p->~stream_socket_write_op();
		::operator delete(p);
		if (!destroy) {
			(handler.object->*handler.proc)(buf->state, buf->bytes_trans);
		}
	}

private:
	socket_type sock_;
	write_handler handler_;
	cbuffer* buf_;
	int flag_;
};
}	// namespace zl_device_agent
