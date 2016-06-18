#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#define MAX_EVENTS 64

int main() {
	struct epoll_event* events;
	int nr_events = 0;
	int i = 0;
	int epfd = 0;

	events = (struct epoll_event*)
		malloc(sizeof (struct epoll_event) * MAX_EVENTS);

	if (!events) {
		perror("malloc failed ");
		return 1;
	}

	nr_events = epoll_wait(epfd, events, MAX_EVENTS, -1);

	if (nr_events < 0) {
		perror("epoll_wait ");
		free(events);
		return 1;
	}

	for (i = 0; i < nr_events; i++) {
		printf("event: %d on fd: %d\n",
			events[i].events,
			events[i].data.fd);
	}

	free(events);
	return 0;
}
