#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAXSIZE		1024
#define IPADDRESS	"127.0.0.1"
#define SERV_PORT	8787
#define FDSIZE		1024
#define EPOLLEVENTSNUM	20

static void handle_connection(int sockfd);
static void handle_events(int epollfd, struct epoll_event* events,
	int num, int sockfd, char* buf);
static void do_read(int epollfd, int fd, int sockfd, char* buf);
static void do_write(int epollfd, int fd, int sockfd, char* buf);
static void add_event(int epollfd, int fd, int state);
static void del_event(int epollfd, int fd, int state);
static void mod_event(int epollfd, int fd, int state);

int main(int argc, char* argv[]) {
	int sockfd = 0;
	struct sockaddr_in servaddr = {};
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof (servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
	connect(sockfd, (struct sockaddr*) &servaddr, sizeof (servaddr));
	// handle connection
	handle_connection(sockfd);
	close(sockfd);
	return 0;
}

static void handle_connection(int sockfd) {
	int epollfd = 0;
	struct epoll_event events[EPOLLEVENTSNUM] = {};
	char buf[MAXSIZE] = {};
	int ret = 0;
	epollfd = epoll_create(FDSIZE);
	add_event(epollfd, STDIN_FILENO, EPOLLIN);
	for ( ; ; ) {
		ret = epoll_wait(epollfd, events, EPOLLEVENTSNUM, -1);
		handle_events(epollfd, events, ret, sockfd, buf);
	}
	close(epollfd);
}

static void handle_events(int epollfd, struct epoll_event* events,
	int num, int sockfd, char* buf) {
	int fd = 0;
	int i = 0;
	for (i = 0; i < num; i++) {
		fd = events[i].data.fd;
		if (events[i].events & EPOLLIN) {
			do_read(epollfd, fd, sockfd, buf);
		} else if (events[i].events & EPOLLOUT) {
			do_write(epollfd, fd, sockfd, buf);
		}
	}
}

static void do_read(int epollfd, int fd, int sockfd, char* buf) {
	int nread = 0;
	nread = read(fd, buf, MAXSIZE);
	if (nread == -1) {
		perror("read error: ");
		close(fd);
	} else if (nread == 0) {
		perror("server close: ");
		close(fd);
	} else {
		if (fd == STDIN_FILENO) {
			add_event(epollfd, sockfd, EPOLLIN);
		} else {
			del_event(epollfd, sockfd, EPOLLIN);
			add_event(epollfd, STDOUT_FILENO, EPOLLOUT);
		}
	}
}

static void do_write(int epollfd, int fd, int sockfd, char* buf) {
	int nwrite = 0;
	nwrite = write(fd, buf, strlen(buf));
	if (nwrite == -1) {
		perror("write error: ");
		close(fd);
	} else {
		if (fd == STDOUT_FILENO) {
			del_event(epollfd, fd, EPOLLOUT);
		} else {
			mod_event(epollfd, fd, EPOLLIN);
		}
	}
	memset(buf, 0, MAXSIZE);
}

static void add_event(int epollfd, int fd, int state) {
	struct epoll_event ev = {};
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void del_event(int epollfd, int fd, int state) {
	struct epoll_event ev = {};
	ev.events  = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void mod_event(int epollfd, int fd, int state) {
	struct epoll_event ev = {};
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
