#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "../socket.h"

void handleRecv(int connfd, char *msg); //处理服务器发来的数据包
void *RecvThread(void *arg);            //接受来自服务端信息的线程
void HelpInfo();                        //输出提示信息
int connectFunc();                      //连接服务端
void DisconnectFunc(int sockfd);        //断开连接
void GetTime(int sockfd);               //封装获取时间的数据包
void GetName(int sockfd);               //封装获取HOSTNAME的数据包
void GetList(int sockfd);               //封装获取客户端列表的数据包
void SendMsg(int sockfd);               //封装发送消息的数据包
int getUserInput();                     //获取用户的输入并处理
