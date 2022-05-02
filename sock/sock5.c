/*
 * @Author: MDXZ
 * @Date: 2022-05-01 17:06:40
 * @LastEditTime: 2022-05-02 10:40:23
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
int socks5_client_method_request_send(const int to_server, const SOCKS5_AUTH_e method)
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

SOCKS5_AUTH_e socks5_srv_method_check(const u_int8_t method)
{
	int ret = -1;
	if ((method == SOCKS5_AUTH_NONE) ||
		(method == SOCKS5_AUTH_GSSAPI) ||
		(method == SOCKS5_AUTH_PASSWORD) ||
		(method == ISOCKS5_AUTH_ANA_ASSIGNED))
	{
		return method;
	}
	else
		return SOCKS5_AUTH_INVALID;
}

int socks5_srv_method_reply_send(const int to_client, const SOCKS5_METHOD_REQ_t *recv_req)
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

	reply.method = socks5_srv_method_check(recv_req->methods[0]); /*only supports one method.SOCKS5_AUTH_PASSWORD*/
	if (-1 == send(to_client, &reply, sizeof(SOCKS5_METHOD_REPLY_t), 0))
	{
		return -1;
	}

	return 0;
}

/**
 * @description:
 * @param {SOCKS5_METHOD_REPLY_t} *reply
 * @return method or -1(error).
 */
int socks5_client_method_result_parse(const SOCKS5_METHOD_REPLY_t *reply)
{
	if (reply->version != SOCKS5_VERSION)
	{
		perror("version err.");
		return -1;
	}
	else if (reply->method == SOCKS5_AUTH_INVALID)
	{
		perror("SOCKS5_AUTH_INVALID");
		return -1;
	}
	return reply->method;
}

int socks5_client_build_request_send(const int to_server, const SOCKS5_CMD_e cmd, const SOCKS5_ATYP_e type, char *dst_addr, u_int16_t domain_len, u_int16_t dstPort)
{
	socks5_build_req_t build_req = {0};
	build_req.version = SOCKS5_VERSION;
	build_req.cmd = cmd;
	build_req.atyp = type;
	switch (type)
	{
	case SOCKS5_ATYP_IPv4:
		inet_aton(dst_addr, &build_req.dst_addr.addr_ipv4);
		break;
	case SOCKS5_ATYP_DOMAIN:
		build_req.dst_addr.addr_domain.domain_len = domain_len;
		memcpy(build_req.dst_addr.addr_domain.domain, dst_addr, domain_len);
		break;
	default:
		return -1;
	}
	build_req.version = SOCKS5_VERSION;

	return 0;
}
