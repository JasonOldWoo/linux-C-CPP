#include "thread_pool.h"

void* myprocess (void *arg)
{
	printf ("TID[%lu], working on task %d\n", pthread_self (),*(int *) arg);
	sleep (1);
	return NULL;
}

int main (int argc, char *argv[])
{
	thpool_init (3);
	int *threads_num = (int *) malloc (sizeof (int) * 10);
	int i;
	for (i=0; i<10; i++)
	{
		threads_num[i] = i;
		thpool_add_worker (myprocess, &threads_num[i]);
	}
	sleep(5);
	thpool_destroy ();

	free(threads_num);
	return 0;
}
