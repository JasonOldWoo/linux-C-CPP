#ifndef __DEVICEAGENT_STREAM_SOCKET_READ_OP_HPP__
#define  __DEVICEAGENT_STREAM_SOCKET_READ_OP_HPP__
#include "./stream_socket_op_decl.hpp"

namespace zl_device_agent {

class stream_socket_read_op : public reactor_op {
public:

	typedef void (cbase::*handle_read)(int ec, int bytes);
	struct read_handler : public handler<handle_read> {
		read_handler() {}
		read_handler(cbase* o, handle_read h)
			: handler<handle_read>(o, h) {}
	};

	explicit stream_socket_read_op(socket_type sock, read_handler handler,
		cbuffer* buf, int flag)
		: reactor_op(static_cast<cbase*>(this),
			(do_work_func) &stream_socket_read_op::do_work,
			static_cast<cbase*>(this),
			(complete_func) &stream_socket_read_op::do_callback),
			sock_(sock), handler_(handler), buf_(buf), flag_(flag)
	{}

	bool do_work() {
		for (; ; ) {
			int bytes = 
		}
	}

	void do_callback(const int&, std::size_t, bool destroy) {
		read_handler handler(handler_.object, handler_.proc);
		cbuffer* buf = buf_;
		stream_socket_read_op* p = this;
		p->~stream_socket_read_op();
		::operator delete(p);
		if (!destroy) {
			(handler.object->*handler.proc)(buf->state, buf->bytes_trans);
		}
	}

private:
	socket_type sock_;
	read_handler handler_;
	cbuffer* buf_;
	int flag_;
};
}	// namespace zl_device_agent
#endif	// __deviceagent_stream_socket_hpp__
