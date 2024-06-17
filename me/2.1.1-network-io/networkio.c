#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<netinet/in.h>
#include<pthread.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include<sys/select.h>

void *client_thread(void *arg){
    // 为了防止多路io阻塞，开一个线程
    int clientfd = *(int *) arg; //先转成int *，强制转换成指向int，再取arg指针内容最外面*

    // 收多条 while
    while(1){
        char buffer[1024] = {0};
        int count = recv(clientfd, buffer, 1024, 0); //Read N bytes into BUF from socket FD.
        
        if (count == 0) { // disconnect
			printf("client disconnect: %d\n", clientfd);
			close(clientfd);
			break;
		}
        
        //Read N bytes into BUF from socket FD.
        printf("recv: %s\n", buffer);

        // client回传server收到的buffer信息，server会打印收到的信息，client端打印buffer大小
        count = send(clientfd, buffer, count, 0);
        printf("send: %d\n", count);
    }
}


int main(){
    // 创建socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 绑定本地端口
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htons(INADDR_ANY); //绑网卡地址 默认0.0.0.0 绑本地地址；htons转成网络字节序 
    servaddr.sin_port = htons(2000); // 0-1023系统默认 大于1024都能用

    if(-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr))){
        printf("bind failed: %s\n", strerror(errno));
    }
    
    listen(sockfd, 10);
    printf("listen finished: %d\n", sockfd);

    // 分配io
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);


#if 0
// 第一版，屏蔽掉
    // 前面sock fd，Await a connection on socket FD. When a connection arrives, open a new socket to communicate with it,
    printf("accept\n");
    int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
    printf("accept finished\n");

    char buffer[1024] = {0};
    int count = recv(clientfd, buffer, 1024, 0); //Read N bytes into BUF from socket FD.
    //Read N bytes into BUF from socket FD.
    printf("recv: %s\n", buffer);

    // client回传server收到的buffer信息，server会打印收到的信息，client端打印buffer大小
    count = send(clientfd, buffer, count, 0);
    printf("send: %d\n", count);
#elif 0
// 一直只要有连接，就分配io，来accept send
    while(1){
        // 前面socfd，Await a connection on socket FD. When a connection arrives, open a new socket to communicate with it,
        printf("accept\n");
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        printf("accept finished\n");

        char buffer[1024] = {0};
        int count = recv(clientfd, buffer, 1024, 0); //Read N bytes into BUF from socket FD.
        //Read N bytes into BUF from socket FD.
        printf("recv: %s\n", buffer);

        // client回传server收到的buffer信息，server会打印收到的信息，client端打印buffer大小
        count = send(clientfd, buffer, count, 0);
        printf("send: %d\n", count);
    }

#elif 0
        while(1){
            printf("accept\n");
            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            printf("accept finished\n");

            // 启动线程
            pthread_t thid; // thread id
            pthread_create(&thid, NULL, client_thread, &clientfd); //线程id，属性，入口函数，线程值
    }
    
#elif 0

    fd_set rfds, rset; 
    //rfds返回数据dataset, 是应用层的，用户设置的
    // rset是复制rfds的， 用于被复制到内核空间，用于判断的

    FD_ZERO(&rfds); //先清空
    FD_SET(sockfd, &rfds); // 再把sockfd 设置在可读rfds里，置1

    int maxfd = sockfd; //用来方便遍历set用到的最大值

    while(1){
        rset = rfds; //关联，


        // maxfd +1因为下标从0开始，数量比最后多1
        //  int select(int nfds, fd_set *readfds, fd_set *writefds,
        //   fd_set *exceptfds, struct timeval *timeout);
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL); //返回就绪的fd数量，就绪的bit位是1

        if(FD_ISSET(sockfd, &rset)){//sockfd位是否置1
            // accept 如果监听的sockfd置1了，就开始accept连接
            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            printf("accept finished: %d\n", clientfd);

            FD_SET(clientfd, &rfds); //有一个fd，就set一下，maxfd也变了
            if(maxfd < clientfd) maxfd = clientfd; //clientfd当client断连了，会被回收，所以要判断一下
        }

        // recv
        int i = 0;
        for(i = sockfd +1; i<= maxfd; i++){ //i就是fd
            if (FD_ISSET(i, &rset)){//判i可读rset吗
                char buffer[1024] = {0};
                int count = recv(i, buffer, 1024, 0); //Read N bytes into BUF from socket FD.
                
                if (count == 0) { // disconnect
                    printf("client disconnect: %d\n", i);
                    close(i);
                    FD_CLR(i, &rfds); //FD_CLR是一个宏，用于从fd_set数据结构中清除指定的文件描述符
                    continue;
                }
		        
				printf("RECV: %s\n", buffer);

				count = send(i, buffer, count, 0);
				printf("SEND: %d\n", count);
			}
		}
	}

