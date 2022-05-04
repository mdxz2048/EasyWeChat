/*
 * @Author: MDXZ
 * @Date: 2022-05-03 15:49:32
 * @LastEditTime: 2022-05-04 13:18:19
 * @LastEditors: MDXZ
 * @Description:
 * @FilePath: /EasyWechat/src/socks_cs/client.c
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
static void error(char *msg)
{
    perror(msg);
    exit(1);
}

int create_local_listen(u_int16_t listen_port)
{
    int socket_local_server;       /* parent socket */
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

    /*
     * socket: create the parent socket
     */
    socket_local_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_local_server < 0)
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    // optval = 1;
    // setsockopt(socket_local_server, SOL_SOCKET, SO_REUSEADDR,
    //            (const void *)&optval, sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *)&serveraddr, sizeof(serveraddr));

    /* this is an Internet address */
    serveraddr.sin_family = AF_INET;

    /* let the system figure out our IP address */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* this is the port we will listen on */
    serveraddr.sin_port = listen_port;

    /*
     * bind: associate the parent socket with a port
     */
    if (bind(socket_local_server, (struct sockaddr *)&serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /*
     * listen: make this socket ready to accept connection requests
     */
    if (listen(socket_local_server, 5) < 0) /* allow 5 requests to queue up */
        error("ERROR on listen");
#if 0
    /*
     * main loop: wait for a connection request, echo input line,
     * then close connection.
     */
    clientlen = sizeof(clientaddr);
    while (1)
    {

        /*
         * accept: wait for a connection request
         */
        childfd = accept(socket_local_server, (struct sockaddr *)&clientaddr, &clientlen);
        if (childfd < 0)
            error("ERROR on accept");

        /*
         * gethostbyaddr: determine who sent the message
         */
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                              sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL)
            error("ERROR on gethostbyaddr");
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            error("ERROR on inet_ntoa\n");
        printf("server established connection with %s (%s)\n",
               hostp->h_name, hostaddrp);

        /*
         * read: read input string from the client
         */
        bzero(buf, BUFSIZE);
        n = read(childfd, buf, BUFSIZE);
        if (n < 0)
            error("ERROR reading from socket");
        printf("server received %d bytes: %s", n, buf);

        /*
         * write: echo the input string back to the client
         */
        n = write(childfd, buf, strlen(buf));
        if (n < 0)
            error("ERROR writing to socket");

        close(childfd);
    }
#endif
    return socket_local_server;
}

int create_local_client(char *server_ip, u_int16_t server_port, uint32_t username_len, char *username, uint32_t password_len, char *password)
{
    int socket_to_server, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    int ret = 0;

    socket_to_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_to_server < 0)
        error("ERROR opening socket");

    server = gethostbyname(server_ip);
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
    if (connect(socket_to_server, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        error("ERROR connecting");
        close(socket_to_server);
    }
    /*send method request to server */
    socks5_client_method_request_send(socket_to_server, SOCKS5_AUTH_PASSWORD);
    /*parse the server response for method*/
    bzero(buf, sizeof(buf));
    ret = recv(socket_to_server, buf, BUFSIZE, 0);
    if (ret)
        socks5_client_method_result_parse((const SOCKS5_METHOD_REPLY_t *)buf);
    /*send dst address and port*/
    char test_url[] = "www.google.com";
    u_int16_t test_port = 80;
    socks5_client_build_request_send(socket_to_server, SOCKS5_CMD_CONNECT, SOCKS5_ATYP_DOMAIN, test_url, ARRAY_SIZE(test_url), test_port);
    /*check build reply from server */
    bzero(buf, sizeof(buf));
    ret = recv(socket_to_server, buf, BUFSIZE, 0);
    if (ret)
    {
        if (socks5_client_build_reply_process((const socks5_reply_t *)buf) < 0) // server rep is error.
        {
            close(socket_to_server);
            return -1;
        }
    }
    return socket_to_server;
}