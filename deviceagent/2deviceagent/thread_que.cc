#include "thread_que.h"

namespace zl_device_agent {
thread_que* thread_que::instance_ = NULL;

void* thread_routine(void* args) {
	routine_handler* h = (routine_handler*) args;
	(((cbase*) h->object)->*(h->proc))(h->args);
	return NULL;
}

}	// namespace zl_device_agent
