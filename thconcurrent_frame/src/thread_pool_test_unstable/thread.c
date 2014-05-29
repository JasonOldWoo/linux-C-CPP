#include "global.h"
#include "thread.h"
#include <errno.h>

static int thpool_keepalive = 1;

// init thread mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

thpool_t* thpool_init(int threadsN)
{
	thpool_t* thpool = NULL;
	if (!threadsN || threadsN < 1)
		threadsN = 1;

	thpool = (thpool_t*) malloc(sizeof (thpool_t));
	if (NULL == thpool)
	{
		printf("malloc thpool_t error!\n");
		return NULL;
	}

	thpool->threadsN = threadsN;
	thpool->threads = (pthread_t*) malloc(threadsN * sizeof (pthread_t));
	if (NULL == thpool->threads)
	{
		printf("malloc thpool->threads error!");
		return NULL;
	}
	if (-1 == thpool_jobqueue_init(thpool))
		return NULL;

	thpool->jobqueue->queueSem = (sem_t*) malloc(sizeof (sem_t));
	sem_init(thpool->jobqueue->queueSem, 0, 1);
	int t;
	for (t=0; t<threadsN; t++)
		pthread_create(&(thpool->threads[t]), NULL, (void *)thpool_thread_do, (void*) thpool);

	return thpool;
}

void thpool_destroy(thpool_t* tp_p)
{
	int i;
	thpool_keepalive  = 0;

	for (i=0; i<(tp_p->threadsN); i++)
		if (sem_post(tp_p->jobqueue->queueSem))
			fprintf(stderr, "thpool_destroy(): could not bypass sem_wait()\n");

	if (sem_post(tp_p->jobqueue->queueSem) != 0)
			fprintf(stderr, "thpool_destroy(): could not destroy semaphore\n");

	for (i=0; i<(tp_p->threadsN); i++)
		pthread_join(tp_p->threads[i], NULL);

	thpool_jobqueue_empty(tp_p);

	free(tp_p->threads);
	free(tp_p->jobqueue->queueSem);
	free(tp_p->jobqueue);
	free(tp_p);
}

// initialize queue
int thpool_jobqueue_init(thpool_t* tp_p)
{	
	if (NULL == tp_p)
	{
		printf("[%s] tp_p = NULL\n", __FUNCTION__);
		return -1;
	}

	tp_p->jobqueue = (thpool_jobqueue*)malloc(sizeof (thpool_jobqueue));	// malloc job queue
	if (NULL == tp_p->jobqueue)
		return -1;

	tp_p->jobqueue->tail = NULL;
	tp_p->jobqueue->head = NULL;
	tp_p->jobqueue->jobN = 0;
	return 0;
}

void thpool_thread_do(thpool_t* tp_p)
{
	while (1 == thpool_keepalive)
	{
		if (sem_wait(tp_p->jobqueue->queueSem))
		{	
			perror("thpool_thread_do(): sem_wait error ");
			exit(1);
		}
		if (thpool_keepalive)
		{
			FUNC function;
			void* arg_buff;
			thpool_job_t* job_p;

			pthread_mutex_lock(&mutex);
			job_p = thpool_jobqueue_peek(tp_p);
			printf("[%s]1 job_p=%p\n", __FUNCTION__, job_p);
			
			function = job_p->function;
			arg_buff = job_p->arg;
			if (thpool_jobqueue_removelast(tp_p))
				return ;

			pthread_mutex_unlock(&mutex);
			function(arg_buff);
			assert(job_p != NULL);
			printf("[%s]2 job_p=%p\n", __FUNCTION__, job_p);
			free(job_p);
			job_p = NULL;
		}
		else
			return ;
	}
	return ;
}

thpool_job_t* thpool_jobqueue_peek(thpool_t* tp_p)
{
	return tp_p->jobqueue->tail;
}

int thpool_jobqueue_removelast(thpool_t* tp_p)
{
	if (NULL == tp_p)
		return -1;
	thpool_job_t* theLastJob;
	theLastJob  = tp_p->jobqueue->tail;
	switch (tp_p->jobqueue->jobN)
	{
	case 0:
		return -1;
	case 1:
		tp_p->jobqueue->head = NULL;
		tp_p->jobqueue->tail = NULL;
		break;
	default:
		theLastJob->prev->next = NULL;
		tp_p->jobqueue->tail = theLastJob->prev;
		break;
	}

	(tp_p->jobqueue->jobN)--;
	int reval;
	sem_getvalue(tp_p->jobqueue->queueSem, &reval);
	return 0;
}

void thpool_jobqueue_add(thpool_t* tp_p, thpool_job_t* newjob_p)
{
	newjob_p->next = NULL;
	newjob_p->prev = NULL;
	thpool_job_t* oldFirstJob;
	oldFirstJob = tp_p->jobqueue->head;

	switch (tp_p->jobqueue->jobN)
	{
	case 0:
		tp_p->jobqueue->head = newjob_p;
		tp_p->jobqueue->tail = newjob_p;
		break;
	default:
		oldFirstJob->prev = newjob_p;
		newjob_p->next = oldFirstJob;
		tp_p->jobqueue->head = newjob_p;
		break;
	}

	(tp_p->jobqueue->jobN)++;
	sem_post(tp_p->jobqueue->queueSem);

	int reval;
	sem_getvalue(tp_p->jobqueue->queueSem, &reval);
	return;
}

// add work to thread pool
int thpool_add_work(thpool_t* tp_p, void* (*function_p)(void*), void* arg_p)
{
	thpool_job_t* newjob;
	newjob = (thpool_job_t*) malloc(sizeof (thpool_job_t));

	if (NULL == newjob)
	{
		fprintf(stderr, "thpool_add_work(): could not allocate memory for new job\n");
		exit(1);
	}
	newjob->function = function_p;
	newjob->arg = arg_p;
	pthread_mutex_lock(&mutex);
	thpool_jobqueue_add(tp_p, newjob);
	pthread_mutex_unlock(&mutex);
	return 0;
}

// empty queue
void thpool_jobqueue_empty(thpool_t* tp_p)
{
	thpool_job_t* curjob;
	curjob = tp_p->jobqueue->tail;

	while (tp_p->jobqueue->jobN)
	{
		tp_p->jobqueue->tail = curjob->prev;
		free(curjob);
		curjob = tp_p->jobqueue->tail;
		(tp_p->jobqueue->jobN)--;
	}

	tp_p->jobqueue->head = NULL;
	tp_p->jobqueue->tail = NULL;
}
