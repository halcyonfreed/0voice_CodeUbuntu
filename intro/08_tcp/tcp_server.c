
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <unistd.h> //close要


#define BUFFER_LENGTH 1024

// 法一 一请求一线程
void *client_routine(void *arg){
    int clientfd = *(int *) arg;
    while(1){
        char buffer[BUFFER_LENGTH] = {0}; 
        // 用于从客户端套接字中接收数据并将其存储到缓冲区中。clientfd：表示要接收数据的套接字文件描述符。buffer：表示接收数据的缓冲区，也就是存放接收到的数据的位置。BUFFER_LENGTH：表示期望接收的最大字节数，即缓冲区的大小。0：表示额外选项，通常设置为 0。
        int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
        if(len < 0){
            // 没数据，如果阻塞，就是一直等，返回-1
            // 在非阻塞 I/O 模式下，当没有可用数据时，recv() 函数可能返回 -1 并设置 errno 为 EAGAIN 或 EWOULDBLOCK。这表示当前没有数据可供接收，并且稍后可能会有更多数据可用。因此，这段代码的作用是检测 errno 是否等于 EAGAIN 或 EWOULDBLOCK，以判断是否需要继续等待更多数据的到达。
            if(errno == EAGAIN ||errno == EWOULDBLOCK){
                close(clientfd);
                break;
            }else if(len == 0){ //disconnect
                close(clientfd);
                break;
            }else{
                printf("recv: %s, %d byte(s)\n",buffer, len);
            }
        }
    }
} 

// 1 socket创建
int main(int argc, char*argv[]){
    if(argc<2) {
        printf("param error\n");
        return -1;
    }

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //聘请一个迎宾的listen

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family  = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY; //任意地址不确定

    // 0成功 1失败
    // 函数bind()将套接字与特定的IP地址和端口号进行绑定，以便后续接收来自该地址的连接请求。
    // 而listen()则表示开始监听连接请求，并指定最大允许等待连接队列的长度为5。这意味着服务器可以同时处理5个未处理的连接请求，超过这个数量的请求将被拒绝或排队等待处理。
    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0){
        perror("bind error");
        return -2;
    }
    if(listen(sockfd, 5) < 0){
        perror("listen error");
        return -3;
    } 

    // 2迎宾sockfd 一直等着 为客户client介绍服务员socket
    while(1){
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        
        socklen_t client_len = sizeof(client_addr);
        
        // 使用accept函数接受来自服务器监听套接字 sockfd 的客户端连接请求，并将客户端的地址信息存储在名为 client_addr 的结构体中。
        int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len); 

        // 法一 一请求一线程
        // 请求来了创建线程
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_routine, &clientfd);
    }



}



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/epoll.h>

#define BUFFER_LENGTH		1024
#define EPOLL_SIZE			1024


void *client_routine(void *arg) {

	int clientfd = *(int *)arg;

	while (1) {

		char buffer[BUFFER_LENGTH] = {0};
		int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
		if (len < 0) {
			close(clientfd);
			break;
		} else if (len == 0) { // disconnect
			close(clientfd);
			break;
		} else {
			printf("Recv: %s, %d byte(s)\n", buffer, len);
		}

	}

}


// ./tcp_server 

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Param Error\n");
		return -1;
	}
	int port = atoi(argv[1]);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY; 

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return 2;
	}

	if (listen(sockfd, 5) < 0) {
		perror("listen");
		return 3;
	}
	// 

#if 0

	while (1) {

		struct sockaddr_in client_addr;
		memset(&client_addr, 0, sizeof(struct sockaddr_in));
		socklen_t client_len = sizeof(client_addr);

		int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
		
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, client_routine, &clientfd);

	}
	
#else

	int epfd = epoll_create(1);  
	struct epoll_event events[EPOLL_SIZE] = {0};

	struct epoll_event ev;
	ev.events = EPOLLIN; 
	ev.data.fd = sockfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	
	while (1) {

		int nready = epoll_wait(epfd, events, EPOLL_SIZE, 5); // -1, 0, 5
		if (nready == -1) continue;

		int i = 0;
		for (i = 0;i < nready;i ++) {

			if (events[i].data.fd == sockfd) { // listen 

				struct sockaddr_in client_addr;
				memset(&client_addr, 0, sizeof(struct sockaddr_in));
				socklen_t client_len = sizeof(client_addr);

				int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

				ev.events = EPOLLIN | EPOLLET; 
				ev.data.fd = clientfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

			} else {

				int clientfd = events[i].data.fd;
				
				char buffer[BUFFER_LENGTH] = {0};
				int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
				if (len < 0) {
					close(clientfd);

					ev.events = EPOLLIN; 
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
					
				} else if (len == 0) { // disconnect
					close(clientfd);

					ev.events = EPOLLIN; 
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
					
				} else {
					printf("Recv: %s, %d byte(s)\n", buffer, len);
				}
				
				
			}

		}

	}
	

#endif
	
	return 0;
}




