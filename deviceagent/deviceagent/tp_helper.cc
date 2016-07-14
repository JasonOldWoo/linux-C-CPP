#include "tp_helper.h"

namespace zl_device_agent {
tp_helper* tp_helper::instance_ = NULL;

void* tp_routine(void* args) {
	tp_handler* h = (tp_handler*) args;
	(((cbase*) h->object)->*(h->proc))(h->args);
	return NULL;
}

}	// namespace zl_device_agent
