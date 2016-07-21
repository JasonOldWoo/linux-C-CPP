#ifndef __DEVICEAGENT_OBJECT_BASE_H__
#define __DEVICEAGENT_OBJECT_BASE_H__

#include"../../basedef.h"
#include "../../config/posix.h"
#include "../../io/async/proactor.hpp"
#include "./operation/socket_write_op.hpp"
#include "./operation/socket_read_op.hpp"

namespace zl_device_agent {

typedef socket_write_op::write_handler write_handler;
typedef socket_write_op::handle_write p_handle_write;
typedef socket_read_op::read_handler read_handler;
typedef socket_read_op::handle_read p_handle_read;

const int invalid_socket = -1;

class object_base : public cbase {
public:

	explicit object_base();

	~object_base();

	/**
	 * @desc 打开描述符
	 **/
	virtual int open() = 0;

	/**
	 * @desc 关闭描述符
	 **/
	virtual void close() = 0;

	/**
 	 * @desc 启动任务
 	 **/
	int run(int& ec);

	/**
 	 * @desc 取消任务
 	 **/
	void cancel();

	/**
 	 * @desc 提交任务
 	 **/
	void post(post_handler& h);

	/**
 	 * @desc 获取描述符
 	 **/
	virtual handle get_descriptor() const;

	/**
 	 * @desc 获取任务队列
 	 **/
	virtual op_set_ptr& get_reactor_data();

	/**
 	 * @desc 获取前摄服务对象
 	 **/
	inline core_proactor_ptr get_service_impl() {
	  return proactor_;
	}

private:

	friend void task_create();
	friend void task_join();
	friend void task_start();
	friend void task_stop();

protected:
	static core_proactor_ptr proactor_;
	handle descriptor_;
	op_set_ptr descriptor_data_;
};

void task_start();

void task_stop();

class task_service_cleanup {
public:
	task_service_cleanup ();

	~task_service_cleanup();
};

}	// namespace zl_device_agent

#endif	// __deviceagent_object_base_h__
