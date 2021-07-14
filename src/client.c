#include <string.h>
#include <pthread.h>
#include "customsocket.h"

#define SERV_PORT 8000
const char *send_style = "\033[33m";
const char *send_tip = ">> ";
const char *recv_style = "\033[32m\t\t";

char menu();
void group_chat();
void *recv_msg(void *arg);

int main() {
  char choose;
  int loop_flag = 1;
  while (loop_flag) {
    choose = menu();
    switch (choose) {
      case 'g':
        group_chat(); break;
      case 'p':
        break;
      case 'h':
        break;
      case 't':
        break;
      case 'q':
	loop_flag = 0; break;
      default:
	system("clear");
	printf("----------------------------------------\n");
	printf("|非法输入，请重新输入                  |\n");
        break;
    }
  }
  
  return 0;
}

char menu() {
  char input[BUFSIZ];
  printf("%s", send_style);
  printf("----------------------------------------\n");
  printf("|输入 g 进入群聊天 (group chat)        |\n");
  printf("|输入 p 进入私聊 (private chat)        |\n");
  printf("|输入 h 查看群聊历史记录(history)      |\n");
  printf("|输入 t 开始文件传输 (transfer file)   |\n");
  printf("|输入 q 退出程序 (quit)                |\n");
  printf("----------------------------------------\n");
  printf("%s", send_tip);
  fgets(input, sizeof(input), stdin);
  // # 号代表非法输入
  if (strlen(input) != 2) {
    return '#';
  }
  if (input[0] != 'g' && input[0] != 'p' && input[0] != 'h' && input[0] != 't') {
    return '#';
  }
  return input[0];
}

void group_chat() {
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
  system("clear");
  printf("----------------------------------------\n");
  printf("|成功连接服务器                        |\n");
  printf("|输入信息直接发送到群聊                |\n");
  printf("|输入 q 退出群聊 (quit)                |\n");
  printf("----------------------------------------\n");
  
  pthread_create(&tid, NULL, recv_msg, (void *)&sfd);
  pthread_detach(tid);

  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);
    if (strlen(buf) == 2 && buf[0] == 'q') {  // 退出群聊
      system("clear");
      break;
    }
    Write(sfd, buf, strlen(buf));
  }

  Close(sfd);
}

void *recv_msg(void *arg) {
  char buf[4068];
  int res = 0;
  int *sfd = (int *)arg;
  for ( ; ; ) {
    res = Read(*sfd, buf, sizeof(buf));
    write(STDOUT_FILENO, buf, res);
    Write(STDOUT_FILENO, (void *)send_style, strlen(send_style));
  }
  Close(*sfd);
}

