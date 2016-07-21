#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__
#include <pthread.h>

namespace zl_device_agent {
template <typename Mutex>
class scoped_lock {
public:
	scoped_lock(Mutex& m)
		: mutex_(m)
	{
		mutex_.lock();
		locked_ = true;
	}

	~scoped_lock() {
		if (locked_) {
			mutex_.unlock();
		}
	}

	void lock() {
		if (!locked_) {
			mutex_.lock();
			locked_ = true;
		}
	}

	void unlock() {
		if (locked_) {
			mutex_.unlock();
			locked_ = false;
		}
	}

	bool locked() {
		return locked_;
	}

	Mutex& mutex() {
		return mutex_;
	}

private:
	Mutex& mutex_;
	bool locked_;
};

class posix_mutex {
public:
	typedef zl_device_agent::scoped_lock<posix_mutex> scoped_lock;
	posix_mutex() {
		::pthread_mutex_init(&mutex_, 0);
	}

	~posix_mutex() {
		::pthread_mutex_destroy(&mutex_);
	}

	void lock() {
		(void)::pthread_mutex_lock(&mutex_);
	}

	void unlock() {
		(void)::pthread_mutex_unlock(&mutex_);
	}

private:
	friend class posix_event;
	pthread_mutex_t mutex_;
};	// class posix_mutex
}	// namespace zl_device_agent

#endif	// __MUTEX_HPP__
