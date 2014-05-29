#ifndef __THREAD_H__
#define __THREAD_H__

#include "global.h"


typedef void* (*FUNC)(void *arg);


typedef struct _thpool_job_t
{
	FUNC			function;
	void*			arg;
	struct _thpool_job_t*	prev;
	struct _thpool_job_t*	next;
} thpool_job_t;

// job queue
typedef struct _thpool_job_queue
{
	thpool_job_t*	head;
	thpool_job_t*	tail;
	int		jobN;	// count of jobs
	sem_t*		queueSem;	// xsem
}thpool_jobqueue;

// thread poool
typedef struct _thpool_t
{
	pthread_t*		threads;
	int			threadsN;	// count of threads
	thpool_jobqueue*	jobqueue;
}thpool_t;

// init count of threads
thpool_t*	thpool_init(int threadsN);

void thpool_thread_do(thpool_t* tp_p);

int thpool_add_work(thpool_t* tp_p, void *(*function_p)(void*), void* arg_p);

void thpool_destroy(thpool_t* tp_p);


int thpoool_jobqueue_init(thpool_t* tp_p);


void thpool_jobqueue_add(thpool_t* tp_p, thpool_job_t* newjob_p);

int thpool_jobqueue_removelast(thpool_t* tp_p);

thpool_job_t* thpool_jobqueue_peek(thpool_t* tp_p);

void thpool_jobqueue_empty(thpool_t* tp_p);


#endif
