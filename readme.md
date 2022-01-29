# socket

> 浙江大学计算机网络lab6

gcc client.c -o client -pthread
gcc server.c -o server -pthread

netstat -anp | grep 5300

```c
//结构体
struct socketaddr_in
{
    unsigned short int sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};
```

1、int socket(int family,int type,int protocol)
family:
    指定使用的协议簇:AF_INET（IPv4） AF_INET6（IPv6） AF_LOCAL（UNIX协议） AF_ROUTE（路由套接字） AF_KEY（秘钥套接字）
type:
    指定使用的套接字的类型:SOCK_STREAM（字节流套接字） SOCK_DGRAM
protocol:
    如果套接字类型不是原始套接字，那么这个参数就为0


2、int bind(int sockfd, struct sockaddr *myaddr, int addrlen)
sockfd:
    socket函数返回的套接字描述符
myaddr:
    是指向本地IP地址的结构体指针
myaddrlen:
    结构长度
struct sockaddr{
    unsigned short sa_family; //通信协议类型族AF_xx
    char sa_data[14];  //14字节协议地址，包含该socket的IP地址和端口号
};
struct sockaddr_in{
    short int sin_family; //通信协议类型族
    unsigned short int sin_port; //端口号
    struct in_addr sin_addr; //IP地址
    unsigned char si_zero[8];  //填充0以保持与sockaddr结构的长度相同
};


3、int connect(int sockfd,const struct sockaddr *serv_addr,socklen_t addrlen)
sockfd:
    socket函数返回套接字描述符
serv_addr:
    服务器IP地址结构指针
addrlen:
    结构体指针的长度


4、int listen(int sockfd, int backlog)
sockfd:
    socket函数绑定bind后套接字描述符
backlog:
    设置可连接客户端的最大连接个数，当有多个客户端向服务器请求时，收到此值的影响。默认值20


5、int accept(int sockfd,struct sockaddr *cliaddr,socklen_t *addrlen)
sockfd:
    socket函数经过listen后套接字描述符
cliaddr:
    客户端套接字接口地址结构
addrlen:
    客户端地址结构长度

    
6、int send(int sockfd, const void *msg,int len,int flags)
7、int recv(int sockfd, void *buf,int len,unsigned int flags)
sockfd:
    socket函数的套接字描述符
msg:
    发送数据的指针
buf:
    存放接收数据的缓冲区
len:
    数据的长度，把flags设置为0
