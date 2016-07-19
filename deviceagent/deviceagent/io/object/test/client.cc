#include "../stream_socket_object.h"
#include <string.h>
#include <arpa/inet.h>

class client : public zl_device_agent::cbase{
public:
	client() {
		sock_ = new zl_device_agent::stream_socket_object;
		sock_->open();
		zl_device_agent::write_handler h;
		h.object = this;
		h.proc = (zl_device_agent::p_handle_write) &client::on_write_data;
		sock_->set_write_handler(h);
		const std::size_t len = 1024 * 1024 * 256;
		//memset(&buf_, 0x0, sizeof (buf_));
		buf_.msg = new char[len];
		buf_.len = len;
	}

	int connect() {
		sockaddr_in addr;
		memset(&addr, 0x0, sizeof (sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(3966);
		inet_aton("10.12.2.124", &addr.sin_addr);
		return sock_->connect((sockaddr*) &addr);
	}

	void async_write() {
		std::cout << __func__ << std::endl;
		sock_->async_write(&buf_, MSG_DONTWAIT);
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
			std::cout << "trans_bytes:" << bytes << std::endl;
			async_write();
		}
	}

	void run() {
		int ec = 0;
		sock_->run(ec);
	}

private:
	zl_device_agent::stream_socket_object* sock_;
	zl_device_agent::cbuffer buf_;
};

int main() {
	client c;
	assert(!c.connect());
	c.async_write();
	c.run();
}
