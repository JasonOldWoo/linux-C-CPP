#ifndef __DEVICEAGENT_EPOLL_HPP__
#define __DEVICEAGENT_EPOLL_HPP__
#include <stdint.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <errno.h>
#include "task_io_service.hpp"
#include "../operation.hpp"
#include "../../basedef.h"
#include "../../common.h"
#include "../../utils/object_pool.hpp"
// for debug
#include <iostream>

namespace zl_device_agent {
namespace async_io {

typedef bool (cbase::*do_work_func)();
struct work_handler : public handler<do_work_func> {
};

class op_poll : public operation {
public:
	int ec_;

  bool do_work() {
		return (wh_.object->*wh_.proc) ();
  }

protected:
  op_poll(work_handler& wh, complete_handler& ch)
	: operation(ch), ec_(0), wh_(wh)
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
class epoller {
public:
	typedef typename config::mutex mutex;
	typedef typename config::mutex::scoped_lock scoped_lock;
	typedef task_io_service<epoller, config> io_service;

	typedef socket_type handle;
	typedef op_poll reactor_op;

	class select_interrupter {
	public:
		select_interrupter() {
			open_descriptor();
		}

		~select_interrupter() {
			close_descriptor();
		}

	private:
		void open_descriptor() {
			read_descriptor_ = ::eventfd(0, 0);
			write_descriptor_ = read_descriptor_;
			if (read_descriptor_ != -1) {
				::fcntl(read_descriptor_, F_SETFL, O_NONBLOCK);
				::fcntl(read_descriptor_, F_SETFD, FD_CLOEXEC);
			}

			if (-1 == read_descriptor_) {
				int pipe_fds[2] = {};
				if (::pipe(pipe_fds) == 0) {
					read_descriptor_ = pipe_fds[0];
					::fcntl(read_descriptor_, F_SETFL, O_NONBLOCK);
					::fcntl(read_descriptor_, F_SETFD, FD_CLOEXEC);
					write_descriptor_ = pipe_fds[1];
					::fcntl(write_descriptor_, F_SETFL, O_NONBLOCK);
					::fcntl(write_descriptor_, F_SETFD, FD_CLOEXEC);
				} else {
				}
			}
		}

		void close_descriptor() {
			if (write_descriptor_ != -1 && write_descriptor_ != read_descriptor_) {
				::close(write_descriptor_);
			}
			if (read_descriptor_ != -1) {
				::close(read_descriptor_);
			}
		}

	public:
		void recreate() {
			close_descriptor();
			write_descriptor_ = -1;
			read_descriptor_ = -1;
			open_descriptor();
		}

		void interrupt() {
			::uint64_t counter(1UL);
			int result = ::write(write_descriptor_, &counter, sizeof (::uint64_t));
			(void) result;
		}

		bool reset() {
			if (write_descriptor_ == read_descriptor_) {
				for (; ; ) {
					::uint64_t counter(0);
					int bytes_read = ::read(read_descriptor_, &counter, sizeof (::uint64_t));
					if (bytes_read < 0 && errno == EINTR) {
						continue ;
					}
					bool was_interrupted = (bytes_read > 0);
					return was_interrupted;
				}
			} else {
				for (; ; ) {
					char data[1024] = {};
					int bytes_read = ::read(read_descriptor_, data, sizeof (data));
					if (bytes_read < 0 && errno == EINTR) {
						continue ;
					}
					bool was_interrupted = (bytes_read > 0);
					while (bytes_read == sizeof (data)) {
						bytes_read = ::read(read_descriptor_, data, sizeof (data));
					}
					return was_interrupted;
				}
			}
		}

		int read_descriptor() const {
			return read_descriptor_;
		}

	private:
		int read_descriptor_;
		int write_descriptor_;
	};

	friend struct perform_io_cleanup_on_block_exit;

	struct perform_io_cleanup_on_block_exit {
		explicit perform_io_cleanup_on_block_exit(epoller* poller)
			: poller_(poller), first_op_(0)
		{
		}

		~perform_io_cleanup_on_block_exit() {
			if (first_op_) {
				if (!ops_.empty()) {
					poller_->io_service_->post_deferred_completions(ops_);
				}
			} else {
				poller_->io_service_->work_started();
			}
		}

