#include <string.h>
#include <pthread.h>

#include "customsocket.h"
#include "customfile.h"

#define SERV_PORT 8000
const char *send_style = "\033[33m";
const char *recv_style = "\033[32m\t\t";

char menu();
void chat();
void *chat_recv_msg(void *arg);
void view_history();
void *his_recv_msg(void *arg);
void transfer_file();

int sfd = 0;
int res = 0;
char buf[BUFSIZ];
char username[BUFSIZ];
struct sockaddr_in serv_addr;

int main() {

  char choose;
  int loop_flag = 1;

  while (loop_flag) {
    choose = menu();
    switch (choose) {
      case 'c':
        chat(); break;
      case 'h':
        view_history(); break;
      case 't':
        transfer_file(); break;
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
  printf("-----------------------------------------\n");
  printf("|输入 :c 进入群聊和私聊 (chat)          |\n");
  printf("|输入 :h 查看历史记录(history)          |\n");
  printf("|输入 :t 开始文件传输 (transfer file)   |\n");
  printf("|输入 :q 退出程序 (quit)                |\n");
  printf("-----------------------------------------\n");
  fgets(input, sizeof(input), stdin);
  // # 号代表非法输入
  if (strlen(input) != 3 && input[0] != ':') {
    return '#';
  }
  if (input[1] != 'c' && input[1] != 'h' && input[1] != 't' && input[1] != 'q') {
    return '#';
  }
  return input[1];
}

void chat() {
  
  sfd = Socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

  Connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  system("clear");
  printf("%s", send_style);
  printf("----------------------------------------\n");
  printf("|成功连接服务器                        |\n");
  printf("----------------------------------------\n");


  const char *chooseinfo = "c";
  Write(sfd, (void *)chooseinfo, sizeof(chooseinfo));

  pthread_t tid;

  bzero(username, sizeof(username));
  Read(sfd, username, sizeof(username));

  printf("昵称: %s\n", username);
  printf("----------------------------------------\n");
  printf("|默认输入的消息是将群发                |\n");
  printf("|输入 :u 昵称 消息内容 将私发给该昵称  |\n");
  printf("|输入 :q 退出群聊 (quit)               |\n");
  printf("----------------------------------------\n");

  pthread_create(&tid, NULL, chat_recv_msg, (void *)&sfd);
  pthread_detach(tid);

  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);
    if (strlen(buf) == 3 && buf[0] == ':' && buf[1] == 'q') {  // 退出群聊
      system("clear");
      Write(sfd, buf, strlen(buf));
      break;
    }
    Write(sfd, buf, strlen(buf));
  }

  Close(sfd);
  return;
}

void *chat_recv_msg(void *arg) {
  char buf[BUFSIZ];
  int res = 0;
  int *sfd = (int *)arg;
  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    res = Read(*sfd, buf, sizeof(buf));
    Write(STDOUT_FILENO, buf, res);
    Write(STDOUT_FILENO, (void *)send_style, strlen(send_style));
  }
  Close(*sfd);
  return (void *)0;
}

void view_history() {

  sfd = Socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

  Connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  system("clear");
  printf("%s", send_style);
  printf("----------------------------------------\n");
  printf("|成功连接服务器                        |\n");
  printf("----------------------------------------\n");

  
  const char *chooseinfo = "h";
  Write(sfd, (void *)chooseinfo, sizeof(chooseinfo));
  
  pthread_t tid;
  
  printf("----------------------------------------\n");
  printf("|输入 :q 退出查看历史记录 (quit)       |\n");
  printf("----------------------------------------\n");

  pthread_create(&tid, NULL, his_recv_msg, (void *)&sfd);
  pthread_detach(tid);


  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);
    if (strlen(buf) == 3 && buf[0] == ':' && buf[1] == 'q') {  // 退出群聊
      system("clear");
      Write(sfd, buf, strlen(buf));
      break;
    }
    Write(sfd, buf, strlen(buf));
  }
  return;
}

void *his_recv_msg(void *arg) {
  char buf[BUFSIZ];
  int res = 0;
  int *sfd = (int *)arg;
  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    res = Read(*sfd, buf, sizeof(buf));
    if (res == 0) {
      break;
    }
    Write(STDOUT_FILENO, buf, res);
    Write(STDOUT_FILENO, (void *)send_style, strlen(send_style));
  }
  Close(*sfd);
  return (void *)0;
}

void transfer_file() {

  sfd = Socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

  Connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  system("clear");
  printf("%s", send_style);
  printf("----------------------------------------\n");
  printf("|成功连接服务器                        |\n");
  printf("----------------------------------------\n");


  const char *chooseinfo = "t";
  Write(sfd, (void *)chooseinfo, sizeof(chooseinfo));
  Close(sfd);

  sfd = Socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);

  Connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  system("clear");
  char up_down_choose[BUFSIZ];
  char filename[BUFSIZ];
  char buf[BUFSIZ];
  bzero(up_down_choose, sizeof(up_down_choose));
  bzero(filename, sizeof(filename));
  
  printf("----------------------------------------\n");
  printf("|输入 :u 文件名 上传该文件到服务器     |\n");
  printf("|输入 :d 文件名 从服务器下载该文件     |\n");
  printf("----------------------------------------\n");

  fgets(up_down_choose, sizeof(up_down_choose), stdin);

  Write(sfd, up_down_choose, sizeof(up_down_choose));

  // 获得文件名
  int i = 3, j = 0;
  while (up_down_choose[i] != '\0') {
    filename[j] = up_down_choose[i];
    i++;
    j++;
  }
  filename[strlen(filename) - 1] = '\0';

  if (up_down_choose[1] == 'u') {
    FILE *fp = Fopen(filename, "r");
    bzero(buf, sizeof(buf));
    while (fgets(buf, sizeof(buf), fp) != NULL) {
      Write(sfd, buf, sizeof(buf));
      bzero(buf, sizeof(buf));
    }
    Fclose(fp);
    Close(sfd);
    printf("上传成功\n");
  } else if (up_down_choose[1] == 'd') {
    char new_filename[BUFSIZ + 5];
    sprintf(new_filename, "down-%s", filename);
    FILE *fp = fopen(new_filename, "w");
    bzero(buf, sizeof(buf));
    while ((res = Read(sfd, buf, sizeof(buf))) != 0) {
      fputs(buf, fp);
      bzero(buf, sizeof(buf));
    }
    Fclose(fp);
    printf("下载成功\n");
  }
  sleep(10);
  system("clear");
  return;

}
