#ifndef __DEVICEAGENT_SOCKET_WRITE_OP_HPP__
#define __DEVICEAGENT_SOCKET_WRITE_OP_HPP__
#include "socket_op_decl.hpp"

namespace zl_device_agent {

class socket_write_op : public op_poll{
public:

	typedef void (cbase::*handle_write)(int ec, int bytes);
	struct write_handler : public handler<handle_write> {
		write_handler() {}
		write_handler(cbase* o, handle_write h)
			: handler<handle_write>(o, h) {}
	};

	explicit socket_write_op(socket_type sock, write_handler handler,
		cbuffer* buf, int flag)
		: op_poll(static_cast<cbase*>(this),
			(do_work_func) &socket_write_op::do_work,
			static_cast<cbase*>(this),
			(complete_func) &socket_write_op::do_callback),
			sock_(sock), handler_(handler), buf_(buf), flag_(flag),
			addr_(), addr_len_(0)
	{}

	socket_write_op(socket_type sock, write_handler handler,
		cbuffer* buf, const sockaddr& addr, int flag)
		: op_poll(static_cast<cbase*>(this),
			(do_work_func) &socket_write_op::do_work,
			static_cast<cbase*>(this),
			(complete_func) &socket_write_op::do_callback),
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
				//std::cout << "[" << count << "]: " << vecs[count].iov_base << ", " << vecs[count].iov_len << std::endl;
			}

			msg.msg_iov = vecs;
			msg.msg_iovlen = count;
			if (addr_len_ > 0) {
				msg.msg_name = const_cast<sockaddr*>(&addr_);
				msg.msg_namelen = static_cast<int>(addr_len_);
			}

		  bytes = ::sendmsg(sock_, &msg, flag_);
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
		write_handler handler(handler_.object, handler_.proc);
		int ec = ec_;
		std::size_t bytes = bytes_trans_;
		socket_write_op* p = this;
		p->~socket_write_op();
		::operator delete(p);
		if (!destroy) {
			(handler.object->*handler.proc)(ec, bytes);
		}
	}

private:
	socket_type sock_;
	write_handler handler_;
	cbuffer* buf_;
	int flag_;
	const sockaddr addr_;
	const std::size_t addr_len_;
};
}	// namespace zl_device_agent
#endif	// __deviceagent_stream_socket_write_op_hpp__