		epoller* poller_;
		lib::list<operation*> ops_;
		operation* first_op_;
	};

	class op_set : public operation {
		friend class epoller;
		friend class object_pool_access;
		template<typename _Object> friend _Object*& object_pool_access::next(_Object*);
		template<typename _Object> friend _Object*& object_pool_access::prev(_Object*);
		template<typename _Object> friend _Object* object_pool_access::create();

		op_set* next_;
		op_set* prev_;

		mutex mutex_;
		epoller* poller_;
		socket_type descriptor_;
		::uint32_t register_events_;
		lib::list<op_poll*> op_que_[op_max];
		bool shutdown_;

		op_set()
			: operation(this,
					(complete_func) &op_set::do_complete)
		{
		}

		void set_ready_event(::uint32_t events) {task_result_ = events;}
		operation* perform_io(::uint32_t events) {
			perform_io_cleanup_on_block_exit io_cleanup(poller_);
			scoped_lock lock(mutex_);

			static const int flag[op_max] = {EPOLLIN, EPOLLOUT, EPOLLPRI};
			for (int j = op_max - 1; j >= 0; j--) {
				if (events & (flag[j] | EPOLLERR | EPOLLHUP)) {
					op_poll* op = 0;
					while ((op = op_que_[j].front()) && op_que_[j].size()) {
						if (op->do_work()) {
							op_que_[j].pop_front();
							io_cleanup.ops_.push_back(op);
						} else {
							break ;
						}
					}
				}
			}
			if (io_cleanup.ops_.size()) {
				io_cleanup.first_op_ = io_cleanup.ops_.front();
				io_cleanup.ops_.pop_front();
				return io_cleanup.first_op_;
			} else {
				return 0;
			}
		}

		void do_complete(const int& ec, std::size_t bytes_transferred) {
			::uint32_t events = static_cast<uint32_t> (bytes_transferred);
			if (operation* op = perform_io(events)) {
				//std::cout << "after perform io op: " << op << std::endl;
				op->do_complete(ec, 0);
			}
		}
	};	// class op_set

	epoller(io_service* serv)
		: io_service_(serv),
		mutex_(),
		interrupter_(),
		epoll_fd_(do_epoll_create()),
		shutdown_(false)
	{
		operation op;
	}

	~epoller () {
		if (epoll_fd_ != -1) {
			close(epoll_fd_);
		}
	}

	void shutdown_service() {
		scoped_lock lock(mutex_);
		shutdown_ = true;
		lock.unlock();

		lib::list<operation*> ops;

		while (op_set* set = registered_descriptors_.first()) {
			for (int i = 0; i < op_max; ++i) {
				ops.merge(set->op_que_[i]);
			}
			set->shutdown_ = true;
			registered_descriptors_.free(set);
		}

		io_service_->abandon_operations(ops);
	}

	int register_descriptor(handle descriptor, op_set*& set) {
		set = allocate_op_set();
		{
			scoped_lock lock(set->mutex_);
			set->poller_ = this;
			set->descriptor_ = descriptor;
			set->shutdown_ = false;
		}

		epoll_event ev = {0, {0}};
		ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLPRI | EPOLLET;
		set->register_events_ = ev.events;
		ev.data.ptr = set;

		int result = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, descriptor, &ev);
		if (result != 0) {
			return errno;
		}
		return 0;
	}

	void start_op(int op_type, handle descriptor, op_set*& set, op_poll* op, bool allow_speculative) {
		if (!set) {
			op->ec_ = EBADF;
			io_service_->post_immediate_completion(op);
			return ;
		}

		scoped_lock lock(set->mutex_);

		if (set->shutdown_) {
			io_service_->post_immediate_completion(op);
			return ;
		}

		if (set->op_que_[op_type].empty()) {
			if (allow_speculative && (op_type != op_read
				|| set->op_que_[op_except].empty())) {
				if (op->do_work()) {
					lock.unlock();
					io_service_->post_immediate_completion(op);
					return ;
				}

				if (op_write == op_type) {
					if ((set->register_events_ & EPOLLOUT) == 0) {
						epoll_event ev = {0, {0}};
						ev.events = set->register_events_ | EPOLLOUT;
						ev.data.ptr = set;
						if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, descriptor, &ev) == 0) {
							set->register_events_ |= ev.events;
						} else {
							op->ec_ = errno;
							io_service_->post_immediate_completion(op);
							return ;
						}
					}
				}
			}
			else {
				if (op_write == op_type) {
					set->register_events_ |= EPOLLOUT;
				}

				epoll_event ev = {0, {0}};
				ev.events = set->register_events_;
				ev.data.ptr = set;
				epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, descriptor, &ev);
			}
		}

