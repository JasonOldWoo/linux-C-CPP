#ifndef __DEVICEAGENT_TASK_IO_SERVICE_HPP__
#define __DEVICEAGENT_TASK_IO_SERVICE_HPP__

#include <limits>
#include <climits>
#include <list>

#include "../operation.hpp"
#include "../../utils/mutex.hpp"
#include "../../utils/event.hpp"

namespace zl_device_agent {
namespace async_io {

class thread_info_base {
public:
	thread_info_base()
		: reusable_memory_(0)
	{}

	~thread_info_base()
	{}

	static void* allocate(thread_info_base* this_thread, std::size_t size) {
		if (this_thread && this_thread->reusable_memory_) {
			void* const pointer = this_thread->reusable_memory_;
			this_thread->reusable_memory_ = 0;

			unsigned char* const mem = static_cast<unsigned char*> (pointer);
			if (static_cast<std::size_t> (mem[0]) >= size) {
				mem[size] = mem[0];
				return pointer;
			}

			::operator delete (pointer);
		}

		void* const pointer = ::operator new (size + 1);
		unsigned char* const mem = static_cast<unsigned char*> (pointer);
		mem[size] = (size <= UCHAR_MAX) ? static_cast<unsigned char> (size) : 0;
		return pointer;
	}

	static void deallocate(thread_info_base* this_thread, void* pointer,
		std::size_t size) {
		if (size <= UCHAR_MAX) {
			if (this_thread && 0 == this_thread->reusable_memory_) {
				unsigned char* const mem = static_cast<unsigned char*> (pointer);
				mem[0] = mem[size];
				this_thread->reusable_memory_ = pointer;
				return ;
			}
		}

		::operator delete (pointer);
	}
private:
	void* reusable_memory_;
};



struct task_io_service_thread_info : public thread_info_base {
	posix_event* wakeup_event;
	std::list<operation*> private_op_que;
	long private_outstanding_work;
	task_io_service_thread_info* next;
};

template <typename _Key, typename _Value = unsigned char>
class call_stack {
public:
	class context {
	public:
		explicit context(_Key* k)
			: key_(k),
			next_(call_stack<_Key, _Value>::top_)
		{
			value_ = reinterpret_cast<unsigned char*> (this);
			call_stack<_Key, _Value>::top_ = this;
		}

		context(_Key* k, _Value& v)
			: key_(k),
			value_(&v),
			next_(call_stack<_Key, _Value>::top_)
		{
			call_stack<_Key, _Value>::top_ = this;
		}

		~context() {
			call_stack<_Key, _Value>::top_ = next_;
		}

		_Value* next_by_key() const {
			context* elem = next_;
			while (elem) {
				if (elem->key_ == key_) {
					return elem->value_;
				}
				elem = elem->next_;
			}
			return 0;
		}

	private:
		friend class call_stack<_Key, _Value>;
		_Key* key_;
		_Value* value_;
		context* next_;
	};

	friend class context;

	static _Value* contains(_Key* k) {
		context* elem = top_;
		while (elem) {
			if (elem->key_ == k) {
				return elem->value_;
			}
			elem = elem->next_;
		}
		return 0;
	}

	static _Value* top() {
		context* elem = top_;
		return elem ? elem->value_ : 0;
	}

private:

	static __thread context* top_;
};

template<typename _Key, typename _Value>
__thread typename call_stack<_Key, _Value>::context*
call_stack<_Key, _Value>::top_;

template <typename reactor, typename config>
class task_io_service : public cbase {

	typedef typename config::mutex mutex;
	typedef typename config::mutex::scoped_lock scoped_lock;

	friend struct task_cleanup;
	friend struct work_cleanup;
	struct task_cleanup {
		~task_cleanup() {
			if (this_thread_->private_outstanding_work > 0) {
				long b = this_thread_->private_outstanding_work;
				while (b > 0) {
					++task_io_service_->outstanding_work_;
					--b;
				}
			}
			this_thread_->private_outstanding_work = 0;

			lock_->lock();
			task_io_service_->task_interrupted_ = true;
			task_io_service_->op_que_.merge(this_thread_->private_op_que);
			task_io_service_->op_que_.push_back(&task_io_service_->task_operation_);
		}

