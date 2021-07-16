#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "customsocket.h"
#include "customfile.h"

#define SER_PORT 8000
#define CLI_CNT 50

struct cli_info {
  struct sockaddr_in addr;
  int cfd;
  char name[BUFSIZ];
};

const char *pri_chat_style = "\033[34m\t\t";
const char *gro_chat_style = "\033[32m\t\t";
const char *err_style = "\033[31m";
const char *err_group_chat = "群聊人数已满, 无法发送信息到群聊\n";

struct cli_info cinfos[CLI_CNT];

void *chat(void *arg);

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
  char chooseinfo[1];
  int res = 0;
  for ( ; ; ) {
    cli_addr_len = 0;
    cfd = 0;
    bzero(chooseinfo, sizeof(chooseinfo));
    res = 0;

    cli_addr_len = sizeof(cli_addr);
    cfd = Accept(lfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    res = Read(cfd, chooseinfo, sizeof(chooseinfo));
    
    if (chooseinfo[0] == 'c') {
      printf("cfd 为 %d 的客户端想要加入群聊\n", cfd);

      if (i == CLI_CNT) { // 群聊人数已满
        Write(cfd, (void *)err_style, strlen(err_style));
        Write(cfd, (void *)err_group_chat, strlen(err_group_chat));
      } else {
        cinfos[i].addr = cli_addr;
        cinfos[i].cfd = cfd;
      
        sprintf(username, "%s%d", "user", cfd);
        strncpy(cinfos[i].name, username, strlen(username));
      
        // 分配昵称
        printf("cfd 为 %d 的客户端分配昵称 %s\n", cinfos[i].cfd, cinfos[i].name);
        Write(cinfos[i].cfd, cinfos[i].name, sizeof(cinfos[i].name));

        pthread_create(&tid, NULL, chat, (void*)&cinfos[i]);
        pthread_detach(tid);  // 子进程分离,防止僵尸线程产生
        i++;
      }
    } else if (chooseinfo[0] == 'h') {
      printf("cfd 为 %d 的客户端想要访问聊天记录\n", cfd);

      FILE *fp = Fopen("chat_history.txt", "r");
      char buf[BUFSIZ];
      bzero(buf, sizeof(buf));
      while (fgets(buf, sizeof(buf), fp) != NULL) {
        Write(cfd, buf, sizeof(buf));
	bzero(buf, sizeof(buf));
      }
    } else if (chooseinfo[0] == 't') {
      // 关闭连接再新建连接，读取是上传文件还是下载文件
      Close(cfd);
      char up_down_choose[BUFSIZ];
      char filename[BUFSIZ];
      char new_filename[BUFSIZ + 5];
      char buf[BUFSIZ];

      bzero(up_down_choose, sizeof(up_down_choose));
      bzero(filename, sizeof(filename));

      cfd = Accept(lfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
      Read(cfd, up_down_choose, sizeof(up_down_choose));

      // 获得文件名
      int i = 3, j = 0;
      while (up_down_choose[i] != '\0') {
        filename[j] = up_down_choose[i];
        i++;
        j++;
      }
      filename[strlen(filename) - 1] = '\0';

      if (up_down_choose[1] == 'u') {
        printf("cfd 为 %d 的客户端想要上传文件\n", cfd);
	sprintf(new_filename, "up-%s", filename);
	FILE *fp = Fopen(new_filename, "w");
	bzero(buf, sizeof(buf));
	while ((res = Read(cfd, buf, sizeof(buf))) != 0) {
	  fputs(buf, fp);
	  bzero(buf, sizeof(buf));
	}
	Fclose(fp);
	printf("上传成功\n");
      } else if (up_down_choose[1] == 'd') {
        printf("cfd 为 %d 的客户端想要下载文件\n", cfd);
        FILE *fp = Fopen(filename, "r");
        bzero(buf, sizeof(buf));
        while(fgets(buf, sizeof(buf), fp) != NULL) {
	  Write(cfd, buf, sizeof(buf));
	  bzero(buf, sizeof(buf));
	}
	Fclose(fp);
	Close(cfd);
      }
    }
  }
  return 0;
}

void *chat(void *arg) {
  struct cli_info *c = (struct cli_info *)arg;
  char buf[BUFSIZ];
  char prefix[BUFSIZ + 2];
  char history[BUFSIZ * 4];
  FILE *fp = NULL;

  sprintf(prefix, "%s| ", (*c).name);
  int res = 0;
  for ( ; ; ) {
    bzero(buf, sizeof(buf));
    bzero(history, sizeof(history));
    fp = Fopen("chat_history.txt", "a+");
    res = Read((*c).cfd, buf, sizeof(buf));
    
    if (res == 0) {
      printf("cfd 为 %d 的客户端已经断开连接\n", (*c).cfd);
      break;
    }
    if (strlen(buf) == 3 && buf[0] == ':' && buf[1] == 'q') {
      break;
    }
    if (buf[0] == ':' && buf[2] == ' ') {
      // 私聊
      char username[BUFSIZ];
      char real_info[BUFSIZ];
      bzero(username, sizeof(username));
      bzero(real_info, sizeof(real_info));
      if (buf[1] == 'u') {
        int i = 3, j = 0;
	while (buf[i] != ' ') {
	  username[j] = buf[i];
	  i++;
	  j++;
	}
	username[++j] = '\0';
	
	++i; j = 0;
	while (buf[i] != '\0') {
	  real_info[j] = buf[i];
	  i++;
	  j++;
	}
	real_info[++j] = '\0';

	// 查找私聊的用户是否存在
	for (int i = 0; i < CLI_CNT; i++) {
	  if ((strcmp(cinfos[i].name, username) == 0) && (username[0] != '\0')) {
	      if (strlen(real_info) != 0) {
	      sprintf(
	              history,
	             "\n<< type: pri, from: %s, to: %s, msg: %s",
		      (*c).name,
	  	      cinfos[i].name,
	  	      real_info
              );
	      fputs(history, fp);
              Write(cinfos[i].cfd, (void *)pri_chat_style, strlen(pri_chat_style));
	      Write(cinfos[i].cfd, prefix, strlen(prefix));
	      Write(cinfos[i].cfd, real_info, strlen(real_info));
	    }
	    break;
	  }
	}
      }
    } else {
      // 群聊
      // 转发消息到除了发送消息之外的客户端
      for (int i = 0; i < CLI_CNT; i++) {
        if ((cinfos[i].cfd != (*c).cfd) && (cinfos[i].cfd != 0)) { 
            if (strlen(buf) != 0) {
	    sprintf(
                    history,
                   "\n<< type: gro, from: %s, to: all, msg: %s",
                    (*c).name,
                    buf
            );
            fputs(history, fp);
            Write(cinfos[i].cfd, (void *)gro_chat_style, strlen(gro_chat_style));
            Write(cinfos[i].cfd, prefix, strlen(prefix));
            Write(cinfos[i].cfd, buf, res);
	  }
        }
      }
    }
    Fclose(fp);
  }

  Close((*c).cfd);
  return (void *)0;
}
