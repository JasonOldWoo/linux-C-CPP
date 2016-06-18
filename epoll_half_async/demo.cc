#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#define MAXBUF 1024
#define SERVER_ADDR "127.0.0.1"
#define PORT 3966

void setnonblocking(int sockfd) {
	int opts;

	opts = fcntl(sockfd, F_GETFL);
	if(opts < 0) {
		std::cerr << "f_getfl failed" << std::endl;
		exit(1);
	}
	opts = (opts | O_NONBLOCK);
	if(fcntl(sockfd, F_SETFL, opts) < 0) {
		std::cerr << "f_setfl failed" << std::endl;
		exit(1);
	}
}

void handler(int sockfd) {
	std::cout << "handler" << std::endl;
	char buffer[MAXBUF] = "";
	int len = read(sockfd, buffer, MAXBUF);
	if (len > 0) {
		std::cout << buffer << std::endl;
	} else {
		std::cerr << "handler failed" << std::endl;
	}
}

typedef void (*proc)(int);

struct op {
	proc do_complete;
};

int main() {
	int sockfd;
	struct sockaddr_in dest;
	char buffer[MAXBUF];
	memset(buffer, 0x0, sizeof (buffer));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "socket failed" << std::endl;
		return 1;
	}
	std::cout << "sockfd: " << sockfd << std::endl;

	//setnonblocking(sockfd);

	bzero(&dest, sizeof (dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(PORT);

	if (inet_aton(SERVER_ADDR, &dest.sin_addr) == 0) {
		std::cerr << SERVER_ADDR << std::endl;
		return 1;
	}

	std::cout << "ready to connect" << std::endl;
	if (connect(sockfd, (struct sockaddr*) &dest,
		sizeof (dest)) != 0) {
		std::cerr << "connect failed" << std::endl;
		return 1;
	}
	std::cout << "connect ok" << std::endl;

	int epfd = epoll_create(20000);
	epoll_event event;
	event.events = EPOLLIN | EPOLLERR | EPOLLET;
#if 0
	event.data.fd = sockfd;
#else
	op p;
	p.do_complete = (proc) handler;
	event.data.ptr = (void*) &p;
	printf("data.ptr: 0x%x\n", (unsigned int) event.data.ptr);
#endif
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);

	while (1) {
		epoll_event read_events[128];
		int events_num = epoll_wait(epfd, read_events,
			128, 2 * 1000);
		//std::cout << "incomming" << std::endl;

		for (int i = 0; i < events_num; i++) {
			if (read_events[i].events == EPOLLIN) {
				std::cout << "event: "
					<< read_events[i].events << std::endl;
				//std::cout << "descriptor: "
				//	<< read_events[i].data.fd << std::endl;
#if 0
				int len = read(read_events[i].data.fd,
					buffer, MAXBUF);

				if (len > 0) {
					std::cout << buffer << std::endl;
				} else {
					std::cerr << "len: " << len << std::endl;
				}
#else
				printf("data.ptr: 0x%x\n",
					(unsigned int) read_events[i].data.ptr);
				op* pp = (op*) read_events[i].data.ptr;
				(*(pp->do_complete))(sockfd);
#endif
			} else if (read_events[i].events == EPOLLERR) {
				printf("reset by remote peer\n");
				close(sockfd);
				return 1;
			} else if (read_events[i].events == EPOLLOUT) {
				std::cout << "epollout" << std::endl;
				continue ;
			} else {
				std::cout << "event: "
					<< read_events[i].events << std::endl;
				continue ;
			}
		}
	}

	return 0;
}
