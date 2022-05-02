/*
 * @Author: MDXZ
 * @Date: 2022-05-01 17:06:40
 * @LastEditTime: 2022-05-02 08:44:54
 * @LastEditors: MDXZ
 * @Description: 
 * @FilePath: /EasyWechat/sock/sock5.c
 * 
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include "comm.h"
#include "sock5.h"

/**
 * @description: 
 * @param {int} to_server
 * @param {SOCKS5_AUTH_e} method
 * @return {*}
 */
int sock5_method_request_send(const int to_server, const SOCKS5_AUTH_e method)
{
    SOCKS5_METHOD_REQ_t req = {0};
    req.version = SOCKS5_VERSION;
    req.methodNum = 1;
    req.methods[0] = method;

    if (-1 == send(to_server, &req, sizeof(SOCKS5_METHOD_REQ_t), 0))
    {
        return -1;
    }

    return 0;
}

SOCKS5_AUTH_e socks5_method_check(u_int8_t method)
{
    int ret = -1;
    if ((method == SOCKS5_AUTH_NONE) ||
        (method == SOCKS5_AUTH_GSSAPI) ||
        (method == SOCKS5_AUTH_PASSWORD) ||
        (method == ISOCKS5_AUTH_ANA_ASSIGNED))
    {
        return method;
    }else
    return SOCKS5_AUTH_INVALID;
}

int sock5_method_reply_send(const int to_client, SOCKS5_METHOD_REQ_t *recv_req)
{
    SOCKS5_METHOD_REPLY_t reply = {0};

    if (recv_req->version != SOCKS5_VERSION)
    {
        perror("version err.");
        return -1;
    }
    if (recv_req->methodNum <= 0)
    {
        perror("methodNum err.");
        return -1;
    }

    reply.method = socks5_method_check(recv_req->methods[0]); /*only supports one method.SOCKS5_AUTH_PASSWORD*/
    if (-1 == send(to_client, &reply, sizeof(SOCKS5_METHOD_REPLY_t), 0))
    {
        return -1;
    }

    return 0;
}