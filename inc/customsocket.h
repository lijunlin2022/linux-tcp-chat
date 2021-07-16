#ifndef _CUSTOMSOCKET_H_
#define _CUSTOMSOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void perr_exit(const char *info);
int Socket(int domain, int type, int protocol);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int baclog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Close(int fd);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, void *buf, size_t count);
ssize_t Readn(int fd, void *buf, size_t n);
ssize_t Writen(int fd, const void *vbuf, size_t n);

#endif
