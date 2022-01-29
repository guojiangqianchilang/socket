#include "server.h"

struct Client
{
    int connfd;                  //socket fd
    char userName[MAX_LEN_NAME]; //nickname
    char logInIdStr[16];         //标识符
} user[BACKLOG];

int breakFlags[100]; //是否收到中断连接信息

int usersNum = 0; //全局变量，表示目前连接客户端的数量
int LogInId = 0;  //表示服务器收到了几次连接请求

int main()
{
    int sockfd, new_fd;          /*socket句柄和建立连接后的句柄*/
    struct sockaddr_in my_addr;  /*本方地址信息结构体，下面有具体的属性赋值*/
    struct sockaddr_in cli_addr; /*对方地址信息*/
    int sin_size;

    puts("Info: Initilizing socket...");
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //建立socket
    if (sockfd == -1)
    {
        printf("socket failed:%d", errno);
        return -1;
    }

    puts("Info: Binding socket...");
    my_addr.sin_family = AF_INET;                /*该属性表示接收本机或其他机器传输*/
    my_addr.sin_port = htons(PORT);              /*端口号*/
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*IP，accept any incoming messages.*/
    bzero(&(my_addr.sin_zero), 8);               /*将其他属性置0*/
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0)
    { //绑定地址结构体和socket
        printf("bind error");
        return -1;
    }
    puts("Info: Listening socket...");
    listen(sockfd, BACKLOG); //开启监听 ，第二个参数是最大监听数
    puts("Info: Offering service...");
    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &sin_size); //在这里阻塞知道接收到消息，参数分别是socket句柄，接收到的地址信息以及大小
        if (new_fd == -1)
        {
            printf("receive failed");
        }
        else
        {
            printf("Info: Creating thread with new_fd = %d\n", new_fd);
            pthread_t pid;
            send(new_fd, "Hello World!", 12, 0); //发送内容，参数分别是连接句柄，内容，大小，其他信息（设为0即可）
            pthread_create(&pid, NULL, clientThread, &new_fd);
        }
    }
    return 0;
}

void ProcessMess(int connfd, char *userName, char *msg)
{

    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);
    time_t timep;
    char hostname[20];
    int i = 2, count = 0;
    while (strlen(msg) && msg[0] == ':')
    {
        switch (msg[1])
        {
        case 't': //获取时间
            //time_t timep;
            memset(buf, 0, MAX_DATA);
            time(&timep);
            struct tm *date = gmtime(&timep);
            sprintf(buf, ":t|%.2d/%.2d/%.2d %.2d-%.2d-%.2d",
                    date->tm_mon + 1,
                    date->tm_mday,
                    date->tm_year + 1900,
                    date->tm_hour,
                    date->tm_min,
                    date->tm_sec);
            // buf[strlen(buf) - 1] = '\0';
            printf("Info[%d]: Send \"%s\" \n", connfd, buf + 3);
            send(connfd, buf, strlen(buf), 0);
            count++;
            msg = msg + 2;
            break;
        case 'n': //获取hostname
            // GetName();
            //char hostname [20];
            gethostname(hostname, 20);
            sprintf(buf, ":n|%s", hostname);
            printf("Info[%d]: Send \"%s\" \n", connfd, buf + 3);
            send(connfd, buf, strlen(buf), 0);
            msg = msg + 2;
            break;
        case 'l': //获取客户端列表
            sprintf(buf, ":l|");
            char cli[128];
            for (int i = 0; i < usersNum; i++)
            {
                memset(cli, 0, 128);
                sprintf(cli, "%s|%s", user[i].userName, user[i].logInIdStr);
                strcat(buf, cli);
                if (i != usersNum - 1)
                {
                    strcat(buf, "|");
                }
            }
            printf("Info[%d]: Send \"%s\" \n", connfd, buf + 3);
            send(connfd, buf, strlen(buf), 0);
            msg = msg + strlen(msg);
            break;
        case 'd':
            breakFlags[connfd] = 1;
            msg = msg + 2;
            break;
        case 's':
            // :s|id|msg
            sendMess(connfd, msg, userName);
            msg = msg + strlen(msg);
            break;
        default:
            printf("No message type \":%c\"\n\n", msg[1]);
            break;
        }
    }
}

