#include <stdio.h>
#include <stdlib.h>
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
  char name[BUFSIZ];
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
  char username[BUFSIZ];

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
      
      sprintf(username, "%s%d", "user", cfd);
      strncpy(cinfos[i].name, username, strlen(username));
      
      // 分配昵称
      printf("给 cfd 为 %d  的客户端分配昵称 %s\n", cinfos[i].cfd, cinfos[i].name);
      Write(cinfos[i].cfd, cinfos[i].name, sizeof(cinfos[i].name));

      pthread_create(&tid, NULL, group_chat, (void*)&cinfos[i]);
      pthread_detach(tid);  // 子进程分离,防止僵尸线程产生
      i++;
    }
  }
  return 0;
}

void *group_chat(void *arg) {
  struct cli_info *c = (struct cli_info *)arg;
  char prefix[BUFSIZ + 2];
  char buf[BUFSIZ];

  sprintf(prefix, "%s: ", c->name);
  int res = 0;
  for ( ; ; ) {
    res = Read(c->cfd, buf, sizeof(buf));
    if (res == 0) {
      printf("%s 已经断开连接\n", c->name);
      break;
    }
    printf("从 %s 接到信息: \n", c->name);
    Write(STDOUT_FILENO, buf, res);
    printf("--------------------\n");
    
    // 转发消息到除了发送消息之外的客户端
    for (int i = 0; i < CLI_CNT; i++) {
      if ((cinfos[i].cfd != (*c).cfd) && (cinfos[i].cfd != 0)) {
        Write(cinfos[i].cfd, (void *)chat_style, strlen(chat_style));
	Write(cinfos[i].cfd, prefix, strlen(prefix));
        Write(cinfos[i].cfd, buf, res);
      }
    }
  }
  Close(c->cfd);
  return (void *)0;
}
