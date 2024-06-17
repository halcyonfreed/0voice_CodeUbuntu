#include<socket.h>
#include<netinet/in.h>
#include<string.h>
#include<errno.h>
#include<epoll.h>

#define BUFFER_LENGTH 1024
typedef int (*RCALLBACK)(int fd); //用于定义一个函数指针类型RCALLBACK。它表示一个接受一个int类型参数fd并返回int类型的函数指针

// global variable
// 抄epoll里构建事件, 把io加入epoll里 不一样封装到set_event里
// int epfd = epoll_create(1); //不可以这样写，在里面改值
int epfd = 0;


struct conn{
    int fd; //io是fd

    char rbuffer[BUFFER_LENGTH];
    int rlength;
    char wbuffer[BUFFER_LENGTH];
    int wlength;

    RCALLBACK send_callback;

    // epollin事件对应callback要么accept或者recv
    /*
    这段代码定义了一个联合体（union）r_action，它包含两个成员：accept_callback和recv_callback，它们都是RCALLBACK类型的函数指针。
    在使用union时，各成员共享同一块内存空间，只能同时使用其中的一个成员。不同成员可以存储不同类型的数据，但在任意给定时间点上只能使用一个成员。
    该联合体的目的可能是为了提供某种灵活性，在特定情况下可以选择调用accept_callback或recv_callback函数来处理相应的操作。
    */
    union{
        RCALLBACK accept_callback;
        RCALLBACK recv_callback;
    }r_action;


}


int init_server(unsigned short port){
    // 创建抄network里
    // 创建socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 绑定本地端口
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htons(INADDR_ANY); //绑网卡地址 默认0.0.0.0 绑本地地址；htons转成网络字节序 
    servaddr.sin_port = htons(port); //改这里 0-1023系统默认 大于1024都能用

    if(-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr))){
        printf("bind failed: %s\n", strerror(errno));
    }
    
    listen(sockfd, 10);
    printf("listen finished: %d\n", sockfd);

    return sockfd;
}


int set_event(int fd, int event){
    
    struct epoll_event ev; //构建事件，只用来add和delete，control里没用
    ev.events = event;  //改这里不用epoll，reactor用event
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev); 

}

int main(){
    unsigned short port = 2000;
    int sockfd = init_server(port);

    epfd = epoll_create(1);
    set_event(sockfd, EPOLLIN);



}