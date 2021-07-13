#include "customsocket.h"

void perr_exit(const char *info) {
  perror(info);
  exit(-1);
}

int Socket(int domain, int type, int protocol) {
  int ret = socket(domain, type, protocol);
  if (ret == -1) {
    perr_exit("socket error\n");
  }
  return ret;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int ret = bind(sockfd, addr, addrlen);
  if (ret == -1) {
    perr_exit("bind error\n");
  }
  return ret;
}

int Listen(int sockfd, int baclog) {
  int ret = listen(sockfd, baclog);
  if (ret == -1) {
    perr_exit("listen error\n");
  }
  return ret;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  int ret = 0;

again:
  if ((ret = accept(sockfd, addr, addrlen)) < 0) {
    if ((errno == ECONNABORTED) || (errno == EINTR)) {
      goto again;
    } else {
      perr_exit("accept error");
    }
  }
  return ret;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int ret = connect(sockfd, addr, addrlen);
  if (ret == -1) {
    perr_exit("accept error\n");
  }
  return ret;
}

int Close(int fd) {
  int ret = close(fd);
  if (ret == -1) {
    perr_exit("close error");
  }
  return ret;
}

ssize_t Read(int fd, void *buf, size_t count) {
  ssize_t n;
again:
  if ((n = read(fd, buf, count)) == -1) {
    if (errno == EINTR) {
      goto again;
    } else {
      return -1;
    }
  }
  return n;
}

ssize_t Write(int fd, void *buf, size_t count) {
  ssize_t n;
again:
  if ((n = write(fd, buf, count)) == -1) {
    if (errno == EINTR) {
      goto again;
    } else {
      return -1;
    }
  }
  return n;
}

// 指定读取 n 个字节
ssize_t Readn(int fd, void *buf, size_t n) {
  size_t nleft;  // 剩余未读取的字节数
  size_t nread;  // 实际读到的字节数
  char *ptr;

  ptr = buf;
  nleft = n;

  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) { // read 读取失败
      if (errno == EINTR) { // 读取被中断
        nread = 0;
      } else {
        return -1;
      }
    } else if (nread == 0) {
      break;
    }
    nleft = nleft - nread;
    ptr += nread;
  }

  return n-nleft;
}

// 指定写入 n 个字节
ssize_t Writen(int fd, const void *buf, size_t n) {
  size_t nleft; // 剩余未写入的字节数
  ssize_t nwrite; // 实际写入的字节数
  const char *ptr;

  ptr = buf;
  nleft = n;

  while (nleft > 0) {
    if ((nwrite = write(fd, ptr, nleft)) <= 0) {
      if (nwrite < 0 && errno == EINTR) {
        nwrite = 0;
      } else {
        return -1;
      }
    }
    nleft -= nwrite;
    ptr += nwrite;
  }
  return n;
}
