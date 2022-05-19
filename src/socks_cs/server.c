/*
 * @Author: MDXZ
 * @Date: 2022-05-03 10:05:56
 * @LastEditTime : 2022-05-19 16:50:36
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
#include <signal.h>
#include "tcp_client.h"

#define BUFSIZE 1024

static int socket_server = -1;

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

void server_process_connect_thread(void *sock)
{
    char read_buf[1024 * 10] = {0};
    int read_count = 0;
    int ret = 0;
    int socket_dst = 0;
    int sock_client = (int)sock;
    uint16_t method_ok = -1;
    SOCKS5_CLIENT_INFO_t sock5_client_info = {0};

    // int flags = fcntl(sock_client, F_GETFL, 0);
    // fcntl(sock_client, F_SETFL, flags | O_NONBLOCK);

    // method and reply to client
    bzero(read_buf, sizeof(read_buf));
    read_count = recv(sock_client, read_buf, sizeof(read_buf), 0);
    if (read_count > 0)
    {
        /*check the sock version and method */
        SOCKS5_METHOD_e method = 0;
        if (socks5_server_parse_version_method(&method, read_buf, read_count) < 0)
        {
            debug_printf("socks5_server_parse_version_method() failed...\n");
            goto err;
        }
        sock5_client_info.isvalid = true;
        sock5_client_info.socket_client = sock_client;
        sock5_client_info.method = method;
        /*package the method and send to client*/
        char method_reply[8] = {0};
        u_int8_t method_reply_len = 0;
        if (socks5_server_package_method_reply(method_reply, &method_reply_len, method < 0))
        {
            debug_printf("socks5_server_package_method_reply() failed");
            goto err;
        }
        debug_printf("send method_len = %d\n", method_reply_len);
        if (send(sock_client, method_reply, method_reply_len, 0) < 0)
        {
            debug_printf("socks5_server_package_method_reply() failed");
            goto err;
        }
    }
    else
    {
        debug_printf("recv_count < 0\n");
        goto err;
    }
    // recv request form client
    bzero(read_buf, sizeof(read_buf));
    read_count = recv(sock_client, read_buf, sizeof(read_buf), 0);
    if (read_count > 0)
    {
        comm_print_hexdump(read_buf, read_count);
        SOCKS5_REQUEST_t *req = (SOCKS5_REQUEST_t *)calloc(1, sizeof(SOCKS5_REQUEST_t));
        socks5_server_parse_request(req, read_buf, read_count);
        if (req->cmd == SOCKS5_CMD_CONNECT)
        {
            sock5_client_info.socket_dst = tcp_create_client(req->atyp, &(req->dst_addr), req->dst_port);
            if (sock5_client_info.socket_dst > 0)
            {
                SOCKS5_REQUEST_REPLY_t req_reply_info = {0};
                req_reply_info.version = 0x05;
                req_reply_info.rep = SOCKS5_REP_SUCCEEDED;
                req_reply_info.rsv = 0;
                req_reply_info.atyp = req->atyp;
                req_reply_info.bndAddr = req->dst_addr;
                req_reply_info.bndPort = req->dst_port;

                char req_reply[256] = {0};
                u_int32_t req_reply_len = 0;
                debug_printf("tcp_create_client success");
                if (socks5_server_package_request_reply(req_reply, &req_reply_len, &req_reply_info) < 0)
                {
                    debug_printf("socks5_server_package_request_reply() failed.");
                }
                else
                {
                    debug_printf("send request reply:");
                    comm_print_hexdump(req_reply, req_reply_len);
                    if (send(sock5_client_info.socket_client, req_reply, req_reply_len, 0) < 0)
                    {
                        debug_printf("send req_reply error.");
                    }
                    else
                    {
                        if (fork() == 0)
                        {
                            char client_buf[1024 * 10] = {0};
                            u_int32_t client_len = 0;
                            while (1)
                            {

                                bzero(client_buf, sizeof(client_buf));
                                client_len = recv(sock5_client_info.socket_client, client_buf, sizeof(client_buf), 0);
                                if (client_len > 0)
                                {
                                    send(sock5_client_info.socket_dst, client_buf, client_len, 0);
                                    debug_printf("recv form socket_dst:%s", client_buf);
                                }
                            }
                        }
                        else
                        {
                            char dst_buf[1024 * 10] = {0};
                            u_int32_t dst_len = 0;

                            while (1)
                            {
                                bzero(dst_buf, sizeof(dst_buf));
                                dst_len = recv(sock5_client_info.socket_dst, dst_buf, sizeof(dst_buf), 0);
                                if (dst_len > 0)
                                {
                                    debug_printf("recv form socket_dst:%s", dst_buf);
                                    send(sock5_client_info.socket_client, dst_buf, dst_len, 0);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                debug_printf("tcp_create_client faild.");
            }
        }
    }
err:
    close(sock_client);
}

int server_process_connect(int sock_client)
{
    pthread_t id;
    // server_process_connect_thread(sock_client);
    pthread_create(&id, NULL, (void *)server_process_connect_thread, (void *)sock_client);
    pthread_detach(id);
}
void server_recv_signal_from_system(int signal_num)
{
    close(socket_server);
    printf("recv signal %d\n", signal_num);
    exit(1);
}

void start_server(u_int32_t port, SOCKS5_METHOD_e method, u_int8_t username_len, u_int8_t *username, u_int8_t password_len, u_int8_t *password)
{
    struct hostent *hostp; /* client host info */
    char *hostaddrp;       /* dotted decimal host addr string */
    struct sockaddr clientaddr;
    int clientaddr_len = sizeof(clientaddr);
    int socket_client = -1;

    signal(SIGINT, server_recv_signal_from_system);
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