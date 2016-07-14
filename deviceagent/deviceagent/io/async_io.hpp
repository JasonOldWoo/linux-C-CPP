#ifndef __DEVICEAGENT_ASYNC_IO_H__
#define __DEVICEAGENT_ASYNC_IO_H__

#pragma once

#include "../common/thread.h"
//#include "epoll.hpp"
#include "../common.h"
#include "operation.hpp"

namespace zl_device_agent {
namespace async_io {

template <typename config>
class io {
public:
	typedef typename config::poll::poll_type poll_type;
	typedef typename poll_type::handle handle;

	explicit io()
		: stopped_(false),
		shutdown_(false)
	{
	}

	~io() {
	}

	std::size_t run() {
		lib::deque<operation*> priv_op_que;
		while (1) {
			if (!do_run_it(priv_op_que)) {
				break ;
			}
		}
	}

protected:
	std::size_t do_run_it(lib::deque<operation*> priv_op_que) {
		while (!stopped_) {
			if (!op_que_.empty()) {
				operation* o = op_que_.front();
				op_que_.pop_front();
				if (o == &task_op_) {
					// TODO taskclean
					poller_.run(priv_op_que);
				}
			}
		}
	}

private:
	poll_type poller_;
	bool stopped_;
	bool shutdown_;
	lib::deque<operation*> op_que_;
	struct task_op : public operation {
		task_op() : operation() {
		}
	} task_op_;
};

}	// zl_device_agent
}	// async_io

#endif	// __DEVICEAGENT_ASYNC_IO_H__
