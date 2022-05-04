/*
 * @Author: MDXZ
 * @Date: 2022-05-01 17:06:40
 * @LastEditTime: 2022-05-04 13:13:27
 * @LastEditors: MDXZ
 * @Description:
 * @FilePath: /EasyWechat/src/sock/sock5.c
 *
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include "common.h"
#include "sock5.h"
#include <unistd.h>

/**
 * @description:
 * @param {int} to_server
 * @param {SOCKS5_AUTH_e} method
 * @return {*}
 */
int socks5_client_method_request_send(const int socket_server, const SOCKS5_AUTH_e method)
{
	SOCKS5_METHOD_REQ_t req = {0};
	req.version = SOCKS5_VERSION;
	req.methodNum = 1;
	req.methods[0] = method;

	if (-1 == send(socket_server, &req, sizeof(SOCKS5_METHOD_REQ_t), 0))
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

int socks5_srv_method_reply_send(const int socket_to_client, const SOCKS5_METHOD_REQ_t *recv_req)
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
	if (-1 == send(socket_to_client, &reply, sizeof(SOCKS5_METHOD_REPLY_t), 0))
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

int socks5_client_build_request_send(const int sockfd_srv, const SOCKS5_CMD_e cmd, const SOCKS5_ATYP_e type, char *dst_addr, u_int16_t domain_len, u_int16_t dstPort)
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

	if (-1 == send(sockfd_srv, &build_req, sizeof(socks5_build_req_t), 0))
	{
		perror("send socks5_build_req_t.");
		return -1;
	}

	return 0;
}

SOCKS5_AUTH_e socks5_srv_cmd_check(const SOCKS5_CMD_e cmd)
{
	int ret = -1;
	if ((cmd == SOCKS5_CMD_CONNECT) ||
		(cmd == SOCKS5_CMD_BIND) ||
		(cmd == SOCKS5_CMD_UDP))
	{
		return cmd;
	}
	else
		return -1;
}

int socks5_srv_build_request_process_connect(const socks5_build_req_t *recv_build_req)
{
	int sock_to_connect, portno, n;
	struct sockaddr_in serveraddr;
	struct hostent *server;

	/* socket: create the socket */
	sock_to_connect = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_to_connect < 0)
	{
		perror("ERROR opening sock_to_connect");
		return -1;
	}

	/* build the server's Internet address */
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr = recv_build_req->dst_addr.addr_ipv4;
	serveraddr.sin_port = htons(portno);

	/* connect: create a connection with the server */
	if (connect(sock_to_connect, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		perror("ERROR connecting");
		return -1;
	}
	else
		return sock_to_connect;
}

int socks5_srv_build_request_process(int socket_client, const socks5_build_req_t *recv_build_req)
{
	socks5_reply_t reply = {0};
	int sock_dst = 0; // connect to dst_addr

	// check
	if (SOCKS5_VERSION != recv_build_req->version)
	{
		perror("SOCKS5_VERSION err.");
		return -1;
	}

	if (recv_build_req->cmd == SOCKS5_CMD_CONNECT)
	{
		sock_dst = socks5_srv_build_request_process_connect(recv_build_req);
		if (sock_dst < 0)
		{
			perror("connect err.");
		}
	}
	// reply
	reply.version = SOCKS5_VERSION;
	if (sock_dst < 0)
		reply.rep = SOCKS5_REP_CONNECTION_REFUSED;
	else
		reply.rep = SOCKS5_REP_SUCCEEDED;
	reply.atyp = recv_build_req->atyp;
	reply.bndAddr = recv_build_req->dst_addr;
	reply.bndPort = recv_build_req->dstPort;

	if (-1 == send(socket_client, &reply, sizeof(socks5_reply_t), 0))
	{
		perror("send socks5_reply_t.");
		return -1;
	}

	return (sock_dst > 0) ? sock_dst : -1;
}

int socks5_client_build_reply_process(const socks5_reply_t *recv_reply)
{
	if (recv_reply->rep != SOCKS5_REP_SUCCEEDED)
	{
		return -1;
	}
	return 0;
}
