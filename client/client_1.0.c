/*
2021/12/2:处理不好disconnect的情况，先存个档
*/
#include "client.h"

struct Client
{
    int connfd;          //socket fd
    char userName[16];   //nickname
    char logInIdStr[16]; //标识符
} user[BACKLOG];

int usersNum = 0; //全局变量，表示目前连接到服务器的客户端的数量

char nickname[20];            //昵称
struct sockaddr_in dest_addr; /*目标地址信息*/
int isConnect;                //是否连接成功

int main()
{
    printf("Please input a nickname:\n");
    scanf("%s", &nickname);
    getc(stdin); //读取上次输入留下的换行符

    HelpInfo(); //输出帮助信息

    printf("Please enter the message ");
    //采用ipv4，流式传输，采用流式传输的默认协议
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); /*建立socket*/
    if (sockfd == -1)
    {
        printf("socket failed:%d\n", errno);
    }

    dest_addr.sin_family = AF_INET;                 /*该属性表示接收本机或其他机器传输*/
    dest_addr.sin_port = htons(DEST_PORT);          /*端口号*/
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP); /*IP，括号内容表示本机IP*/
    bzero(&(dest_addr.sin_zero), 8);                /* Set 8 bytes of S to 0.  */

    while (getUserInput(sockfd))
        ;

    printf("The client will shutdown.\n");

    close(sockfd); //关闭socket
    return 0;
}

void *RecvThread(void *arg)
{
    int connfd = *(int *)arg;

    while (1)
    {
        char buf[1024] = {0};
        int recvRet = recv(connfd, buf, sizeof(buf), 0);
        if (recvRet > 0)
        {
            handleRecv(connfd, buf); //处理服务器发来的数据包
        }
        else
        {
            break;
        }
    }
}

void handleRecv(int connfd, char *msg)
{
    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);

    if (msg[0] == ':')
    {
        switch (msg[1])
        {
        case 't': //获取时间
            printf("Server time : %s\n", msg + 3);
            break;
        case 'n': //获取hostname
            printf("Your name : %s\n", msg + 3);
            break;
        case 'l': //获取客户端列表
            char userStr[2048];
            strcpy(userStr, msg + 3);
            usersNum = 0;
            char *pch = strtok(userStr, "|");
            while (pch)
            {
                strcpy(user[usersNum].userName, pch);
                // puts(user[usersNum].nickname);
                pch = strtok(NULL, "|");
                strcpy(user[usersNum].logInIdStr, pch);
                // puts(user[usersNum].logInIdStr);
                ++usersNum;
                pch = strtok(NULL, "|");
            }
            int i;
            printf("\n       Nickname    |LogInId\n");
            for (i = 0; i < usersNum; ++i)
            {
                printf("%3d:%15s|%7s\n", i + 1, user[i].userName, user[i].logInIdStr);
            }
            putchar('\n');
            break;
        case 's':
            // SendMess();
            break;
        default:
            printf("No message type \":%c\"\n\n", msg[1]);
            break;
        }
    }
}

void HelpInfo()
{
    FILE *fp = fopen("info_start", "r");
    if (NULL == fp) //以返回值fp判断是否打开成功，如果为NULL表示失败
    {
        printf("Failed to open the file !\n");
        exit(0);
    }
    char buffer;
    while (!feof(fp))
    {
        fscanf(fp, "%c", &buffer);
        printf("%c", buffer);
    }
    printf("\n");
}

void connectFunc(int sockfd)
{
    if (isConnect == 0)
    {
        char buf[MAX_DATA]; //储存接收数据
        //sockfd 是系统调用socket() 返回的套接字文件描述符。
        //serv_addr 是保存着目的地端口和 IP 地址的数据结构 struct sockaddr。addrlen 设置 为sizeof(struct sockaddr)。
        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
        {                                         //连接方法，传入句柄，目标地址和大小
            printf("connect failed:%d\n", errno); //失败时可以打印errno
        }
        else
        {
            printf("connect success\n");
            //可以先send数据包，根据数据包的类型返回相应的内容
            recv(sockfd, buf, MAX_DATA, 0); //将接收数据打入buf，参数分别是句柄，储存处，最大长度，其他信息（设为0即可）。
            printf("Received:%s\n", buf);

            //开启一个线程负责接受消息
            pthread_t pid;
            pthread_create(&pid, NULL, RecvThread, &sockfd);

            isConnect = 1;
            send(sockfd, nickname, strlen(nickname), 0);
        }
    }
}

void DisconnectFunc(int sockfd)
{
    // printf("isConnect = %d\n",*isConnect);
    if (isConnect)
    {
        send(sockfd, ":d", 2, 0);
        isConnect = 0;

        printf("Disconnct from the server.\n");
        // close(sockfd);
    }
}

void GetTime(int sockfd)
{
    char buf[MAX_DATA];
    memset(buf, 0, sizeof(buf));

    //先发送一个请求数据包
    sprintf(buf, ":t");
    send(sockfd, buf, strlen(buf), 0);
}

void GetName(int sockfd)
{
    char buf[MAX_DATA];
    memset(buf, 0, sizeof(buf));

    //先发送一个请求数据包
    sprintf(buf, ":n");
    send(sockfd, buf, strlen(buf), 0);
}

void GetList(int sockfd)
{
    char buf[MAX_DATA];
    memset(buf, 0, sizeof(buf));

    //先发送一个请求数据包
    sprintf(buf, ":l");
    send(sockfd, buf, strlen(buf), 0);
}

int getUserInput(int sockfd)
{
    int rtnCode = 1;

    char buffer[256];
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    if (buffer[0] == ':')
    {
        switch (buffer[1])
        {
        case 'c': //连接
            connectFunc(sockfd);
            break;
        case 'd': //断开连接
            // printf("isConnect = %d\n",isConnect);
            DisconnectFunc(sockfd);
            break;
        case 't': //获取时间
            GetTime(sockfd);
            break;
        case 'n': //获取hostname
            GetName(sockfd);
            break;
        case 'l': //获取客户端列表
            GetList(sockfd);
            break;
        case 's':
            // SendMess();
            break;
        case 'q':
            DisconnectFunc(sockfd);
            rtnCode = 0; //quit
            break;
        case 'h':
            HelpInfo();
            break;
        case '\n':
        case '\0':
            puts("Command Imcomplete\n");
            break;
        default:
            printf("No Command \":%c\"\n\n", buffer[1]);
            break;
        }
    }

    return rtnCode;
}
