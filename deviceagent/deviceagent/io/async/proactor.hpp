#ifndef __DEVICEAGENT_PROACTOR_H__
#define __DEVICEAGENT_PROACTOR_H__

#include "../../basedef.h"
#include "../operation.hpp"

namespace zl_device_agent {
namespace async_io {
template <typename config>
class proactor : public cbase {
	typedef typename config::template poll_type<config> reactor;
	typedef typename config::template service_type<reactor, config> io_service;
	typedef typename config::object_impl object_impl;
	typedef typename reactor::reactor_op reactor_op;
	typedef typename reactor::handle handle;

public:
	proactor();
	~proactor();

	void init();
	void register_handle();
	void set_open_handler();
	void set_write_handler();
	void set_read_handler();
	void set_fail_handler();
	void set_close_handler();
	void run();
	void start_write_op(reactor_op* op);
	void start_read_op();
	void start_connect_op();

private:
	reactor* reactor_;
	io_service* io_service_;
};
}	// namespace async_io
}	// namespace zl_device_agent

#include "proactor.ipp"
#endif
