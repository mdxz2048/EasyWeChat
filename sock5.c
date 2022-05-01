#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include "comm.h"
#include "sock5.h"




int sock5_method_request_send(const SOCKET to_server, const SOCKS5_AUTH_e method)
{
    SOCKS5_METHOD_REQ_t req = 0;
    req.version = SOCKS5_VERSION;
    req.methodNum = 1;
    req.methods[0] = method;

    if(-1 == send(to_server, &req, sizeof(SOCKS5_METHOD_REQ_t), NULL))
    {
        return -1;
    }

    return 0;
}

int socks5_method_check(uint8_t method)
{
    if()
}
int sock5_method_reply_send(const SOCKET to_client, SOCKS5_METHOD_REQ_t recv_req)
{
    SOCKS5_METHOD_REPLY_t reply = 0;

    if (recv_req.version != SOCKS5_VERSION)
    {
        perror("version is %d\n", version);
        return EINVAL;
    }
    if (recv_req.methodNum <= 0)
    {
        perror("methodNum is %d\n", version);
        return EINVAL;
    }

    reply.method = SOCKS5_AUTH_PASSWORD;

    return req;
}
