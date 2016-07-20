#include "../mutex.hpp"
#include <unistd.h>
#include <iostream>

zl_device_agent::posix_mutex g_mutex;

void* routine_1(void* args) {
	zl_device_agent::posix_mutex::scoped_lock lock(g_mutex);
	//g_mutex.lock();
	std::cout << "tid: " << ::pthread_self() << ", routine_1 enter" << std::endl;
	sleep(5);
	//g_mutex.unlock();
	std::cout << "tid: " << ::pthread_self() << ", routine_1 exit" << std::endl;
	return 0;
}

void* routine_2(void* args) {
	zl_device_agent::posix_mutex::scoped_lock lock(g_mutex);
	//g_mutex.lock();
	std::cout << "tid: " << ::pthread_self() << ", routine_2 enter" << std::endl;
	sleep(5);
	//g_mutex.unlock();
	std::cout << "tid: " << ::pthread_self() << ", routine_2 exit" << std::endl;
	return 0;
}

int main() {
	pthread_t t1;
	pthread_t t2;
	::pthread_create(&t1, NULL, routine_1, NULL);
	sleep(1);
	::pthread_create(&t2, NULL, routine_2, NULL);

	void* ret;

	::pthread_join(t1, &ret);
	::pthread_join(t2, &ret);

	return 0;
}
