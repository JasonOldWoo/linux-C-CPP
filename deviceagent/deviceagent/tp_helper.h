#ifndef __TP_HELPER_H__
#define __TP_HELPER_H__

#include "thread_pool_lite/thread_pool.h"
#include "basedef.h"

namespace zl_device_agent {
typedef void* (cbase::*tp_proc) (void*);
struct tp_handler : public handler<tp_proc> {
	void* args;
};

#define MAX_DEVICEAGENT_THREADS 5

void* tp_routine(void* args);

class tp_helper {
public:
	static tp_helper* instance() {
		if (NULL == instance_) {
			instance_ = new tp_helper(MAX_DEVICEAGENT_THREADS);
		}

		return instance_;
	}

	explicit tp_helper(int max_con) {
		thpool_init(max_con);
	}

	~tp_helper() {
		thpool_destroy();
	}

	void add_worker(tp_handler* args) {
		thpool_add_worker(tp_routine, (void*) args);
	}

private:
	static tp_helper* instance_;
};

#define g_thpool_helper (*tp_helper::instance())
}	// namespace zl_device_agent

#endif
