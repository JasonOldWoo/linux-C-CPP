#ifndef __DEVICEAGENT_THREAD_QUE_H__
#define __DEVICEAGENT_THREAD_QUE_H__

#include "libmhthread/thread_pool.h"
#include "basedef.h"

namespace zl_device_agent {

typedef void* (cbase::*routine) (void*);
struct routine_handler : public handler<routine> {
	void* args;
};

#define MAX_DEVICEAGENT_THREADS 5

void* thread_routine(void* args);

class thread_que {
public:
	static thread_que* instance() {
		if (NULL == instance_) {
			instance_ = new thread_que(MAX_DEVICEAGENT_THREADS);
		}

		return instance_;
	}

	explicit thread_que(int max_con) {
		thpool_init(max_con);
	}

	~thread_que() {
		thpool_destroy();
	}

	void destroy() {
		instance_->~thread_que();
		::operator delete (instance_);
	}

	void add_worker(routine_handler* args) {
		thpool_add_worker(thread_routine, (void*) args);
	}

private:
	static thread_que* instance_;
};

}	// namespace zl_device_agent

#define g_thread_que (*zl_device_agent::thread_que::instance())

#endif	// __deviceagent_thread_que_h__
