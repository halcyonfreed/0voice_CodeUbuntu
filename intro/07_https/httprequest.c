
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 网络编程的.h
#include <sys/socket.h> 
//网络地址 下面两个
#include <netinet/in.h> 
#include <arpa/inet.h> 
// 标准unix库
#include <unistd.h> 


#include <netdb.h>
#include <fcntl.h> //非阻塞IO



#define BUFFER_SIZE 4096
#define HTTP_VERSION "HTTP/1.1"
#define CONNECTION_TYPE "Connection: close\r\n"

// 1 不像上次DNS翻译太麻烦，用接口gethostbyname
char *host_to_ip(const char* hostname){
    struct hostent *host_entry = gethostbyname(hostname);

    // 点分十进制， 14.215.177.39--->unsigned int
    //  inet_ntoa: unsigned int-->char *    0x121212--->"18.18.18.18"
    if(host_entry) return  inet_ntoa(*(struct in_addr*)host_entry->h_addr_list); 
    return NULL;

}


// 2 连接服务器
int http_create_socket(char *ip){
    // TCP连接必须先建立一个socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port= htons(80);
    sin.sin_addr.s_addr = inet_addr(ip); //ip变成域名反过来
    
	if (0 != connect(sockfd, (struct sockaddr*)&sin, sizeof(struct sockaddr_in))) {
		return -1;
	}
    fcntl(sockfd, F_SETFL, O_NONBLOCK); //阻塞非阻塞

	return sockfd;
}

// 3 请求http
// hostname: github.com
// resource /wangbojing
char * http_send_request(const char *hostname, const char *resource) {
    char *ip = host_to_ip(hostname); //name-->ip
    int sockfd = http_create_socket(ip); //创建一个socket tcp连接

    char buffer[BUFFER_SIZE] ={0};
    sprintf(buffer, 
    "GET %s %s\r\n\
    HOST: %s\r\n\
    %s\r\n\
    \r\n",
    resource, HTTP_VERSION, 
    hostname,
    CONNECTION_TYPE
    );

    send(sockfd,buffer, strlen(buffer), 0);

    // 3 获取response， 因为非阻塞，可能还没返回所以recv是空的
    // select
    fd_set fdread; 
    FD_ZERO(&fdread);
    FD_SET(sockfd, &fdread);

    struct timeval tv;
    tv.tv_sec = 5; //等待秒数
    tv.tv_usec = 0; //等待微秒数

    char *result = malloc(sizeof(int));  //malloc的数据一定要memset0防止混乱数据，最后要free
	memset(result, 0, sizeof(int));
	

    while(1){
        int selection = select(sockfd+1, &fdread, NULL, NULL, &tv); // 可读maxfd+1， 可读集合&rset，可写集合&wset, 错误集合&eset, 多长时间遍历一次所有IO 
		if (!selection || !FD_ISSET(sockfd, &fdread)) {
			break;
		} else{
            memset(buffer,  0, BUFFER_SIZE);
            int len = recv(sockfd, buffer, BUFFER_SIZE, 0); //sockfd读到buffer
            if(len ==0) break; //disconnect
            
            result = realloc(result, (strlen(result) + len + 1) * sizeof(char));
            strncat(result, buffer, len);
        }
    }
    return result;
}

int main(int argc, char *argv[]) {

	if (argc < 3) return -1;

	char *response = http_send_request(argv[1], argv[2]);
	printf("response : %s\n", response);

	free(response);
	
}