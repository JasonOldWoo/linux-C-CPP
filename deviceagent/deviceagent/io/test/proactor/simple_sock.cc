#include "../../epoll.hpp"
#include "../../task_io_service.hpp"
#include "../../../tp_helper.h"
#include "../../../utils/mutex.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <limits>
#include <climits>

typedef zl_device_agent::async_io::op_poll op_poll;
typedef zl_device_agent::socket_type socket_type;
typedef zl_device_agent::async_io::work_handler work_handler;
typedef zl_device_agent::async_io::do_work_func do_work_func;
typedef zl_device_agent::complete_handler complete_handler;
typedef zl_device_agent::complete_func complete_func;

typedef bool (zl_device_agent::cbase::*handle_write)(int ec, int bytes);
struct write_handler : public zl_device_agent::handler<handle_write> {
  write_handler() {}
  write_handler(zl_device_agent::cbase* obj, handle_write wrh)
  : handler<handle_write>(obj, wrh)
  {}
};

struct cbuffer {
  char* msg;
  std::size_t len;
  std::size_t bytes_trans;
  int state;
};

struct core_config {
	typedef zl_device_agent::posix_mutex mutex;
	typedef zl_device_agent::posix_mutex::scoped_lock scoped_lock;
};

typedef zl_device_agent::async_io::epoller<core_config> reactor;
typedef zl_device_agent::async_io::task_io_service<reactor, core_config> task_io_service;

class sock_send_op : public op_poll {
public:
  explicit sock_send_op(socket_type sock, write_handler handler, cbuffer* buf)
    : op_poll(static_cast<cbase*>(this), (do_work_func) &sock_send_op::do_work,
      static_cast<cbase*>(this), (complete_func) &sock_send_op::do_callback),
    sock_(sock), handler_(handler), buf_(buf)
  {
  }

  bool do_work() {
    int bytes = ::send(sock_, buf_->msg + buf_->bytes_trans,
      buf_->len - buf_->bytes_trans, MSG_DONTWAIT);
    std::cout << __func__ << " -- bytes: " << bytes << std::endl;
		buf_->state = bytes;
    if (bytes < 0) {
			return false;
    } else {
			buf_->bytes_trans += bytes;
			return true;
    }
  }

  void do_callback(std::size_t) {
    std::cout << __func__ << " -- state: " << buf_->state
      << ", bytes: " << buf_->bytes_trans << std::endl;
    (handler_.object->*handler_.proc)(buf_->state, buf_->bytes_trans);
  }

private:
  socket_type sock_;
  write_handler handler_;
  cbuffer* buf_;
};

class sock {
public:
  sock() {
    sock_ = -1;
  }

  sock(write_handler handler, task_io_service* serv, std::string& remote_addr, int port)
    : wrh_(handler), io_service_(serv)
  {
    sock_ = ::socket(AF_INET, SOCK_STREAM, 0);

    bzero(&remote_, sizeof (remote_));
    remote_.sin_family = AF_INET;
    remote_.sin_port = htons(port);
    inet_aton(remote_addr.c_str(), &remote_.sin_addr);
    assert(::connect(sock_, (struct sockaddr*) &remote_, sizeof (remote_)) == 0);
    assert(io_service_->get_reactor()->register_descriptor(sock_, op_set_) == 0);
  }

  void async_write(cbuffer* buf) {
    if (sock_ < 0) return ;
    // TODO memory leak op
    sock_send_op* op = new sock_send_op(sock_, wrh_, buf);
    io_service_->get_reactor()->start_op(zl_device_agent::async_io::op_write, sock_, op_set_, op, true);
    return ;
  }

  ~sock() {
    if (sock_ > 0) {
    io_service_->get_reactor()->deregister_descriptor(sock_, op_set_, true);
    ::close(sock_);
    }
  }

private:
  write_handler wrh_;
  struct sockaddr_in remote_;
  socket_type sock_;
	task_io_service* io_service_;
  reactor::op_set* op_set_;
};

class client : public zl_device_agent::cbase {
  public:
  explicit client(std::string& remote, int port, cbuffer* buf)
	{
		if (!io_service_) {
			io_service_ = new task_io_service(1);
			io_service_->init_task();
		}
		tsock_ = new sock(write_handler(dynamic_cast<zl_device_agent::cbase*>(this),
			(handle_write) &client::write_handle), io_service_, remote, port);
		buf_ = buf;
	}

  ~client() {
		if (tsock_) {
			delete tsock_;
		}

		if (io_service_) {
			delete io_service_;
		}
  }

	bool write_handle(int ec, std::size_t bytes) {
		std::cout << __func__ << std::endl;
		if (NULL == tsock_) {
			return false;
		}
		if (bytes < buf_->len) {
			tsock_->async_write(buf_);
		} else {
			exit(1);
		}
		return true;
	}

	void run(void* args) {
		int ec = *(int*) args;
		io_service_->run(ec);
	}

  void consume() {
		tsock_->async_write(buf_);
		int ec = 0;
#if 0
		zl_device_agent::tp_handler handler;
		handler.object = this;
		handler.proc = (zl_device_agent::tp_proc) &client::run;
		handler.args = &ec;
		zl_device_agent::tp_helper::instance()->add_worker(&handler);
		zl_device_agent::tp_helper::instance()->add_worker(&handler);
		//zl_device_agent::tp_helper::instance()->add_worker(&handler);
		//zl_device_agent::tp_helper::instance()->add_worker(&handler);
		while (1) {
			sleep(1);
			std::cout << "hit" << std::endl;
		}
#else
		io_service_->run(ec);
#endif
	}

  private:
	static task_io_service* io_service_;
  sock* tsock_;
  cbuffer* buf_;
};

task_io_service* client::io_service_ = 0;

int main(int argc, char* argv[]) {
  assert(argc >= 3);
  std::string remote = argv[1];
  int port = atoi(argv[2]);
  cbuffer buf;
  memset(&buf, 0x0, sizeof (buf));
	::size_t len = 80 * 1024 * 1024;
  char* buffer = new char[len];
  buf.msg = buffer;
  buf.len = len;
  client c(remote, port, &buf);
  c.consume();
	delete buffer;
  return 0;
}
