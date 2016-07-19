#include "../stream_socket_object.h"
#include "../../../tp_helper.h"
#include <string.h>
#include <arpa/inet.h>

class client : public zl_device_agent::cbase{
public:
	client() {
		sock_ = new zl_device_agent::stream_socket_object;
		sock_->open();
		zl_device_agent::write_handler wh;
		wh.object = this;
		wh.proc = (zl_device_agent::p_handle_write) &client::on_write_data;
		sock_->set_write_handler(wh);
		zl_device_agent::read_handler rh;
		rh.object = this;
		rh.proc = (zl_device_agent::p_handle_read) &client::on_read_data;
		sock_->set_read_handler(rh);
		const std::size_t len = 1024 * 1024 * 256;
		//memset(&buf_, 0x0, sizeof (buf_));
		buf_.msg = new char[len];
		buf_.len = len;
	}

	~client() {
	  delete [] buf_.msg;
	}

	int connect() {
		sockaddr_in addr;
		memset(&addr, 0x0, sizeof (sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(3966);
		inet_aton("10.12.2.100", &addr.sin_addr);
		return sock_->connect((sockaddr*) &addr);
	}

	void async_write() {
		std::cout << __func__ << std::endl;
		sock_->async_write(&buf_, MSG_DONTWAIT);
	}

	void async_read() {
		std::cout << __func__ << std::endl;
		sock_->async_read(&buf_, MSG_DONTWAIT);
	}

	void reset() {
#if 0
	  if (pp->stopped()) {
	  	pp->reset();
	  } else {
		pp->shutdown_service();
		pp->reset();
	  }
#else
	  zl_device_agent::core_proactor_ptr pp
	    = sock_->get_service_impl();
	  try {
	  	while (!pp->stopped()) {
		  sleep(1);
		  std::cout << "waiting for service stop" << std::endl;
	  	}
	  	pp->reset();
	  } catch(...) {
		std::cerr << "error!" << std::endl;
	  }
#endif
	  sock_->close();
	  sock_->open();
	  assert(!connect());
	  buf_.bytes_trans = 0;
	}

	void on_write_data(int ec, int bytes) {
		if (ec < 0) {
			std::cout << "on error" << std::endl;
			async_write();
			return ;
		}

		if ((std::size_t) bytes == buf_.len) {
			std::cout << "send ok" << std::endl;
			return ;
		} else {
			std::cout << "trans_bytes: " << bytes << std::endl;
			async_write();
		}
	}

	void on_read_data(int ec, int bytes) {
		if ((std::size_t) bytes == buf_.len) {
		  std::cout << "recv ok" << std::endl;
		  return ;
		} else {
		  std::cout << "trans_bytes: " << bytes << std::endl;
		  async_read();
		}
	}

	void run(void* args) {
	  int ec = *(int*) args;
	  sock_->run(ec);
	}

private:
	zl_device_agent::stream_socket_object* sock_;
	zl_device_agent::cbuffer buf_;
};

int main() {
	client c;
	assert(!c.connect());
	int ec = 0;
	zl_device_agent::tp_handler handler;
	handler.object = (zl_device_agent::cbase*) &c;
	handler.proc = (zl_device_agent::tp_proc) &client::run;
	handler.args = &ec;
	c.async_read();
	zl_device_agent::tp_helper::instance()->add_worker(&handler);
	char msg[1024] = "";
	while (std::cin.getline(msg, 1024)) {
	  if (!strcmp("s", msg)) {
		std::cout << "1111 resend" << std::endl;
		c.reset();
		c.async_write();
		std::cout << "2222 resend" << std::endl;
		zl_device_agent::tp_helper::instance()->add_worker(&handler);
	  } else if (!strcmp("r", msg)) {
		std::cout << "1111 rerecv" << std::endl;
		c.reset();
		c.async_read();
		std::cout << "222 rerecv" << std::endl;
		zl_device_agent::tp_helper::instance()->add_worker(&handler);
	  }
	}
}
