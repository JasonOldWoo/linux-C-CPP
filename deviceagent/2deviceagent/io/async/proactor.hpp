#ifndef __DEVICEAGENT_PROACTOR_H__
#define __DEVICEAGENT_PROACTOR_H__

#include "./operation.hpp"
#include "../../basedef.h"

namespace zl_device_agent {
namespace async_io {
template <typename config>
class proactor : public cbase {
public:

	typedef typename config::poll_type reactor;
	typedef typename config::service_type io_service;
	typedef typename config::object_impl object_impl;
	typedef typename config::op_poll op_poll;
	typedef typename config::post_handler post_handler;
	typedef typename config::handle_post handle_post;

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
	void start_write_op(op_poll* op, object_impl& impl, bool is_nonblocking);

	/**
	 * @desc 开始读操作
	 **/
	void start_read_op(op_poll* op, object_impl& impl, bool is_nonblocking);

	/**
	 * @desc 开始连接操作
	 **/
	void start_connect_op(op_poll* op, object_impl& impl, bool is_nonblocking);

	/**
	 * @desc 执行I/O和用户回调 
	 **/
	int run(int& ec);

	/**
	 * @desc 立即执行任务
	 **/
	void post_immediate_completion(op_poll* op);

	/**
 	 * @desc 提交任务
 	 **/
	void post(post_handler& h);

	/**
	 * @desc 停止服务,并且销毁当前任务(不执行用户回调)
	 **/
	void shutdown_service();

	/**
 	 * @desc 停止服务,并且会终断反应器的阻塞
 	 **/
	void stop();

	/**
 	 * @desc 是否停止
 	 **/
	bool stopped();

	/**
 	 * @desc 重置服务
 	 **/
	void reset();

	/**
 	 * @desc 取消任务
 	 **/
	void cancel_ops(object_impl& impl);

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
