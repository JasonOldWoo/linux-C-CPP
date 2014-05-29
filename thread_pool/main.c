#include "thread_pool.h"

void* myprocess (void *arg)
{
	printf ("TID[%lu], arg=[%d]\n", pthread_self (),*(int*) arg);
	sleep (1);
	return NULL;
}

int main (int argc, char *argv[])
{
	thpool_init (2);
	int *arg = (int *) malloc (sizeof (int) * 20);
	int i;
	for (i=0; i<20; i++)
	{
		arg[i] = i;
		thpool_add_worker (myprocess, &arg[i]);
	}
	sleep(20);
	thpool_destroy();

	free(arg);
	return 0;
}