#elif 0
    //  进pollfd 看参数
    struct pollfd fds[1024]= {0}; //fdset就是
    fds[sockfd].fd = sockfd;
    fds[sockfd].events= POLLIN; //pollin就是可读，设置为POLLIN表示对该文件描述符上是否有可读数据感兴趣

    int maxfd = sockfd; //来遍历用的，检查哪个fd set了 
    while(1){
        int nread = poll(fds, maxfd + 1, -1);//set， set大小， timeout =-1一直阻塞等待
        if (fds[sockfd].revents & POLLIN){
            // pollin是x十六进制0x0001，变成8位2进制00000001
           
           
            // 如果有可读的，用accept处理分配io，复制上面
            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            printf("accept finished: %d\n", clientfd);

            // FD_SET(clientfd, &rfds); //select的这句改成poll的下面两行
            fds[clientfd].fd= clientfd;
            fds[clientfd].events=POLLIN;

            if(maxfd < clientfd) maxfd = clientfd; //clientfd当client断连了，会被回收，所以要判断一下
        }

            // 抄上面recv
            int i = 0;
            for(i = sockfd +1; i<= maxfd; i++){ //i就是fd
                // if (FD_ISSET(i, &rset)){//判i可读rset吗 select有，poll没有
                if(fds[i].revents & POLLIN){ //判i可读吗，和Pollin位与
                    char buffer[1024] = {0};
                    int count = recv(i, buffer, 1024, 0); //Read N bytes into BUF from socket FD.
                    
                    if (count == 0) { // disconnect
                        printf("client disconnect: %d\n", i);
                        close(i);
                        // FD_CLR(i, &rfds); //FD_CLR是fdset里的一个宏，select有；poll没有
                        fds[i].fd= -1; //因为从0开始，置-1
                        fds[i].events= 0;
                        continue;
                    }
                    
                    printf("RECV: %s\n", buffer);

                    count = send(i, buffer, count, 0);
                    printf("SEND: %d\n", count);
                }
            }
    }

#else
    int epfd = epoll_create(1);

    struct epoll_event ev; //构建事件，只用来add和delete，control里没用
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); //control


    while(1){
        struct epoll_event events[1024] = {0};
        int nready = epoll_wait(epfd, events, 1024, -1);

		int i = 0;
		for (i = 0;i < nready;i ++) {
			int connfd = events[i].data.fd;
			if (connfd == sockfd) {
                
                
                // accept				
				int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
				printf("accept finshed: %d\n", clientfd);
                // 创建events， 添到ctl里
				ev.events = EPOLLIN;
				ev.data.fd = clientfd;
                // 这里ev不写也可以
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
				
			} else if (events[i].events & EPOLLIN) {

				char buffer[1024] = {0};
				
				int count = recv(connfd, buffer, 1024, 0);
				if (count == 0) { // disconnect
					printf("client disconnect: %d\n", connfd);
					close(connfd);
                    // 改了这里
					epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
					// epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, &ev); 也可以
					
					continue;
				}

				printf("RECV: %s\n", buffer);

				count = send(connfd, buffer, count, 0);
				printf("SEND: %d\n", count);

			}

		}

	}
	
	

#endif


    getchar();// 阻塞在这里，一直等命令行给东西进来

    printf("exit\n");

    
    return 0;
}