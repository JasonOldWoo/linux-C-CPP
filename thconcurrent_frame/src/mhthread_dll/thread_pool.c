#include "thread_pool.h"


static thread_pool* t_pool = NULL;

void thpool_init(int max_thread_num)
{
	t_pool = (thread_pool*) malloc(sizeof (thread_pool));

	// initialize mutex.
	pthread_mutex_init(&(t_pool->queue_lock), NULL);
	// initialize condition variable
	pthread_cond_init(&(t_pool->queue_ready), NULL);

	t_pool->queue_head = NULL;
	t_pool->max_thread_num = max_thread_num;
	t_pool->curr_queue_size = 0;

	t_pool->is_shutdown = 0;

	t_pool->threadp = (pthread_t*) malloc(max_thread_num * sizeof (pthread_t));

	int i = 0;
	for (i=0; i<max_thread_num; i++)
	{
		pthread_create(&(t_pool->threadp[i]), NULL, thread_routine, NULL);
	}
}

// add workers to thread pool
int thpool_add_worker(void* (*function) (void* arg), void* arg)
{
	thread_worker* newworker = (thread_worker*) malloc(sizeof (thread_worker));
	newworker->function = function;
	newworker->arg = arg;
	newworker->next = NULL;

	pthread_mutex_lock(&(t_pool->queue_lock));

	thread_worker *worker = t_pool->queue_head;
	//printf("[%s]1 t_pool->queue_head=%p\n", __FUNCTION__, worker);
	if (worker != NULL)
	{
		while (worker->next != NULL)
			worker = worker->next;
		worker->next = newworker;
	}
	else
		t_pool->queue_head = newworker;

	//printf("[%s]2 t_pool->queue_head=%p\n", __FUNCTION__, worker);
	assert(t_pool->queue_head != NULL);

	t_pool->curr_queue_size++;
	pthread_mutex_unlock(&(t_pool->queue_lock));
	// signal the condition, and this worker will get on working!
	// just like a worker get the key and then step into the workroom!
	pthread_cond_signal(&(t_pool->queue_ready));
	return 0;
}

// destroy the thread pool
// it should be noted that active threads will
// exit untill it's  done!
int thpool_destroy()
{
	if (t_pool->is_shutdown)
		return -1;
	t_pool->is_shutdown = 1;

	// signal all the conditions.
	pthread_cond_broadcast(&(t_pool->queue_ready));

	// wait for unfinished jobs.
	int i = 0;
	assert(t_pool->threadp != NULL);
	for (i=0; i<t_pool->max_thread_num; i++)
		pthread_join(t_pool->threadp[i], NULL);
	free(t_pool->threadp);
	t_pool->threadp = NULL;

	thread_worker *head = NULL;
	while (t_pool->queue_head != NULL)
	{
		head = t_pool->queue_head;
		assert(head != NULL);
		t_pool->queue_head = t_pool->queue_head->next;
		free(head);
		head = NULL;
	}

	// release mutex and cond
	pthread_mutex_destroy(&(t_pool->queue_lock));
	pthread_cond_destroy(&(t_pool->queue_ready));

	free(t_pool);
	t_pool = NULL;
	return 0;
}

void* thread_routine(void* arg)
{
	printf("TID[%lu] starting thread....\n", pthread_self());

	while (1)
	{
		pthread_mutex_lock(&(t_pool->queue_lock));
		while (0 == t_pool->curr_queue_size && !t_pool->is_shutdown)
		{
			printf("TID[%lu] is waiting....\n", pthread_self());
			pthread_cond_wait(&(t_pool->queue_ready),	\
			                  &(t_pool->queue_lock));
		}

		if (t_pool->is_shutdown)
		{
			pthread_mutex_unlock(&(t_pool->queue_lock));
			printf("TID[%lu] will exit!\n", pthread_self());
			pthread_exit(NULL);
		}

		printf("TID[%lu] start to work!\n", pthread_self());
		//printf("curr_queue_size=[%d]\n", t_pool->curr_queue_size);
		if (0 == t_pool->curr_queue_size)
		{
			pthread_mutex_unlock(&(t_pool->queue_lock));
			continue ;
		}

		assert(t_pool->curr_queue_size != 0);
		assert(t_pool->queue_head != NULL);

		// active work size-1.
		t_pool->curr_queue_size--;
		thread_worker *worker = t_pool->queue_head;
		t_pool->queue_head = worker->next;
//		printf("[%s]1 worker=%p\n", __FUNCTION__, worker);
//		printf("[%s]1 worker->next=%p\n", __FUNCTION__, worker->next);

		pthread_mutex_unlock(&(t_pool->queue_lock));

		// now do the reall job here.
		(*(worker->function)) (worker->arg);
		assert(worker != NULL);
//		printf("[%s]2 worker=%p\n", __FUNCTION__, worker);
//		printf("[%s]2 worker->next=%p\n", __FUNCTION__, worker->next);
		free(worker);
		worker = NULL;
	}
	// shouldn't arrive here!
	pthread_exit(NULL);
}
