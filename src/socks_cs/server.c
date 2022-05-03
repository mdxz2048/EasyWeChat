/*
 * @Author: MDXZ
 * @Date: 2022-05-03 10:05:56
 * @LastEditTime: 2022-05-03 10:18:39
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
#include "sock5.h"

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int create_socks5_server(u_int16_t port, SOCKS5_AUTH_e method)
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
        error("ERROR opening socket");
    bzero((char *)&serveraddr, sizeof(serveraddr));

    /* this is an Internet address */
    serveraddr.sin_family = AF_INET;

    /* let the system figure out our IP address */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* this is the port we will listen on */
    serveraddr.sin_port = htons((unsigned short)portno);
    if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
        error("ERROR on listen");
    childfd = accept(parentfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (childfd < 0)
        error("ERROR on accept");
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
    while (true)
    {
        int recv_cnt = -1;
        recv_cnt = recv(childfd, buf, BUFSIZE, 0);
        socks5_srv_method_reply_send(childfd, (const SOCKS5_METHOD_REQ_t *)buf);
    }

    return 0;
}
