#include "global.h"
#include "thread.h"

void* task1()
{
	printf("# thread working: %u\n", (int) pthread_self());
	printf("task1 running..\n");
}

void *task2(int a)
{
	printf("# thread working: %u\n", (int) pthread_self());
	printf("task 2 running..\n");
	printf("%d\n", a);
}

int main()
{
	thpool_t* thpool;
	int i;
	thpool = thpool_init(5);
	puts("Adding 20 tasks to threadpool");
	int a = 54;
	for (i=0; i<20; i++)
	{
		thpool_add_work(thpool, (void*) task1, NULL);	
		thpool_add_work(thpool, (void*) task2, (void*) a);
	}

	puts("destroy thread pool");
	thpool_destroy(thpool);
}
