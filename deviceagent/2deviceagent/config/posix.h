#ifndef __DEVICEAGENT_POSIX_H__
#define __DEVICEAGENT_POSIX_H__

#include "../utils/mutex.hpp"
#include "../io/async/epoll.hpp"
#include "../io/async/task_io_service.hpp"

namespace zl_device_agent {

class object_base;

typedef struct posix_config {
  struct util_config {
	typedef posix_mutex mutex;
	typedef mutex::scoped_lock lock;
  };
  typedef zl_device_agent::async_io::epoller<util_config> poll_type;
  typedef zl_device_agent::async_io::task_io_service<poll_type, util_config> service_type;
	typedef zl_device_agent::async_io::op_poll op_poll;
	typedef zl_device_agent::async_io::op_complete op_complete;
	typedef zl_device_agent::async_io::post_handler post_handler;
	typedef zl_device_agent::async_io::handle_post handle_post;
  typedef object_base object_impl;
} core_config;

typedef core_config::poll_type poll_type;

typedef poll_type::handle handle;
typedef poll_type::op_set op_set;

typedef async_io::op_poll op_poll;
typedef async_io::op_complete op_complete;
typedef async_io::work_handler work_handler;
typedef async_io::do_work_func do_work_func;
typedef async_io::post_handler post_handler;
typedef async_io::handle_post handle_post;

typedef op_poll* op_poll_ptr;
typedef op_complete* op_complete_ptr;
typedef op_set* op_set_ptr;

}	// namespace zl_device_agent

#endif