		set->op_que_[op_type].push_back(op);
		io_service_->work_started();
	}

	void cancel_ops(handle, op_set*& set) {
		if (!set) {
			return ;
		}

		scoped_lock lock(set->mutex_);

		lib::list<operation*> ops;
		for (int i = 0; i < op_max; i++) {
			while (op_poll* op = set->op_que_[i].front() && set->op_que_[i].size()) {
				op->ec_ = ECANCELED;
				set->op_que_[i].pop_front();
				ops.push_back(op);
			}
		}

		lock.unlock();
		io_service_->post_deferred_completions(ops);
	}

	void deregister_descriptor(handle descriptor, op_set*& set, bool closing) {
		if (!set) {
			return ;
		}

		scoped_lock lock(set->mutex_);

		if (!set->shutdown_) {
			if (closing) {
			} else {
				epoll_event ev = {0, {0}};
				epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, descriptor, &ev);
			}

			lib::list<operation*> ops;
			for (int i = 0; i < op_max; ++i) {
				op_poll* op = 0;
				while ((op = set->op_que_[i].front()) && set->op_que_[i].size()) {
					op->ec_ = ECANCELED;
					set->op_que_[i].pop_front();
					ops.push_back(op);
				}
			}

			set->descriptor_ = -1;
			set->shutdown_ = true;

			lock.unlock();

			free_op_set(set);
			set = 0;

			io_service_->post_deferred_completions(ops);
		}
	}

	void run(bool block, lib::list<operation*>& ops) {
		int timeout = block ? 5 * 60 * 1000 : 0;
		//std::cout << __func__ << " -- timeout: " << timeout << std::endl;
		epoll_event evs[128];
		int num_evs = epoll_wait(epoll_fd_, evs, 128, timeout);
		//std::cout << __func__ << " -- num_evs: " << num_evs << std::endl;

		for (int i = 0; i < num_evs; i++) {
			void* ptr = evs[i].data.ptr;
			if (ptr == &interrupter_) {
			} else {
				//std::cout << __func__ << " -- " << i << " events: " << evs[i].events << std::endl;
				assert(ptr);
				op_set* set = static_cast<op_set*> (ptr);
				set->set_ready_event(evs[i].events);
				ops.push_back(set);
			}
		}
	}

	void interrupt() {
		epoll_event ev = {0, {0}};
		ev.events = EPOLLIN | EPOLLERR | EPOLLET;
		ev.data.ptr = &interrupter_;
		epoll_ctl(epoll_fd_, EPOLL_CTL_MOD,
			interrupter_.read_descriptor(), &ev);
	}

	int do_epoll_create() {
		int fd = -1;

		fd = epoll_create(20000);
		if (fd != -1) {
			// no clone
			::fcntl(fd, F_SETFD, FD_CLOEXEC);
		}

		//assert(fd);

		return fd;
	}

	op_set* allocate_op_set() {
		scoped_lock lock(registered_descriptor_mutex_);
		return registered_descriptors_.alloc();
	}

	void free_op_set(op_set* set) {
		scoped_lock lock(registered_descriptor_mutex_);
		registered_descriptors_.free(set);
	}

private:
	io_service* io_service_;
	mutex mutex_;
	select_interrupter interrupter_;
	int epoll_fd_;
	bool shutdown_;
	mutex registered_descriptor_mutex_;
	object_pool<op_set> registered_descriptors_;
};

}	// namespace async_io
}	// namespace zl_device_agent
#endif	// __DEVICEAGENT_EPOLL_H__
