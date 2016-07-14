#ifndef __DEVICEAGENT_EPOLL_H__
#define __DEVICEAGENT_EPOLL_H__
#pragma once
#include <stdint.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "../basedef.h"
#include "../common.h"
#include "operation.hpp"
#include "task_io_service.hpp"
// for debug
#include <iostream>

namespace zl_device_agent {
namespace async_io {

enum op_state {
	op_read = 0,
	op_write = 1,
	op_connect = 2,
	op_except = 3,
	op_max,
};

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
	: operation(ch), wh_(wh)
  {
  }

  op_poll(cbase* whobj, do_work_func whproc,
	  cbase* chobj, complete_func chproc)
	: operation(chobj, chproc)
  {
		wh_.object = whobj;
		wh_.proc = whproc;
  }

private:
  work_handler wh_;
};

template <typename config>
class epoller : cbase {
public:
  typedef epoller poll_type;
  typedef socket_type handle;
	typedef task_io_service<epoller, config> io_service;

  class op_set : public operation {
		friend class epoller;
		public:
		epoller* poller_;
		handle descriptor_;
		uint32_t resigter_events_;
		lib::deque<op_poll*> op_que_[op_max];
		bool shutdown_;
		bool destroy_;

		public:
		op_set()
			: operation(this,
					(complete_func) &op_set::do_complete_poll)
		{
		}

		//private:
		op_poll* do_work(uint32_t events) {
			std::cout << __func__ << " --  events: " << events << std::endl;
			lib::deque<op_poll*> clean_ops;
			static const uint32_t ev_flag[op_max]
				= {EPOLLIN, EPOLLOUT, EPOLLPRI};
			for (int j = op_max - 1; j >= 0; j--) {
				if ((ev_flag[j] | EPOLLERR | EPOLLHUP) & events) {
					std::cout << "op_que_[" << j << "].size(): " << op_que_[j].size() << std::endl;
					while (op_poll* op
							= op_que_[j].front()) {
						if (op->do_work()) {
							std::cout << "1111" << std::endl;
							op_que_->pop_front();
							clean_ops.push_back(op);
						} else {
							std::cout << "2222" << std::endl;
							break ;
						}
					}
				}
			}

			op_poll* first_op = clean_ops.front();
			if (first_op) {
				clean_ops.pop_front();
				// TODO call io_service completion
				// comletion(ops);
				// ....
			}

			return first_op;
		}

		void do_complete_poll(std::size_t bytes) {
			// TODO bytes_transfered
			uint32_t events = (uint32_t) bytes;
			if (op_poll* op = do_work(events)) {
				op->do_complete(0);
			} else {
				std::cout << __func__ << " -- empty op" << std::endl;
			}
		}

		void set_ready_event(uint32_t events) {
			task_result_ = events;
		}
  };

  epoller(io_service* io_service)
	: ep_handle_(do_create()), io_service_(io_service)
  {
  };

  ~epoller() {
  }

  int register_handle(handle descriptor, op_set& set) {
		// TODO lock
		set.poller_ = this;
		set.descriptor_ = descriptor;
		set.shutdown_ = false;
		epoll_event ev = {0, {0}};
		// edge-triggered
		ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLOUT
			| EPOLLPRI | EPOLLET;
		set.resigter_events_ = ev.events;
		ev.data.ptr = &set;
		int result = epoll_ctl(ep_handle_, EPOLL_CTL_ADD,
				descriptor, &ev);

		if (result != 0) {
			return errno;
		} else {
			return result;
		}
  }

  void deregister_handle(handle descriptor, op_set& set) {
		if (!set.shutdown_) {
			epoll_event ev = {0, {0}};
			epoll_ctl(ep_handle_, EPOLL_CTL_DEL, descriptor, &ev);

			// TODO op_set mutex
			lib::deque<op_poll*> ops;
			for (int i = 0; i < op_max; i++) {
				lib::deque<op_poll*>& que = set.op_que_[i];
				lib::deque<op_poll*>::iterator it;
				for (it = que.begin(); it != que.end(); it++) {
					ops.push_back(*it);
				}
			}
			set.descriptor_ = -1;
			set.shutdown_ = true;
			set.destroy_ = true;

#if 1
#else
			// TODO io_service completion
#endif
		}
  }

  void run(bool block, lib::deque<operation*>& ops) {
		// TODO epoll_wait
		epoll_event ev[128];
		// block 2 seconds on exit task
		int timeout = block ? -1 : 2;
		std::cout << __func__ << " -- timeout: " << timeout << std::endl;
		int ev_num = epoll_wait(ep_handle_, ev, 128, timeout);

		for (int i = 0; i < ev_num; i++) {
			void* ptr = ev[i].data.ptr;
			if (ptr) {
				op_set* set = static_cast<op_set*> (ptr);
				set->set_ready_event(ev[i].events);
				ops.push_back(set);
			}
		}
  }

	int start(int state, handle descriptor, op_set& set, op_poll* op) {
		if (set.destroy_) {
			return -1;
		}

		if (state >= op_max || op < 0) {
			return -1;
		}

		// TODO set mutex
		if (set.shutdown_) {
			io_service_->post_immediate_completion(op);
			return 0;
		}

		if (set.op_que_[state].empty()) {
			if (state != op_read
					|| set.op_que_[op_except].empty()) {
				if (op->do_work()) {
					// TODO unlock
					io_service_->post_immediate_completion(op);
					return 0;
				}

				if (op_write == state) {
					if ((set.resigter_events_ & EPOLLOUT) == 0) {
						epoll_event ev = {0, {0}};
						ev.events = set.resigter_events_ | EPOLLOUT;
						ev.data.ptr = &set;
						if (epoll_ctl(ep_handle_, EPOLL_CTL_MOD,
									descriptor, &ev) == 0) {
							set.resigter_events_ |= EPOLLOUT;
						} else {
							io_service_->post_immediate_completion(op);
							return -1;
						}
					}
				}
			} else {
				if (op_write == state) {
					set.resigter_events_ |= EPOLLOUT;
					epoll_event ev = {0, {0}};
					ev.events = set.resigter_events_ | EPOLLOUT;
					ev.data.ptr = &set;
					epoll_ctl(ep_handle_, EPOLL_CTL_MOD, descriptor, &ev);
				}
			}
		}
		set.op_que_[state].push_back(op);
		io_service_->work_started();
		return 0;
	}

	void interrupt() {
		epoll_event ev = {0, {0}};
		ev.events = EPOLLIN | EPOLLERR | EPOLLET;
		// TODO interrupter_
	}

protected:
  int do_create() {
		int fd = epoll_create(20000);
		// close on exec
		if (fd != -1) {
			::fcntl(fd, F_SETFD, FD_CLOEXEC);
		}

		return fd;
  }

private:
  int ep_handle_;
	io_service* io_service_;
};

/*
 template <typename config>
 class poller : public config::poll_type {
 public:
 typedef typename config::poll_type poll_type;
 typedef typename poll_type::handle handle;

 explicit poller() {
 }

 ~poller() {
 }
 };
*/

}	// namespace async_io
}	// namespace zl_device_agent
#endif	// __DEVICEAGENT_EPOLL_H__