		task_io_service* task_io_service_;
		scoped_lock* lock_;
		task_io_service_thread_info* this_thread_;
	};

struct work_cleanup {
	~work_cleanup() {
		if (this_thread_->private_outstanding_work > 1) {
			long b = this_thread_->private_outstanding_work - 1;
			while (b > 0) {
				++task_io_service_->outstanding_work_;
				--b;
			}
		} else if (this_thread_->private_outstanding_work < 1) {
			task_io_service_->work_finished();
		}
		this_thread_->private_outstanding_work = 0;

		// threads feature
		if (!this_thread_->private_op_que.empty()) {
			lock_->lock();
			task_io_service_->op_que_.merge(this_thread_->private_op_que);
		}
	}

	task_io_service* task_io_service_;
	scoped_lock* lock_;
	task_io_service_thread_info* this_thread_;
};


public:
	task_io_service(std::size_t concurrency_hint)
		:one_thread_(concurrency_hint == 1),
		mutex_(),
		poller_(),
		task_interrupted_(true),
		outstanding_work_(0),
		stopped_(false),
		shutdown_(false),
		first_idle_thread_(0)
	{
	}

	~task_io_service() {
		if (poller_) {
			delete poller_;
		}
	}

	reactor* get_reactor() {
		return poller_;
	}

	void shutdown_service() {
		scoped_lock lock(mutex_);
		shutdown_ = true;
		lock.unlock();

		while (!op_que_.empty()) {
			operation* op = op_que_.front();
			op_que_.pop_front();
			if (op != &task_operation_) {
				op->destroy();
			}
		}

		poller_ = 0;
	}

	void init_task() {
		scoped_lock lock(mutex_);
		if (!shutdown_ && !poller_) {
			poller_ = new reactor(this);
			op_que_.push_back(&task_operation_);
			// unlock here
			wake_one_thread_and_unlock(lock);
		}
	}

	std::size_t run(int& ec) {
		ec = errno;
		if (0 == outstanding_work_) {
			stop();
			return 0;
		}

		task_io_service_thread_info this_thread;
		posix_event wakeup_event;
		this_thread.wakeup_event = &wakeup_event;
		this_thread.private_outstanding_work = 0;
		this_thread.next = 0;
		typename thread_call_stack::context ctx(this, this_thread);

		scoped_lock lock(mutex_);

		std::size_t n = 0;
		for (; do_run_one(lock, this_thread, ec); lock.lock()) {
			if (n != (std::numeric_limits<std::size_t>::max())) {
				++n;
			}
		}
		return n;
	}

	void stop() {
		scoped_lock lock(mutex_);
		stop_all_threads(lock);
	}

	bool stopped() {
		scoped_lock lock(mutex_);
		return stopped_;
	}

	void reset() {
		scoped_lock lock(mutex_);
		stopped_ = false;
	}

	void work_started() {
		++outstanding_work_;
	}

	void work_finished() {
		if (0 == --outstanding_work_) {
			stop();
		}
	}

	void post_immediate_completion(operation* op) {
		if (one_thread_) {
			if (task_io_service_thread_info* this_thread = thread_call_stack::contains(this)) {
				++this_thread->private_outstanding_work;
				this_thread->private_op_que.push_back(op);
				return ;
			}
		}

		work_started();
		scoped_lock lock(mutex_);
		//std::cout << __func__ << " -- op_que_ push op" << std::endl;
		//std::cout << "op: " << op << std::endl;
		op_que_.push_back(op);
		//std::cout << "op size: " << op_que_.size() << std::endl;
		wake_one_thread_and_unlock(lock);
	}

	void post_deferred_completion(operation* op) {
		if (one_thread_) {
			if (task_io_service_thread_info* this_thread = thread_call_stack::contains(this)) {
				this_thread->private_op_que.push_back(op);
				return ;
			}
		}

		scoped_lock lock(mutex_);
		op_que_.push_back(op);
		wake_one_thread_and_unlock(lock);
	}

