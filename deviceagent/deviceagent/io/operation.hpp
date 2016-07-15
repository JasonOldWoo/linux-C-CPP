#ifndef __DEVICEAGENT_OPERATION_H__
#define __DEVICEAGENT_OPERATION_H__

#include "../basedef.h"
#include "../common.h"
// for debug
#include <iostream>
#include <stdio.h>
#include <assert.h>

namespace zl_device_agent {
  // TODO with owner parameter
  typedef void (cbase::*complete_func)(const int&, std::size_t);
  struct complete_handler : public handler<complete_func> {
  };

  class operation : public cbase{
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
			assert(ch_.object && ch_.proc);
			(ch_.object->*ch_.proc) (ec, bytes);
		}

		void destroy() {
			assert(ch_.object && ch_.proc);
			(ch_.object->*ch_.proc) (errno, 0);
		}

	  //protected:
	  unsigned int task_result_;

	private:
	  complete_handler ch_;
  };
}	// namespace zl_device_agent

#endif
