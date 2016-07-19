#include "object_base.h"

namespace zl_device_agent {

core_proactor_ptr object_base::proactor_ = 0;

object_base::object_base()
	: descriptor_(invalid_socket)
{
}

object_base::~object_base() {
}

void object_base::init() {
	if (!proactor_) {
		proactor_ = new core_proactor;
		proactor_->init();
	}
}

#if 0
void object_base::destroy() {
	if (proactor_) {
		delete proactor_;
	}
}
#endif

void object_base::set_write_handler(write_handler& wh) {
	wh_ = wh;
}

handle object_base::get_descriptor() const {
	return descriptor_;
}

op_set_ptr& object_base::get_reactor_data() {
	return descriptor_data_;
}

void object_base::run(int& ec) {
	proactor_->run(ec);
}

}	// namespace zl_device_agent
