/*
 * @Author: MDXZ
 * @Date: 2022-05-03 10:05:56
 * @LastEditTime: 2022-05-03 15:21:02
 * @LastEditors: MDXZ
 * @Description:
 * @FilePath: /EasyWechat/src/socks_cs/server.c
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int create_socks5_server_socket(u_int16_t port)
{
    int parentfd;                  /* parent socket */
    int childfd;                   /* child socket */
    int portno;                    /* port to listen on */
    int clientlen;                 /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp;         /* client host info */
    char buf[BUFSIZE];             /* message buffer */
    char *hostaddrp;               /* dotted decimal host addr string */
    int optval;                    /* flag value for setsockopt */
    int n;                         /* message byte size */

    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0)
        perror("ERROR opening socket");

    bzero((char *)&serveraddr, sizeof(serveraddr));
    /* this is an Internet address */
    serveraddr.sin_family = AF_INET;
    /* let the system figure out our IP address */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* this is the port we will listen on */
    serveraddr.sin_port = htons((unsigned short)portno);
    if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
        error("ERROR on listen");
#if 1
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                          sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
        error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
        error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n",
           hostp->h_name, hostaddrp);
#endif
    return parentfd;
}

void server_process_connect_thread(void *sock)
{
    char read_buf[1024 * 10] = {0};
    int read_count = 0;
    int ret = 0;
    int socket_dst = 0;
    int sock_client = *(int *)sock;
    // method
    bzero(read_buf, sizeof(read_buf));
    read_count = recv(sock_client, read_buf, sizeof(read_buf), 0);
    if (read_count)
    {
        socks5_srv_method_reply_send(sock_client, (const SOCKS5_METHOD_REQ_t *)read_buf);
    }
    // build
    bzero(read_buf, sizeof(read_buf));
    read_count = recv(sock_client, read_buf, sizeof(read_buf), 0);
    if (read_count)
    {
        socket_dst = socks5_srv_build_request_process(sock_client, (const socks5_build_req_t *)read_buf);
    }

    // proxy
    if (socket_dst > 0)
    {
        char dst_buf[1024 * 10] = {0};

        if (fork() == 0) // clild
        {
            char client_buf[1024 * 10] = {0};
            char client_buf_encode[1024 * 10] = {0};
            int recv_cnt = 0;
            int ret = -1;
            while (1)
            {
                recv_cnt = recv(socket_dst, client_buf, ARRAY_SIZE(client_buf), 0);
                ret = send(sock_client, client_buf, recv_cnt, 0);
                if (ret < 0)
                    perror("send err");
            }
        }
        else
        {
            char dst_buf[1024 * 10] = {0};
            int recv_cnt = 0;
            int ret = -1;
            while (1)
            {
                recv_cnt = recv(sock_client, dst_buf, ARRAY_SIZE(dst_buf), 0);
                ret = send(socket_dst, dst_buf, recv_cnt, 0);
                if (ret < 0)
                    perror("send err");
            }
        }
    }
}

int server_process_connect(int sock_client)
{
    pthread_t id;
    pthread_create(&id, NULL, (void *)server_process_connect_thread, (void *)&sock_client);
}

void start_server(u_int16_t port, SOCKS5_AUTH_e method, u_int8_t username_len, u_int8_t *username, u_int8_t password_len, u_int8_t *password)
{
    struct sockaddr_in clientaddr;
    int clientaddr_len = sizeof(clientaddr);
    int socket_server = -1;
    int socket_client = -1;
    socket_server = create_socks5_server_socket(port);
    if (socket_server > 0)
    {
        char buf[1024 * 10] = {0};
        while (1)
        {
            socket_client = accept(socket_server, (struct sockaddr *)&clientaddr, &clientaddr_len);
            if (socket_client > 0)
                server_process_connect(socket_client);
        }
    }
    else
        perror("create_socks5_server_socket error");
}