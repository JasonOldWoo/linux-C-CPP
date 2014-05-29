#include "thread_pool.h"

void* myprocess (void *arg)
{
	printf ("TID[%lu], arg=[%d]\n", pthread_self (),*(int*) arg);
	return NULL;
}

int main (int argc, char* argv[])
{
	thpool_init (100);
	int *arg = (int *) malloc (sizeof (int) * 1000);
	int i;
	for (i=0; i<1000; i++)
	{
		arg[i] = i;
		thpool_add_worker (myprocess, &arg[i]);
	}
	sleep(6);
	thpool_destroy();

	free(arg);
	return 0;
}