void sendMess(int connfd, char *msg, char *userName)
{
    char buf[MAX_DATA];
    memset(buf, 0, MAX_DATA);

    char id[16];
    char info[MAX_DATA];
    char *pch = strtok(msg + 2, "|");
    strcpy(id, pch);
    pch = strtok(NULL, "|");
    strcpy(info, pch);
    info[strlen(info) - 1] = '\0';
    // printf("DEBUG: %s %s",id,info);

    // :s|from:   name|message
    sprintf(buf, ":s|from:%s|%s", userName, info);
    int i = 0;
    for (; i < usersNum; i++)
    {
        if (strcmp(user[i].logInIdStr, id) == 0)
        {
            break;
        }
    }
    printf("Info[%d]: Send \"%s\" to %d[id = %s] \n", connfd, buf + 3, i, id);
    if (send(user[i].connfd, buf, strlen(buf), 0) == -1)
    {
        //发送失败数据包
        send(connfd, ":so", 4, 0);
    }
    else
    {
        //发送成功数据包
        send(connfd, ":sx", 4, 0);
    }
}

void *clientThread(void *arg)
{
    int connfd = *(int *)arg;
    printf("Info[%d]: New thread created.\n", connfd);
    user[usersNum].connfd = connfd;
    char userName[MAX_LEN_NAME] = {0};
    if (recv(connfd, userName, sizeof(userName), 0) > 0 && userName[0] != '\0')
    {
        strcpy(user[usersNum].userName, userName);
    }
    else
    {
        strcpy(user[usersNum].userName, "anonymous");
    }
    printf("Info[%d]: userName: %s\n", connfd, userName[0] ? userName : "anonymous");
    itoa(LogInId, user[usersNum].logInIdStr);
    ++LogInId;
    ++usersNum;

    while (1)
    {
        if (breakFlags[connfd]) // client exit
        {
            printf("Info[%d]: Client disconnected.\n", connfd);
            int i;
            int serNum;
            for (i = 0; i < usersNum; ++i)
            {
                if (user[i].connfd == connfd)
                {
                    serNum = i;
                    break;
                }
            }
            printf("Info[%d]: Changing users list...\n", connfd);
            --usersNum;
            for (i = serNum; i < usersNum; ++i)
            {
                strcpy(user[i].userName, user[i + 1].userName);
                user[i].connfd = user[i + 1].connfd;
            }
            printf("Info[%d]: Closing socket...\n", connfd);
            breakFlags[connfd] = 0;
            close(connfd);
            break;
        }

        char buf[1024] = {0};
        int recvRet = recv(connfd, buf, sizeof(buf), 0);
        int lastBeatTime;
        int i;

        if (recvRet > 0)
        {
            printf("Info:    Received %s\n", buf);
            ProcessMess(connfd, userName, buf);
        }
        else // client exit
        {
            printf("Info[%d]: Client disconnected.\n", connfd);
            int i;
            int serNum;
            for (i = 0; i < usersNum; ++i)
            {
                if (user[i].connfd == connfd)
                {
                    serNum = i;
                    break;
                }
            }
            printf("Info[%d]: Changing users list...\n", connfd);
            --usersNum;
            for (i = serNum; i < usersNum; ++i)
            {
                strcpy(user[i].userName, user[i + 1].userName);
                user[i].connfd = user[i + 1].connfd;
            }
            printf("Info[%d]: Closing socket...\n", connfd);
            breakFlags[connfd] = 0;
            close(connfd);
            break;
        }
    }
    printf("Info[%d]: Thread ended.\n", connfd);
}
