#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>

#define IPADDRESS	"127.0.0.1"
#define PORT		8787
#define MAXSIZE		1024
#define LISTENQ		5
#define FDSIZE		1000
#define EPOLLEVENTS	100

// function declaration
// create socket and bind it
static int socket_bind(const char* ip, int port);
// IO multi-reuse epoll
static void do_epoll(int listenfd);
// event handler
static void handle_events(int epollfd, struct epoll_event* events,
	int num, int listenfd, char* buf);
// incomming connection handler
static void handle_accept(int epollfd, int listenfd);
// reading process
static void do_read(int epollfd, int fd, char* buf);
// writing process
static void do_write(int epollfd, int fd, char* buf);
// add event
static void add_event(int epollfd, int fd, int state);
// mod event
static void mod_event(int epollfd, int fd, int state);
// del event
static void del_event(int epollfd, int fd, int state);

int main(int argc, char* argv[]) {
	int listenfd = 0;
	listenfd = socket_bind(IPADDRESS, PORT);
	listen(listenfd, LISTENQ);
	do_epoll(listenfd);
	return 0;
}

static int socket_bind(const char* ip, int port) {
	int listenfd = 1;
	struct sockaddr_in serveraddr = {};
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		perror("socket error: ");
		return 1;
	}

	bzero(&serveraddr, sizeof (serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr*) &serveraddr, sizeof (serveraddr)) == -1) {
		perror("bind error: ");
		return 1;
	}
	return listenfd;
}

static void do_epoll(int listenfd) {
	int epollfd = 0;
	struct epoll_event events[EPOLLEVENTS] = {};
	int ret = 0;
	char buf[MAXSIZE] = {};
	// create a descriptor
	epollfd = epoll_create(FDSIZE);
	// add descriptor
	add_event(epollfd, listenfd, EPOLLIN);
	for ( ; ; ) {
		// fetch ready descriptor
		ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
		handle_events(epollfd, events, ret, listenfd, buf);
	}
	close(epollfd);
}

static void handle_events(int epollfd, struct epoll_event* events,
	int num, int listenfd, char* buf) {
	int i = 0;
	int fd = 0;
	for (i = 0; i < num; i++) {
		fd = events[i].data.fd;
		if ((fd == listenfd) && (events[i].events & EPOLLIN)) {
			handle_accept(epollfd, listenfd);
		} else if (events[i].events & EPOLLIN) {
			do_read(epollfd, fd, buf);
		} else if (events[i].events & EPOLLOUT) {
			do_write(epollfd, fd, buf);
		}
	}
}

static void handle_accept(int epollfd, int listenfd) {
	int clifd = 0;
	struct sockaddr_in cliaddr = {};
	socklen_t cliaddrlen = 0;
	clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
	if (clifd == -1) {
		perror("accept error: ");
	} else {
		printf("accept a new client: %s, %d\n",
			inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
		add_event(epollfd, clifd, EPOLLIN);
	}
}

static void do_read(int epollfd, int fd, char* buf) {
	int nread = 0;
	nread = read(fd, buf, MAXSIZE);
	if (nread == -1) {
		perror("read error: ");
		close(fd);
		del_event(epollfd, fd, EPOLLIN);
	} else if (nread == 0) {
		perror("client close: ");
		close(fd);
		del_event(epollfd, fd, EPOLLIN);
	} else {
		printf("read message is : %s\n", buf);
		mod_event(epollfd, fd, EPOLLOUT);
	}
}

static void do_write(int epollfd, int fd, char* buf) {
	int nwrite = 0;
	nwrite = write(fd, buf, strlen(buf));
	if (nwrite == -1) {
		perror("write error: ");
		close(fd);
		del_event(epollfd, fd, EPOLLOUT);
	} else {
		mod_event(epollfd, fd, EPOLLIN);
	}
	memset(buf, 0, MAXSIZE);
}

static void add_event(int epollfd, int fd, int state) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void del_event(int epollfd, int fd, int state) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void mod_event(int epollfd, int fd, int state) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
