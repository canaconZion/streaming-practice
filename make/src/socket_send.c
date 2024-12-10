#define _GNU_SOURCE
#define __FAVOR_BSD
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define SOURCE_ADDR "192.168.52.130"
#define SOURCE_PORT 54321
#define TARGET_ADDR "192.168.66.19"
#define TARGET_PORT 8554

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

void full_ip(struct iphdr *iph, struct sockaddr_in *dest, char *buffer, int data_len, int pro_type)
{
    iph->ihl = 5;                        // IP头长度
    iph->version = 4;                    // IPv4
    iph->tos = 0;                        // 服务类型
    iph->id = htonl(54321);              // 标识符
    iph->frag_off = 0;                   // 不分片
    iph->ttl = 64;                       // 生存时间
    iph->check = 0;                      // 校验和，先置为0
    iph->saddr = inet_addr(SOURCE_ADDR); // 源IP
    iph->daddr = dest->sin_addr.s_addr;  // 目标IP
    iph->check = checksum(buffer, sizeof(struct iphdr));
    if (pro_type == 1)
    {
        iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + data_len); // 总长度
        iph->protocol = IPPROTO_TCP;
    }
    else if (pro_type == 2)
    {
        iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len); // 总长度
        iph->protocol = IPPROTO_UDP;
    }
}

void full_tcp(struct iphdr *iph, struct tcphdr *tcph, struct sockaddr_in *dest, int data_len)
{
    tcph->source = htons(54321);
    tcph->dest = dest->sin_port;
    tcph->seq = htonl(123456);
    tcph->ack_seq = htonl(0);
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 1;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840);
    tcph->check = 0;
    tcph->urg_ptr = 0;

    struct pseudo_header
    {
        unsigned int src_addr;
        unsigned int dst_addr;
        unsigned char placeholder;
        unsigned char protocol;
        unsigned short tcp_length;
    };

    struct pseudo_header psh;
    psh.src_addr = iph->saddr;
    psh.dst_addr = iph->daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + data_len);

    int pseudo_packet_len = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + data_len;
    char *pseudo_packet = malloc(pseudo_packet_len);
    memcpy(pseudo_packet, &psh, sizeof(struct pseudo_header));
    memcpy(pseudo_packet + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + data_len);

    tcph->check = checksum(pseudo_packet, pseudo_packet_len);

    free(pseudo_packet);
}

void full_udp(struct udphdr *udph, struct sockaddr_in *dest, int data_len)
{
    udph->source = htons(54321);                         // 源端口
    udph->dest = dest->sin_port;                         // 目标端口
    udph->len = htons(sizeof(struct udphdr) + data_len); // UDP长度
    udph->check = 0;
}

int create_socket()
{
    int fd;
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd < 0)
    {
        perror("Socket creation failed");
    }
    int one = 1;
    /**
     * 防止系统修改ip头部
     */
    // if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
    // {
    //     perror("setsockopt");
    //     return -1;
    // }
    return fd;
}

int full_protocol(int sockfd, int pro_type)
{
    char buffer[4096];
    struct sockaddr_in dest;
    struct sockaddr_in dest2;

    struct iphdr *iph = (struct iphdr *)buffer;
    const char *custom_data = "Hello, this is custom TCP payload!";
    int data_len = strlen(custom_data);

    dest.sin_family = AF_INET;
    dest.sin_port = htons(TARGET_PORT);
    dest.sin_addr.s_addr = inet_addr(TARGET_ADDR);

    memset(buffer, 0, sizeof(buffer));
    full_ip(iph, &dest, buffer, data_len, pro_type);
    if (pro_type == 1)
    {
        // TCP protocol
        struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct iphdr));
        char *data = (char *)(buffer + sizeof(struct iphdr) + sizeof(struct tcphdr));
        memcpy(data, custom_data, data_len);
        full_tcp(iph, tcph, &dest, data_len);
        printf("TCP packet with payload will sent .\n");
    }
    else if (pro_type == 2)
    {
        // UDP protocol
        struct udphdr *udph = (struct udphdr *)(buffer + sizeof(struct iphdr));
        char *data = (char *)(buffer + sizeof(struct iphdr) + sizeof(struct udphdr));
        memcpy(data, custom_data, data_len);
        full_udp(udph, &dest, data_len);
        printf("UDP packet with payload will sent .\n");
    }
    if (sendto(sockfd, buffer, ntohs(iph->tot_len), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
    {
        perror("Send failed");
        close(sockfd);
        return -1;
    }
    printf("Packet with payload sent successfully.\n");

    dest2.sin_family = AF_INET;
    dest2.sin_port = htons(8554);
    dest2.sin_addr.s_addr = inet_addr("");
    full_ip(iph, &dest2, buffer, data_len, pro_type);
    struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct iphdr));
    full_tcp(iph, tcph, &dest2, data_len);

    if (sendto(sockfd, buffer, ntohs(iph->tot_len), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
    {
        perror("Send 2 failed");
        close(sockfd);
        return -1;
    }
    printf("Packet 2 with payload sent successfully.\n");
    return 0;
}

int main()
{
    int ret;
    int sockfd = create_socket();
    full_protocol(sockfd, 1);
    // full_protocol(sockfd, 2);
    char buffer[4096];
    // while (1)
    // {
    //     struct sockaddr_in addr;
    //     socklen_t addr_len = sizeof(addr);

    //     // 接收数据包
    //     ret = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
    //     if (ret < 0) {
    //         perror("Recvfrom failed");
    //         continue;
    //     }
    //     else
    //     {
    //         printf("recv %d msg\r\n", ret);
    //     }
    // }

    close(sockfd);
    return 0;
}
