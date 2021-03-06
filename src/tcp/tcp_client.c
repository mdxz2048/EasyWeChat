/*
 * @Author       : lv zhipeng
 * @Date         : 2022-05-18 14:28:38
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2022-05-20 09:57:11
 * @FilePath     : /EasyWeChat/src/tcp/tcp_client.c
 * @Description  :
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "tcp_client.h"

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(0);
}

int tcp_create_client_by_ipv4(uint32_t dst_addr, u_int16_t dst_port)
{
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    char *ip;
    struct in_addr in;
    in.s_addr = htonl(dst_addr);
    ip = inet_ntoa(in); //将网络字节序的32位地址转换为点分十进制字符串，通过返回值返回
    debug_printf("ip :%s", ip);
    // u_int16_t port = 1234;
    hostname = ip;
    portno = dst_port;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", hostname);
        return -1;
    }

    /* build the server's Internet address */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    // /* get message line from the user */
    // printf("Please enter msg: ");
    // bzero(buf, BUFSIZE);
    // fgets(buf, BUFSIZE, stdin);

    // /* send the message line to the server */
    // n = write(sockfd, buf, strlen(buf));
    // if (n < 0)
    //     error("ERROR writing to socket");

    // /* print the server's reply */
    // bzero(buf, BUFSIZE);
    // n = read(sockfd, buf, BUFSIZE);
    // if (n < 0)
    //     error("ERROR reading from socket");
    // printf("Echo from server: %s", buf);
    // close(sockfd);

    return sockfd;
}

int tcp_create_client_by_domain(socks5_dst_addr_domain_t *addr_domain, u_int16_t dst_port)
{
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    hostname = addr_domain->domain;
    portno = dst_port;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        debug_printf("connect() failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int tcp_create_client(SOCKS5_ATYP_e address_type, ADDR_u *dst_addr, u_int16_t dst_port)
{
    int sock = -1;
    switch (address_type)
    {
    case SOCKS5_ATYP_IPv4:
        sock = tcp_create_client_by_ipv4(dst_addr->addr_ipv4, dst_port);
        break;
    case SOCKS5_ATYP_DOMAIN:
        sock = tcp_create_client_by_domain(&(dst_addr->addr_domain), dst_port);
        break;
    case SOCKS5_ATYP_IPv6:
        break;
    }
    return sock;
}
