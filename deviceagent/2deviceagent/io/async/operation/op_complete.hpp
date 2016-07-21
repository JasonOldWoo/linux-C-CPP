#ifndef __DEVICEAGENT_OP_COMPLETE_HPP__
#define __DEVICEAGENT_OP_COMPLETE_HPP__
#include "../operation.hpp"

namespace zl_device_agent {
namespace async_io {

typedef void (cbase::*handle_post)(void*);
struct post_handler : public handler<handle_post> {
	void* args;
};

class op_complete : public operation {
public:
	op_complete(post_handler& h)
		: operation(static_cast<cbase*>(this),
			(complete_func) &op_complete::do_callback),
		h_(h)
	{}

	void do_callback(const int&, std::size_t&, bool destroy) {
		post_handler h = h_;
		op_complete* p = this;
		p->~op_complete();
		::operator delete(p);
		if (!destroy) {
			(h.object->*h_.proc)(h.args);
		}
	}

private:
	post_handler h_;
};

}	// namespace async_io
}	// namespace zl_device_agent
#endif	// __deviceagent_op_complete_hpp__
