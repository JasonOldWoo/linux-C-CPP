#include "../../stream_socket.h"
#include "../../../thread_que.h"
#include <string.h>
#include <arpa/inet.h>

#define remote_address "10.12.2.100"
#define remote_port 3966
#define remote_port2 3967

using zl_device_agent::cbuffer;

void print(cbuffer* buf) {
	for (cbuffer* item = buf->get_head(); 
		item && item->get_next() != item->get_head();
		item = item->get_next()) {
		std::cout << "head: " << item->get_head() << std::endl
			<< "end: " << item->get_end() << std::endl
			<< "len: " << item->get_len() << std::endl
			<< "total: " << item->get_total() << std::endl
			<< "trans: " << item->get_trans() << std::endl
			<< "data: " << item->get_data() << std::endl
			<< "consumed: " << item->get_consumed() << std::endl
			<< "cur: " << item->get_cur() << std::endl
			<< "self: " << item << std::endl;
		std::cout << std::endl;
	}
}

class client : public zl_device_agent::cbase {
public:
	client()
	{
		sock_ = new zl_device_agent::stream_socket;
		sock_->open();
		zl_device_agent::write_handler wh;
		wh.object = this;
		wh.proc = (zl_device_agent::p_handle_write) &client::on_write_data;
		sock_->set_write_handler(wh);
		zl_device_agent::read_handler rh;
		rh.object = this;
		rh.proc = (zl_device_agent::p_handle_read) &client::on_read_data;
		sock_->set_read_handler(rh);
		for (int i = 0; i < MAX_CBUFF_SIZ; i++) {
			const std::size_t len = 1024 * 1024 * 10;
			char* msg = new char[len];
			buf_[i] = new zl_device_agent::cbuffer(msg, len);
			if (i > 0) {
				buf_[i - 1]->append(buf_[i]);
			}
		}
		//print(*buf_);
	}

	~client() {
		sock_->close();
		destroy_buffer_list(*buf_);
	}

	int connect() {
		zl_device_agent::remote_info ri = {remote_address, remote_port};
		return sock_->connect(&ri);
	}

	int connect2() {
		zl_device_agent::remote_info ri = {remote_address, remote_port2};
		return sock_->connect(&ri);
	}

	void async_write() {
		//std::cout << __func__ << std::endl;
		sock_->async_write(*buf_, MSG_DONTWAIT);
	}

	void async_read() {
		sock_->async_read(*buf_, 0);
	}

	void stop() {
		zl_device_agent::post_handler h;
		h.object = (cbase*) this;
		h.proc = (zl_device_agent::handle_post) &client::do_stop;
		sock_->post(h);
	}

	void do_stop(void* ) {
		std::cout << __func__ << std::endl;
		sock_->close();
	}

	void on_write_data(int ec, int bytes) {
		if (ec && !bytes) {
			std::cout << __func__ << " -- on error: " << ec << std::endl;
			return ;
		}

		if ((*buf_)->get_trans() >= (*buf_)->get_total()) {
			std::cout << __func__ <<  " -- send ok: " << (*buf_)->get_trans() << std::endl;
			(*buf_)->set_bytes(0);
			async_write();
			return ;
		} else {
			async_write();
		}
	}

	void on_read_data(int ec, int bytes) {
		if (ec && !bytes) {
			std::cout << __func__ << " -- on error: " << ec << std::endl;
			return ;
		}

		if ((std::size_t) (*buf_)->get_trans() >= (*buf_)->get_total()) {
			std::cout << __func__ << " -- recv ok: " << (*buf_)->get_trans() << std::endl;
			(*buf_)->set_bytes(0);
			async_read();
			return ;
		} else {
			async_read();
		}
	}

	void run(void* args) {
	  int ec = *(int*) args;
	  sock_->run_task(ec);
	}

private:
	zl_device_agent::stream_socket* sock_;
	zl_device_agent::cbuffer* buf_[MAX_CBUFF_SIZ];
};

int main() {
	zl_device_agent::task_service_cleanup clean;
	client c;
	client c2;
	assert(!c.connect());
	assert(!c2.connect2());
	int ec = 0;
	int ec2 = 0;
	using zl_device_agent::routine_handler;
	using zl_device_agent::routine;
	using zl_device_agent::cbase;
	routine_handler handler;
	handler.object = (cbase*) &c;
	handler.proc = (routine) &client::run;
	handler.args = &ec;

	routine_handler handler2;
	handler2.object = (cbase*) &c2;
	handler2.proc = (routine) &client::run;
	handler2.args = &ec2;
	c.async_read();
	c2.async_write();

	// one for read and one for write
	g_thread_que.add_worker(&handler);
	g_thread_que.add_worker(&handler2);

	char msg[1024] = "";
	while (std::cin.getline(msg, 1024)) {
	  if (!strcmp("s", msg)) {
	  } else if (!strcmp("r", msg)) {
			c.stop();
			c2.stop();
	  } else if (!strcmp("q", msg)) {
			std::cout << "quit" << std::endl;
			zl_device_agent::task_stop();
			g_thread_que.destroy();
			return 0;
		}
	}
}
