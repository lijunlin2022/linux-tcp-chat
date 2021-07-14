#include <string.h>
#include <pthread.h>
#include "customsocket.h"

#define SERV_PORT 8000
const char *send_style = "\033[33m";
const char *recv_style = "\033[32m\t\t";

void *recv_msg(void *arg);

int main() {
  int sfd = 0;
  int res = 0;
  char buf[BUFSIZ];
  pthread_t tid;

  struct sockaddr_in serv_addr;
  sfd = Socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

  Connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  pthread_create(&tid, NULL, recv_msg, (void *)&sfd);
  pthread_detach(tid);

  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    Write(STDOUT_FILENO, (void *)send_style, strlen(send_style));
    res = Read(STDIN_FILENO, buf, sizeof(buf));
    Write(sfd, buf, res);
  }

  Close(sfd);
  return 0;
}

void *recv_msg(void *arg) {
  char buf[4068];
  int res = 0;
  int *sfd = (int *)arg;
  for ( ; ; ) {
    res = Read(*sfd, buf, sizeof(buf));
    Write(STDOUT_FILENO, (void *)recv_style, strlen(recv_style));
    write(STDOUT_FILENO, buf, res);
    Write(STDOUT_FILENO, (void *)send_style, strlen(send_style));
  }
  Close(*sfd);
}