	void post_deferred_completions(std::list<operation*>& ops) {
		if (!ops.empty()) {
			if (one_thread_) {
				if (task_io_service_thread_info* this_thread = thread_call_stack::contains(this)) {
					this_thread->private_op_que.merge(ops);
					return ;
				}
			}
			scoped_lock lock(mutex_);
			op_que_.merge(ops);
			wake_one_thread_and_unlock(lock);
		}
	}

	void abandon_operations(std::list<operation*>& ops) {
		std::list<operation*> ops2;
		ops2.merge(ops);
		while (!ops2.empty()) {
			operation* op = ops2.back();
			op->destroy();
			ops2.pop_back();
		}
	}

private:
	std::size_t do_run_one(scoped_lock& lock,
			task_io_service_thread_info& this_thread, int& ec) {
		while (!stopped_) {
			if (!op_que_.empty()) {
				operation* o = op_que_.front();
				op_que_.pop_front();
				bool more_handlers = (!op_que_.empty());

				if (o == &task_operation_) {
					task_interrupted_ = more_handlers;
					if (more_handlers && !one_thread_) {
						if (!wake_one_idle_thread_and_unlock(lock)) {
							lock.unlock();
						}
					} else {
						lock.unlock();
					}
					task_cleanup on_exit = {this, &lock, &this_thread};
					(void)on_exit;

					poller_->run(!more_handlers, this_thread.private_op_que);
				} else {
					//std::cout << __func__ << " -- o: " << o << std::endl;
					std::size_t task_result = o->task_result_;

					if (more_handlers && !one_thread_) {
						wake_one_thread_and_unlock(lock);
					} else {
						lock.unlock();
					}

					work_cleanup on_exit = {this, &lock, &this_thread};
					(void)on_exit;

					o->do_complete(ec, task_result);
					return 1;
				}
			} else {
				// nothing to run
				this_thread.next = first_idle_thread_;
				first_idle_thread_ = &this_thread;
				this_thread.wakeup_event->clear(lock);
				this_thread.wakeup_event->wait(lock);
			}
		}

		return 0;
	}

	void stop_all_threads(scoped_lock& lock) {
		stopped_ = true;

		while (first_idle_thread_) {
			task_io_service_thread_info* idle_thread = first_idle_thread_;
			first_idle_thread_ = idle_thread->next;
			idle_thread->next = 0;
			idle_thread->wakeup_event->signal(lock);
		}

		if (!task_interrupted_ && poller_) {
			task_interrupted_ = true;
			// incomplete
			poller_->interrupt();
		}
	}

	void wake_one_thread_and_unlock(scoped_lock& lock) {
		if (!wake_one_idle_thread_and_unlock(lock)) {
			if (!task_interrupted_ && poller_) {
				task_interrupted_ = true;
				// incomplete
				poller_->interrupt();
			}
			lock.unlock();
		}
	}

	bool wake_one_idle_thread_and_unlock(scoped_lock& lock) {
		if (first_idle_thread_) {
			task_io_service_thread_info* idle_thread = first_idle_thread_;
			first_idle_thread_ = idle_thread->next;
		  idle_thread->next = 0;
			idle_thread->wakeup_event->signal_and_unlock(lock);
			return true;
		}
		return false;
	}

#if 0
private:
	template<typename _Que>
	void move_que(_Que& q1, _Que& q2) {
		typedef typename _Que::iterator _Iter;
		_Iter it;
		for (it = q2.begin(); it != q2.end(); it++) {
			q1.push_back(*it);
		}
		q2.clear();
	}
#endif

private:
	const bool one_thread_;
	mutex mutex_;
	reactor* poller_;
	struct task_operation : operation {
		task_operation() {}
	} task_operation_;
	bool task_interrupted_;
	// TODO atomic
	int outstanding_work_;
	std::list<operation*> op_que_;
	bool stopped_;
	bool shutdown_;
	typedef call_stack<task_io_service, task_io_service_thread_info> thread_call_stack;
	task_io_service_thread_info* first_idle_thread_;
};	// class task_io_service
}	// namespace async
}	// namespace zl_device_agent

#endif	// __task_io_service_h__
