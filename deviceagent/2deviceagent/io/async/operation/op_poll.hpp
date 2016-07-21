#ifndef __DEVICEAGENT_OP_POLL_HPP__
#define  __DEVICEAGENT_OP_POLL_HPP__

#include "../operation.hpp"

namespace zl_device_agent {
namespace async_io {

typedef bool (cbase::*do_work_func)();
struct work_handler : public handler<do_work_func> {
};

class op_poll : public operation {
public:
  bool do_work() {
		return (wh_.object->*wh_.proc) ();
  }

protected:
  op_poll(work_handler& wh, complete_handler& ch)
		: operation(ch), ec_(0), bytes_trans_(0), wh_(wh)
  {
  }

  op_poll(cbase* whobj, do_work_func whproc,
		cbase* chobj, complete_func chproc)
		: operation(chobj, chproc)
  {
		wh_.object = whobj;
		wh_.proc = whproc;
  }

public:
  int ec_;
	std::size_t bytes_trans_;

private:
  work_handler wh_;
};

}	// namespace async_io
}	// namespace zl_device_agent

#endif
