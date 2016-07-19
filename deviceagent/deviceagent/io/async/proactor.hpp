#ifndef __DEVICEAGENT_PROACTOR_H__
#define __DEVICEAGENT_PROACTOR_H__

#include "../../basedef.h"
#include "../operation.hpp"

namespace zl_device_agent {
namespace async_io {
template <typename config>
class proactor : public cbase {
public:
	typedef typename config::poll_type reactor;
	typedef typename config::service_type io_service;
	typedef typename config::object_impl object_impl;
	typedef typename reactor::reactor_op reactor_op;
	typedef typename reactor::handle handle;

	proactor();
	~proactor();

	void init();

	/**
	 * @desc 注册描述符
	 **/
	int attach(object_impl& impl);

	/**
	 * @desc 注销描述符
	 **/
	void detach(object_impl& impl, bool closing);

	/**
	 * @desc 开始写操作
	 **/
	void start_write_op(reactor_op* op, object_impl& impl, bool is_nonblocking);

	/**
	 * @desc 开始读操作
	 **/
	void start_read_op(reactor_op* op, object_impl& impl, bool is_nonblocking);

	/**
	 * @desc 开始连接操作
	 **/
	void start_connect_op(reactor_op* op, object_impl& impl, bool is_nonblocking);

	/**
	 * @desc 执行I/O和用户回调 
	 **/
	void run(int& ec);

	/**
	 * @desc 立即执行任务
	 **/
	void post_immediate_completion(reactor_op* op);

	/**
	 * @desc 停止服务
	 **/
	void shutdown_service();

private:

	/**
	 * @desc 反应器
	 **/
	reactor* reactor_;

	/**
	 * @desc 任务管理
	 **/
	io_service* io_service_;
};
}	// namespace async_io
}	// namespace zl_device_agent

#include "proactor.ipp"
#endif
