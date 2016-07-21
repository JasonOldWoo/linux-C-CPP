#ifndef __DEVICEAGENT_SOCKET_READ_OP_HPP__
#define  __DEVICEAGENT_SOCKET_READ_OP_HPP__
#include "socket_op_decl.hpp"

namespace zl_device_agent {

class socket_read_op : public op_poll {
public:

	typedef void (cbase::*handle_read)(int ec, int bytes);
	struct read_handler : public handler<handle_read> {
		read_handler() {}
		read_handler(cbase* o, handle_read h)
			: handler<handle_read>(o, h) {}
	};

	explicit socket_read_op(socket_type sock, read_handler handler,
		cbuffer* buf, int flag)
		: op_poll(static_cast<cbase*>(this),
			(do_work_func) &socket_read_op::do_work,
			static_cast<cbase*>(this),
			(complete_func) &socket_read_op::do_callback),
			sock_(sock), handler_(handler), buf_(buf), flag_(flag),
			addr_(), addr_len_(0)
	{}

	socket_read_op(socket_type sock, read_handler handler,
		cbuffer* buf, const sockaddr& addr, int flag)
		: op_poll(static_cast<cbase*>(this),
			(do_work_func) &socket_read_op::do_work,
			static_cast<cbase*>(this),
			(complete_func) &socket_read_op::do_callback),
			sock_(sock), handler_(handler), buf_(buf), flag_(flag),
			addr_(addr), addr_len_(sizeof (sockaddr))
	{}

	bool do_work() {
		int bytes = 0;
		for (; ; ) {
			msghdr msg = msghdr();
			iovec vecs[MAX_IOVEC_NUM];
			int count = 0;
			for (cbuffer* item = buf_->get_cur(); item; item = item->get_next(), count++) {
				vecs[count].iov_base = item->get_data() + item->get_consumed();
				vecs[count].iov_len = item->get_len() - item->get_consumed();
			}

			msg.msg_iov = vecs;
			msg.msg_iovlen = count;
			if (addr_len_ > 0) {
				msg.msg_name = const_cast<sockaddr*>(&addr_);
				msg.msg_namelen = static_cast<int>(addr_len_);
			}

			bytes = ::recvmsg(sock_, &msg, flag_);
			// 被中断的系统调用
			if (bytes < 0 && EINTR == errno) {
			  continue ;
			} else {
			  break ;
			}
		}

		if (bytes <= 0) {
			bytes_trans_ = 0;
			ec_ = errno;
		} else {
			buf_->add_bytes(bytes);
			bytes_trans_ = bytes;
			ec_ = 0;
		}

		// 需要重新执行
		if (EAGAIN == ec_) {
			return false;
		} else {
			return true;
		}
	}

	void do_callback(const int&, std::size_t, bool destroy) {
		read_handler handler(handler_.object, handler_.proc);
		int ec = ec_;
		std::size_t bytes = bytes_trans_;
		socket_read_op* p = this;
		p->~socket_read_op();
		::operator delete(p);
		if (!destroy) {
			(handler.object->*handler.proc)(ec, bytes);
		}
	}

private:
	socket_type sock_;
	read_handler handler_;
	cbuffer* buf_;
	int flag_;
	const sockaddr addr_;
	const std::size_t addr_len_;
};
}	// namespace zl_device_agent
#endif	// __deviceagent_socket_hpp__
