#ifndef __OBJECT_BASE_H__
#define __OBJECT_BASE_H__

#include"../../basedef.h"
#include "../../config/core.h"
#include "../../io/async/proactor.hpp"
#include "./operation/stream_socket_write_op.hpp"

namespace zl_device_agent {

typedef stream_socket_write_op::write_handler write_handler;
typedef stream_socket_write_op::handle_write p_handle_write;

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
	 * @desc 设置写处理
	 **/
	virtual void set_write_handler(write_handler&);

#if 0
	/**
	 * @desc 设置读处理
	 **/
	virtual void set_read_handler();

	/**
	 * @desc 设置连接处理
	 **/
	virtual void set_connect_handler();

	/**
	 * @desc 设置失败处理
	 **/
	virtual void set_fail_handler();
#endif

	void run(int& ec);

	virtual void init();

#if 0
	virtual void destroy();
#endif

	virtual handle get_descriptor() const;

	virtual op_set_ptr& get_reactor_data();

protected:
	static core_proactor_ptr proactor_;
	handle descriptor_;
	op_set_ptr descriptor_data_;
	write_handler wh_;
};

}	// namespace zl_device_agent

#endif	// __object_base_h__
