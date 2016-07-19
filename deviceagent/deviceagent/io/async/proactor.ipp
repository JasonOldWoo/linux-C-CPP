#include "proactor.hpp"
#include "../operation.hpp"
//#include "task_io_service.hpp"
//// TODO 抽象类
//#include "epoll.hpp"

namespace zl_device_agent {
namespace async_io {

template <typename config>
proactor<config>::proactor() {
	io_service_ = new io_service(0);
}

template <typename config>
proactor<config>::~proactor() {
	if (io_service_) {
		delete io_service_;
	}
}

template <typename config>
void proactor<config>::init() {
	io_service_->init_task();
	reactor_ = io_service_->get_reactor();
}

template <typename config>
int proactor<config>::attach(object_impl& impl) {
	return reactor_->register_descriptor(impl.get_descriptor(),
		impl.get_reactor_data());
}

template <typename config>
void proactor<config>::detach(object_impl& impl, bool closing) {
	reactor_->deregister_descriptor(impl.get_descriptor(),
		impl.get_reactor_data(), closing);
}

template <typename config>
void proactor<config>::start_write_op(reactor_op* op, object_impl& impl,
	bool is_nonblocking) {
	reactor_->start_op(op_write, impl.get_descriptor(),
		impl.get_reactor_data(), op, is_nonblocking);
}

template <typename config>
void proactor<config>::start_read_op(reactor_op* op, object_impl& impl,
	bool is_nonblocking) {
	reactor_->start_op(op_read, impl.get_descriptor(),
			impl.get_reactor_data(), op, is_nonblocking);
}

template <typename config>
void proactor<config>::start_connect_op(reactor_op* op, object_impl& impl,
	bool is_nonblocking) {
	reactor_->start_op(op_connect, impl.get_descriptor(),
			impl.get_reactor_data(), op, is_nonblocking);
}

template <typename config>
void proactor<config>::run(int& ec) {
	io_service_->run(ec);
}

template <typename config>
void proactor<config>::post_immediate_completion(reactor_op* op) {
	io_service_->post_immediate_completion(op);
}

template <typename config>
void proactor<config>::shutdown_service() {
	io_service_->shutdown_service();
}

template <typename config>
void proactor<config>::stop() {
  io_service_->stop();
}

template <typename config>
bool proactor<config>::stopped() {
  io_service_->stopped();
}

template <typename config>
void proactor<config>::reset() {
  io_service_->reset();
}

}	// async_io
}	// namespace zl_device_agent
