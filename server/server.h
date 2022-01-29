#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include<sys/utsname.h>

#include <pthread.h>
#include <time.h>
#include "../socket.h"

//处理客户端的消息
void ProcessMess(int connfd, char *userName, char *msg);
//服务端为每个客户端新建一个线程
void *clientThread(void *arg);
//发送消息至客户端
void sendMess(int connfd, char *msg, char *userName);

/* itoa:实现整数到char数组的转换 */
char *itoa(int num, char *str)
{ /*索引表*/
    char index[] = "0123456789";
    unsigned unum; /*中间变量*/
    int i = 0, j, k;
    /*确定unum的值*/
    if (num < 0) /*十进制负数*/
    {
        unum = (unsigned)-num;
        str[i++] = '-';
    }
    else
        unum = (unsigned)num; /*其他情况*/
    /*转换*/
    do
    {
        str[i++] = index[unum % 10];
        unum /= 10;
    } while (unum);
    str[i] = '\0';
    /*逆序*/
    if (str[0] == '-')
        k = 1; /*十进制负数*/
    else
        k = 0;

    for (j = k; j <= (i - 1) / 2; j++)
    {
        char temp;
        temp = str[j];
        str[j] = str[i - 1 + k - j];
        str[i - 1 + k - j] = temp;
    }
    return str;
}
