#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <pthread.h>

namespace zl_device_agent {
class posix_event {
public:
	posix_event() {
		::pthread_cond_init(&cond_, 0);
	}

	~posix_event() {
		::pthread_cond_destroy(&cond_);
	}

	template <typename Lock>
	void signal(Lock& lock) {
		lock.locked();
		(void)lock;
		signalled_ = true;
		::pthread_cond_signal(&cond_);
	}

	template <typename Lock>
	void signal_and_unlock(Lock& lock) {
		lock.locked();
		signalled_ = true;
		lock.unlock();
		::pthread_cond_signal(&cond_);
	}

	template <typename Lock>
	void clear(Lock& lock) {
		lock.locked();
		(void)lock;
		signalled_ = false;
	}

	template <typename Lock>
	void wait(Lock& lock) {
		lock.locked();
		while (!signalled_) {
			::pthread_cond_wait(&cond_, &lock.mutex().mutex_);
		}
	}

private:
	pthread_cond_t cond_;
	bool signalled_;
};
}	// namespace zl_device_agent

#endif	// __event_hpp__
