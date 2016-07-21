#ifndef __DEVICEAGENT_OPERATION_H__
#define __DEVICEAGENT_OPERATION_H__

#include "../../basedef.h"
#include "../../common.h"
#include "./task_io_service_decl.hpp"

namespace zl_device_agent {
// TODO with owner parameter
enum op_state {
	op_read = 0,
	op_write = 1,
	op_connect = 2,
	op_except = 3,
	op_max,
};

typedef void (cbase::*complete_func)(const int&, std::size_t, bool destroy);
struct complete_handler : public handler<complete_func> {
};

class operation : public cbase {
	template <typename reactor, typename config> friend class async_io::task_io_service;
public:
  operation() {
  	ch_.object = 0;
  	ch_.proc = 0;
  }

  operation(complete_handler& ch)
	: task_result_(0), ch_(ch)
  {
  }

	operation(cbase* object, complete_func proc) {
		ch_.object = object;
		ch_.proc = proc;
	}

	void do_complete(const int& ec, std::size_t bytes) {
		(ch_.object->*ch_.proc) (ec, bytes, false);
	}

	void destroy() {
		(ch_.object->*ch_.proc) (errno, 0, true);
	}

protected:
  unsigned int task_result_;

private:
  complete_handler ch_;
};
}	// namespace zl_device_agent

#endif
