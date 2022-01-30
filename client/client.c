/**
 * 更新日志：成功添加断开连接功能(2021/12/02)
 * 成功实现基本功能：(2021/12/03)
 *  1.连接服务端
 *  2.断开连接
 *  3.获取hostname
 *  4.获取服务器时间
 *  5.获取连接上的服务器的客户端列表
 *  6.向客户端列表中的某个客户端传输数据包
 * 
 * */
#include "client.h"

int count = 0;

struct Client
{
    int connfd;                  //socket fd
    char userName[MAX_LEN_NAME]; //nickname
    char logInIdStr[16];         //标识符
} user[BACKLOG];

int usersNum = 0;   //全局变量，表示目前连接到服务器的客户端的数量

char nickname[20];            //昵称
struct sockaddr_in dest_addr; /*目标地址信息*/
int isConnect;                //是否连接成功

int main()
{
    printf("Please input a nickname(max_length = 16):\n");
    scanf("%s", &nickname);
    getc(stdin); //读取上次输入留下的换行符

    HelpInfo(); //输出帮助信息

    printf("Please enter the message ");

    dest_addr.sin_family = AF_INET;                   /*该属性表示接收本机或其他机器传输*/
    dest_addr.sin_port = htons(PORT);                 /*端口号*/
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP); /*IP，括号内容表示目的服务器的IP*/
    bzero(&(dest_addr.sin_zero), 8);                  /* Set 8 bytes of S to 0.  */

    while (getUserInput())
        ;

    printf("The client will shutdown.\n");

    return 0;
}

//接受来自服务端信息的线程
void *RecvThread(void *arg)
{
    //获取当前连接的socket标识符
    int connfd = *(int *)arg;

    while (1)
    { //接收循环
        char buf[MAX_DATA] = {0};
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

//处理服务器发来的数据包
void handleRecv(int connfd, char *msg)
{
    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);

    char *pch;
    char userStr[2048];
    char name[MAX_LEN_NAME];
    char info[MAX_DATA];
    char msg2[MAX_DATA];
    char tmp[MAX_DATA];
    int i, index = 0;
    // printf("------%s-------",msg);

    while (strlen(msg) && msg[0] == ':')
    {
        switch (msg[1])
        {
        case 't': //获取时间
            //数据包格式   :t|time :t|time :t|time
            memset(tmp, 0, MAX_DATA);
            for (i = 3; i < strlen(msg); i++)
            {
                if (msg[i] == ':')
                    break;
                tmp[i - 3] = msg[i];
            }
            printf("[count = %d]Server time : %s\n", count, tmp);
            count++;
            msg = msg + i;
            break;
        case 'n': //获取hostname
            //数据包格式   :n|hostname
            memset(tmp, 0, MAX_DATA);
            for (i = 3; i < strlen(msg); i++)
            {
                if (msg[i] == ':')
                    break;
                tmp[i - 3] = msg[i];
            }
            printf("Your name : %s\n", tmp);
            msg = msg + i;
            break;
        case 'l': //获取客户端列表
            //数据包格式   :l| username1 | id1 | username2 | id2 ...
            //char userStr[2048];
            strcpy(userStr, msg + 3);
            usersNum = 0;
            pch = strtok(userStr, "|");
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
            // int i;
            printf("\n       Nickname     |     Id\n");
            for (i = 0; i < usersNum; ++i)
            {
                printf("%3d:%16s|%7s\n", i + 1, user[i].userName, user[i].logInIdStr);
            }
            putchar('\n');
            msg = msg + strlen(msg);
            break;
        case 's':
            //char name[MAX_LEN_NAME];
            //char info[MAX_DATA];
            //char msg2[MAX_DATA];
            //int index = 0;
            if (msg[2] == '|')
            {
                pch = strtok(msg + 2, ":");
                pch = strtok(NULL, "|");
                strcpy(name, pch);
                pch = strtok(NULL, "|");
                strcpy(info, pch);
                printf("Message[from:%s]: %s\n", name, info);
            }
            else if (msg[2] == 'x')
            {
                //(成功发送)数据包格式   :sx
                puts("\nInfo: Message sended.\n");
            }
            else
            {
                //(发送失败)数据包格式   :so
                puts("\nError: Message not sended.\n");
            }
            msg = msg + strlen(msg);
            break;
        default:
            printf("No message type \":%c\"\n\n", msg[1]);
            break;
        }
    }
}

//输出提示信息
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

//连接服务端，需要返回socket标识符
int connectFunc()
{
    int sockfd = 0;
    if (isConnect == 0) //判断当前状态为未连接
    {
        //采用ipv4，流式传输，采用流式传输的默认协议
        sockfd = socket(AF_INET, SOCK_STREAM, 0); /*建立socket*/
        if (sockfd == -1)
        {
            printf("socket failed:%d\n", errno);
        }
        printf("sockfd = %d\n", sockfd);
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
    else
    {
        return -1; //已经连接上，不需要再次连接
    }
    return sockfd;
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

    //发送请求数据包
    // for (int i = 0; i < 100; i++)
    // {
        sprintf(buf, ":t");
        send(sockfd, buf, strlen(buf), 0);
    // }
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

//获取用户的输入并处理
int getUserInput()
{
    static int sockfd; //这个是连接上服务端后保存的标识符
    // printf("DEBUG: sockfd = %d\n",sockfd);
    int rtnCode = 1;

    char buffer[256];
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    int tmp;
    if (buffer[0] == ':')
    {
        switch (buffer[1])
        {
        case 'c': //连接
            tmp = connectFunc();
            if (tmp == -1)
            {
                printf("You have connected already!");
            }
            else
            {
                sockfd = tmp;
            }
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
        case 's': //发送信息
            SendMsg(sockfd);
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

void SendMsg(int sockfd)
{
    //需要用户输入目标client的id
    printf("The destination client id:");
    char id[16];
    fgets(id, 16, stdin);
    id[strlen(id) - 1] = '\0';
    // getc(stdin);
    //接着输入要发送的内容
    printf("The message you want to transfer:");
    char msg[MAX_DATA - 16 - 3];
    char msg2[MAX_DATA - 16 - 3]; //进行转义
    int index = 0;
    fgets(msg, MAX_DATA - 16 - 3, stdin);

    //封装请求包
    char buf[MAX_DATA];
    memset(buf, 0, sizeof(buf));

    //先发送一个请求数据包
    sprintf(buf, ":s|%s|%s", id, msg);
    // printf("DEBUG: %s", buf);
    send(sockfd, buf, strlen(buf), 0);
}
