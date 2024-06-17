#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define DNS_SERVER_PORT 53
#define DNS_SERVER_IP "114.114.114.114"



// 1 查协议报文格式
struct dns_header{
    // 长度固定
    unsigned short id;
    unsigned short flags;
    unsigned short questions;
    unsigned short answer;
    unsigned short authority;
    unsigned short additional;
};

struct dns_question {
    // 长度不固定就是name大小
	int length;
	unsigned short qtype;
	unsigned short qclass;
	unsigned char *name; // 
};


// 2 client send to dns server 填充header
int dns_create_header(struct dns_header *header){
    // 先判断传进来的参数，习惯！！！
    if(header ==NULL) return -1;
    memset(header, 0, sizeof(struct dns_header)); //接着，使用 memset 函数将传入的 header 指针所指向的内存区域全部设置为 0。这个操作通常用于初始化内存，确保结构体中的所有字段都被正确初始化为 0。

    // random, 下面两行一起产生的，多线程，不安全，先srandom产生种子，在其他地方random（）调用会出问题
    srandom(time(NULL)); //提供一个随机种子，限定随机值的范围； 现在距离1970年的秒数，所以随机值很大！// srandom(100); //产生0-99的值！
    header->id = random();

    header->flags = htons(0x100);
    // header->questions = 1; //每次查一个DNS
    header->questions = htons(1); // 要转成网络字节序
}

// hostname:www.0voice.com 
// name：3www6voice3com0
int dns_create_question(struct dns_question *question, const char *hostname){
    if(question == NULL || hostname == NULL) return -1;
    memset(question, 0, sizeof(struct dns_question));

    question->name = (char*)malloc(strlen(hostname));
    if(question->name == NULL) return -2;

    question->length = strlen(hostname) + 2;
    question->qtype = htons(1); //变成A
	question->qclass = htons(1);

    // name转换,一段段截
    const char delim[2] = "."; //字符数组2个.
    char *qname = question->name;

    char *hostname_dup = strdup(hostname); //复制duplicate，自带malloc，返回一个动态的非常量！！
    char *token = strtok(hostname_dup,delim); //截取token, 按分隔符delim截
    while(token != NULL){
        // 第一组www
        size_t len = strlen(token);//取这段长度，size_t: 是一种用于表示内存大小的数据类型，
        *qname = len;
        qname++; //第一个放长度

        strncpy(qname, token, len + 1);//最后的\0结束符也copy
        qname += len;//指针移到结尾

        // 更新token，后面2组0voice com
        token = strtok(NULL, delim); //0voice.com第一次截要hostname_dup，第二次就接着截，所以线程不安全，因为保存了上一次截取的会错乱
    }

    free(hostname_dup);

}

//  协议包组织打包，已有header question合并到request
int dns_build_request(struct dns_header *header, struct dns_question *question, char *request, int rlen){
    if(header == NULL || question  == NULL || request == NULL) return -1;
    memset(request, 0, rlen);
    
    // header-->request
    memcpy(request, header, sizeof(struct dns_header)); //header复制到request
    int offset = sizeof(struct dns_header);
    
    // question-> request    
    memcpy(request + offset,question->name, question->length); // request +offset这里
    offset += question->length;
    
    memcpy(request+offset, &question->qtype, sizeof(question->qtype));
	offset += sizeof(question->qtype);

	memcpy(request+offset, &question->qclass, sizeof(question->qclass));
	offset += sizeof(question->qclass);

	return offset;

}

// 3 udp
int dns_client_commit(const char *domain){
    // AF_INET是一种常见的网络地址家族，它用于IPv4（Internet Protocol version 4）地址。在网络编程中，AF_INET常用于创建和处理IPv4套接字（socket）。它指示使用IPv4地址格式来进行通信。
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); //client发的forward fd

    if(sockfd < 0) return -1;

    // 结构体的定义这么多年基本这么写，比较固定
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DNS_SERVER_PORT);
	servaddr.sin_addr.s_addr = inet_addr(DNS_SERVER_IP);

    int ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	printf("connect : %d\n", ret);


    struct dns_header header = {0};
    dns_create_header(&header);

    struct dns_question question = {0};
    dns_create_question(&question, domain);

    char request[1024] = {0};
    int length = dns_build_request(&header, &question, request, 1024);

    // request
    int slen = sendto(sockfd, request, length, 0, (struct sockaddr*)&servaddr, sizeof(struct sockaddr));

    // recvfrom在这里阻塞，send和recieve在一起
    char response[1024] = {0};
    struct sockaddr_in addr;
    size_t addr_len = sizeof(struct sockaddr_in);

    int n = recvfrom(sockfd, response,sizeof(response), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
    printf("recvfrom: %d, %s\n", n , response);


    // for(int i = 0; i < n; i++) printf("%c", response[i]);
    // printf("\n");
    // for(int i = 0; i < n; i++) printf("%x", response[i]);


    return n;
}

int main(int argc, char* argv[]){
    if(argc <2) return -1;
    dns_client_commit(argv[1]);
}