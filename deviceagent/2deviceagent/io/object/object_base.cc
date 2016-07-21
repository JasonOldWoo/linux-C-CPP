#include "object_base.h"

namespace zl_device_agent {

void task_create() {
	if (!object_base::proactor_) {
		object_base::proactor_ = new core_proactor;
		object_base::proactor_->init();
	}
}

void task_join() {
	if (object_base::proactor_) {
		object_base::proactor_->stop();
		object_base::proactor_->shutdown_service();
		delete object_base::proactor_;
		object_base::proactor_ = 0;
	}
}

void task_start() {
	object_base::proactor_->reset();
}

void task_stop() {
	object_base::proactor_->stop();
}

task_service_cleanup::task_service_cleanup() {
	task_create();
}

task_service_cleanup::~task_service_cleanup() {
	task_join();
}

core_proactor_ptr object_base::proactor_ = 0;

object_base::object_base()
	: descriptor_(invalid_socket)
{
}

object_base::~object_base() {
}

handle object_base::get_descriptor() const {
	return descriptor_;
}

op_set_ptr& object_base::get_reactor_data() {
	return descriptor_data_;
}

int object_base::run(int& ec) {
	return proactor_->run(ec);
}

void object_base::cancel() {
	proactor_->cancel_ops(*this);
}

void object_base::post(post_handler& h) {
	proactor_->post(h);
}

}	// namespace zl_device_agent
