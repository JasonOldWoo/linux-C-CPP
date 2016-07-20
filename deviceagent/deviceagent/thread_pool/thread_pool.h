#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

/********************************************************/
/* for more user cases, please check out my github:	*/
/* https://github.com/JasonOldWoo/linux-C-CPP		*/
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus
// worker is a task which could be asleep or
// on wokring in the thread pool.
typedef struct _thread_worker
{
	// call back function who is doing the real job.
	void* 	(*function) (void* arg);
	void* 	arg;
	struct 	_thread_worker *next;
} thread_worker;

// thread pool struct
typedef struct _thread_pool
{
	pthread_mutex_t	queue_lock;
	pthread_cond_t	queue_ready;

	// workers's head node.
	thread_worker	*queue_head;

	int 		is_shutdown;
	pthread_t*	threadp;
	// allowed threads number.
	int		max_thread_num;
	// number of active work.
	int		curr_queue_size;
} thread_pool;

void thpool_init(int max_thread_num);

int thpool_add_worker(void* (*function) (void* arg), void* arg);

int thpool_destroy();

void* thread_routine(void* arg);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif	// __thread_pool_h__
