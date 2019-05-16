#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static int _resolvev6()
{
    const char* addr = "www.google.com";
    struct addrinfo hints;
    struct addrinfo* result = NULL;
    struct addrinfo* ri = NULL;
    struct sockaddr_in6 dest;
    struct sockaddr_in dest4;
    struct sockaddr* vdest;
    int s = 0, sock = 0;
    socklen_t socklen = 0;
    bzero(&hints, sizeof(hints));
    bzero(&dest, sizeof(dest));
    bzero(&dest4, sizeof(dest4));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(addr, "443", &hints, &result);
    if (s != 0) {
        perror("getaddrinfo");
        return -1;
    }
    if (!result) {
        perror("result=NULL\n");
        return -1;
    }
    s = AF_INET6;
    for (ri = result; ri != NULL; ri = ri->ai_next) {
        char szdst[128] = {};
        if (ri->ai_family == AF_INET) {
            inet_ntop(ri->ai_family, &((struct sockaddr_in*) ri->ai_addr)->sin_addr, szdst, sizeof(szdst));
            memcpy(&dest4.sin_addr, &((struct sockaddr_in*) ri->ai_addr)->sin_addr, sizeof(struct sockaddr_in));
            s = AF_INET;
        } else {
            inet_ntop(ri->ai_family, &((struct sockaddr_in6*) ri->ai_addr)->sin6_addr, szdst, sizeof(szdst));
            memcpy(&dest.sin6_addr, &((struct sockaddr_in6*) ri->ai_addr)->sin6_addr, sizeof(struct sockaddr_in6));
        }
        printf("family=%d, is_ipv6=%d, host=%s\n", ri->ai_family, (ri->ai_family == AF_INET6), szdst);
    }
    freeaddrinfo(result);

try_again:

    if (s == AF_INET) {
        if (!dest4.sin_addr.s_addr) {
            s = AF_INET6;
            goto try_again;
        }
        dest4.sin_family = AF_INET;
        dest4.sin_port = htons(443);
        vdest = (struct sockaddr*) &dest4;
        socklen = sizeof(dest4);
    } else {
        dest.sin6_family = AF_INET6;
        dest.sin6_port = htons(443);
        vdest = (struct sockaddr*) &dest;
        socklen = sizeof(dest);
    }

    sock = socket(s, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket DGRAM");
        return -1;
    }

    // test ipv4/ipv6 connectivity
    if (connect(sock, vdest, socklen) < 0) {
        perror("connect DGRAM");
        if (s == AF_INET6) {
            close(sock);
            return -1;
        } else {
            s = AF_INET;
            goto try_again;
        }
    }

    close(sock);
    if (s == AF_INET6) {
        return 0;
    } else {
        return -1;
    }
}

int main()
{
    printf("resolve6=%d\n", _resolvev6());
}
