#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "customsocket.h"

#define MAXLINE 8192
#define SER_PORT 8000
#define CLI_CNT 50

struct cli_info {
  struct sockaddr_in addr;
  int cfd;
};

const char *chat_style = "\033[32m\t\t";
const char *err_style = "\033[31m";
const char *err_group_chat = "群聊人数已满, 无法发送信息到群聊\n";

struct cli_info cinfos[CLI_CNT];

void *group_chat(void *arg);

int main() {

  struct sockaddr_in ser_addr, cli_addr;
  socklen_t cli_addr_len;
  int lfd, cfd;
  pthread_t tid;

  lfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&ser_addr, sizeof(ser_addr));
  bzero(&cli_addr, sizeof(cli_addr));
  memset(cinfos, 0, sizeof(cinfos));

  ser_addr.sin_family = AF_INET;
  ser_addr.sin_port = htons(SER_PORT);
  ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  Bind(lfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));

  Listen(lfd, 128);
  printf("接收客户端连接...\n");

  int i = 0;
  for ( ; ; ) {
    cli_addr_len = sizeof(cli_addr);
    cfd = Accept(lfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    if (i == CLI_CNT) { // 群聊人数已满
      Write(cfd, (void *)err_style, strlen(err_style));
      Write(cfd, (void *)err_group_chat, strlen(err_group_chat));
    } else {
      cinfos[i].addr = cli_addr;
      cinfos[i].cfd = cfd;
      pthread_create(&tid, NULL, group_chat, (void*)&cinfos[i]);
      pthread_detach(tid);  // 子进程分离,防止僵尸线程产生
      i++;
    }
  }
  return 0;
}

void *group_chat(void *arg) {
  struct cli_info *cinfo = (struct cli_info *)arg;
  char buf[MAXLINE];
  char str[INET_ADDRSTRLEN];

  int res = 0;
  for ( ; ; ) {
    res = Read(cinfo->cfd, buf, MAXLINE);
    if (res == 0) {
      printf("描述符为 %d 的客户端已经断开连接\n", cinfo->cfd);
      break;
    }
    printf("从 %s:%d 接到信息: \n",
            inet_ntop(AF_INET, &(*cinfo).addr.sin_addr, str, sizeof(str)),
            ntohs((*cinfo).addr.sin_port));
    Write(STDOUT_FILENO, buf, res);
    
    // 转发消息到除了发送消息之外的客户端
    for (int i = 0; i < CLI_CNT; i++) {
      if ((cinfos[i].cfd != (*cinfo).cfd) && (cinfos[i].cfd != 0)) {
        Write(cinfos[i].cfd, (void *)chat_style, strlen(chat_style));
        Write(cinfos[i].cfd, buf, res);
      }
    }
  }
  Close(cinfo->cfd);
  return (void *)0;
}
