/*
 * @Author: MDXZ
 * @Date: 2022-05-03 10:05:56
 * @LastEditTime : 2022-05-22 11:42:03
 * @LastEditors  : mdxz2048
 * @Description:
 * @FilePath     : /EasyWechat/src/socks_cs/server.c
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

#define BUFSIZE 1024 * 20

static int socket_server = -1;

typedef struct
{
    int socket_src;
    int socket_dst;

} SOCKS5_SERVER_Data_forward_t;

typedef struct
{
    int socket;
} SOCKS5_SERVER_PROCESS_CONNECT_t;

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
        debug_printf("create_socks5_server_socket() failed\n");
        goto error;
    }

    bzero(&servaddr, sizeof(servaddr));
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        debug_printf("socket bind failed...\n");
        goto error;
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        debug_printf("Listen failed...\n");
        goto error;
    }

    len = sizeof(cli);
    return sockfd;

error:
    close(sockfd);
    return -1;
}

void data_forward_loop(SOCKS5_SERVER_Data_forward_t *socka_to_sockb)
{
    char data_buf[BUFSIZE] = {0};
    u_int32_t recv_count = 0;
    bool isOK = true;
    int socket_src = socka_to_sockb->socket_src;
    int socket_dst = socka_to_sockb->socket_dst;
    debug_printf("src = %d dst = %d\n", socket_src, socket_dst);
    while (isOK)
    {

        bzero(data_buf, BUFSIZE);
        recv_count = recv(socket_src, data_buf, BUFSIZE, 0);
        debug_printf("recv: socket[%d] data length %d", socket_src, recv_count);
        if (recv_count > 0)
        {
            u_int32_t send_count = 0;
            send(socket_dst, data_buf, recv_count, 0);
            if (send_count < 0)
            {
                // close(socket_dst);
                // close(socket_src);
                // isOK = false;
            }
        }
        else
        {
            // close(socket_src);
            // close(socket_dst);
            // isOK = false;
        }
    }
    debug_printf("exit src:%d", socket_src);
    exit(0);
}

int server_process_data_forward(SOCKS5_CLIENT_INFO_t *sock5_client_info)
{
    pthread_t recv, send;
    void *thread_return = NULL;

    SOCKS5_SERVER_Data_forward_t client_to_dst;
    SOCKS5_SERVER_Data_forward_t dst_to_client;
    client_to_dst.socket_src = sock5_client_info->socket_client;
    client_to_dst.socket_dst = sock5_client_info->socket_dst;
    pthread_create(&recv, NULL, (void *)data_forward_loop, (void *)&client_to_dst);
    // pthread_detach(recv);
    dst_to_client.socket_src = sock5_client_info->socket_dst;
    dst_to_client.socket_dst = sock5_client_info->socket_client;
    pthread_create(&send, NULL, (void *)data_forward_loop, (void *)&dst_to_client);
    // pthread_detach(send);
    pthread_join(recv, &thread_return);
    pthread_join(send, &thread_return);
}

void server_process_connect_thread(void *sock)
{
    char read_buf[BUFSIZE] = {0};
    int read_count = 0;
    int ret = 0;
    int sock_client = (int)sock;

    uint16_t method_ok = -1;
    SOCKS5_CLIENT_INFO_t sock5_client_info = {0};

    // method and reply to client
    bzero(read_buf, sizeof(read_buf));
    read_count = recv(sock_client, read_buf, sizeof(read_buf), 0);
    if (read_count > 0)
    {
        debug_printf("sock_client:[%d] recv method:", sock_client);
        comm_print_hexdump(read_buf, read_count);
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
        debug_printf("sock_client:[%d] send method_reply:", sock_client);
        comm_print_hexdump(method_reply, method_reply_len);
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
        debug_printf("sock_client:[%d] recv request:", sock_client);
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
                // get bnd addr and port from socketdst
                struct sockaddr_in bnd_addr = {0};
                socklen_t bnd_addr_len = 0;
                if (getsockname(sock5_client_info.socket_dst, (struct sockaddr *)&bnd_addr, &bnd_addr_len) < 0)
                {
                    perror("getsockname");
                    goto err;
                }
                req_reply_info.atyp = SOCKS5_ATYP_IPv4;
                req_reply_info.bndAddr.addr_ipv4 = bnd_addr.sin_addr.s_addr;
                req_reply_info.bndPort = req->dst_port;

                char req_reply[256] = {0};
                u_int32_t req_reply_len = 0;
                if (socks5_server_package_request_reply(req_reply, &req_reply_len, &req_reply_info) < 0)
                {
                    debug_printf("socks5_server_package_request_reply() failed.");
                }
                else
                {
                    debug_printf("sock_client:[%d] send request_reply:", sock_client);
                    comm_print_hexdump(req_reply, req_reply_len);
                    if (send(sock5_client_info.socket_client, req_reply, req_reply_len, 0) < 0)
                    {
                        debug_printf("send req_reply error.");
                    }
                    else
                    {
                        debug_printf("sock_client:[%d] server_process_data_forward start...", sock_client);
                        // SOCKS5_SERVER_Data_forward_t client_to_dst;
                        // SOCKS5_SERVER_Data_forward_t dst_to_client;
                        // client_to_dst.socket_src = sock5_client_info.socket_client;
                        // client_to_dst.socket_dst = sock5_client_info.socket_dst;
                        // dst_to_client.socket_src = sock5_client_info.socket_dst;
                        // dst_to_client.socket_dst = sock5_client_info.socket_client;

                        // if (fork() == 0)
                        // {
                        //     data_forward_loop(&client_to_dst);
                        // }
                        // else
                        // {
                        //     data_forward_loop(&dst_to_client);
                        // }
                        server_process_data_forward(&sock5_client_info);
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
    SOCKS5_SERVER_PROCESS_CONNECT_t sock;
    sock.socket = sock_client;
    pthread_create(&id, NULL, (void *)server_process_connect_thread, (void *)sock_client);
    // pthread_detach(id);
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
    if (socket_server > 0)
    {
        debug_printf("create_socks5_server_socket success.PORT: %d", port);
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
                //?????????????????????????????????
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