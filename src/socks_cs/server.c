/*
 * @Author: MDXZ
 * @Date: 2022-05-03 10:05:56
 * @LastEditTime : 2022-05-17 16:56:32
 * @LastEditors  : lv zhipeng
 * @Description:
 * @FilePath     : /EasyWeChat/src/socks_cs/server.c
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
#include "common.h"
#include <fcntl.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
static void error(char *msg)
{
    perror(msg);
    exit(1);
}

int create_socks5_server_socket(u_int32_t port)
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    return sockfd;
}

void server_process_connect_thread(u_int32_t sock)
{
    char read_buf[1024 * 10] = {0};
    int read_count = 0;
    int ret = 0;
    int socket_dst = 0;
    int sock_client = sock;
    uint16_t method_ok = -1;

    // int flags = fcntl(sock_client, F_GETFL, 0);
    // fcntl(sock_client, F_SETFL, flags | O_NONBLOCK);
    while (1)
    {
        // method
        bzero(read_buf, sizeof(read_buf));
        read_count = recv(sock_client, read_buf, sizeof(read_buf), 0);
        if (read_count > 0)
        {
            debug_printf("recvlen=%d\nrecv:  VER=%d\n", read_count, read_buf[0]);
            debug_printf("recv:  NMETHODS=%d\n", read_buf[1]);
            for (int i = 0; i < read_buf[1]; i++)
            {
                printf("METHODS[%d] = 0X%x\n", i, read_buf[2 + i]);
            }

            // socks5_srv_method_reply_send(sock_client, (const SOCKS5_METHOD_REQ_t *)read_buf);
        }
        else
        {
            debug_printf("recv_count < 0\n");
        }
    }
}

int server_process_connect(int sock_client)
{
    pthread_t id;
    server_process_connect_thread(sock_client);
    // pthread_create(&id, NULL, (void *)func, sock_client);
}

void start_server(u_int32_t port, SOCKS5_METHOD_e method, u_int8_t username_len, u_int8_t *username, u_int8_t password_len, u_int8_t *password)
{
    struct hostent *hostp; /* client host info */
    char *hostaddrp;       /* dotted decimal host addr string */
    struct sockaddr clientaddr;
    int clientaddr_len = sizeof(clientaddr);
    int socket_server = -1;
    int socket_client = -1;
    socket_server = create_socks5_server_socket(port);
    debug_printf("create_socks5_server_socket success.PORT: %d", port);
    if (socket_server > 0)
    {
        char buf[1024 * 10] = {0};
        while (1)
        {
            socket_client = accept(socket_server, (struct sockaddr *)&clientaddr, &clientaddr_len);
            // Accept the data packet from client and verification
            if (socket_client > 0)
            {
                // hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                //                       sizeof(clientaddr.sin_addr.s_addr), AF_INET);
                // if (hostp == NULL)
                //     error("ERROR on gethostbyaddr");
                // hostaddrp = inet_ntoa(clientaddr.sin_addr);
                // if (hostaddrp == NULL)
                //     error("ERROR on inet_ntoa\n");
                // printf("server established connection with %s (%s)\n",
                //        hostp->h_name, hostaddrp);
                debug_printf("server accept the client...\n");
                server_process_connect(socket_client);
            }
            else
            {
                debug_printf("accept failed.");
            }
        }
    }
    else
        perror("create_socks5_server_socket error");
}