/*
 * @Author       : mdxz2048
 * @Date         : 2022-05-17 19:09:03
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2022-05-19 16:28:43
 * @FilePath     : /EasyWeChat/src/sock/sock5.c
 * @Description  :
 *
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include "common.h"
#include "sock5.h"
#include <unistd.h>

/*
	+----+----------+----------+
	|VER | NMETHODS | METHODS  |
	+----+----------+----------+
	| 1  |    1     | 1 to 255 |
	+----+----------+----------+
*/

int socks5_client_package_version_method(char *data, u_int8_t *data_len, const u_int8_t nmethods_num, const SOCKS5_METHOD_e methods)
{
	u_int32_t len = 0;

	*(data + len) = 0x05;
	data_len++;
	*(data + len) = nmethods_num;
	data_len++;
	*(data + len) = methods;

	*data_len = len;

	return 0;
}

int SOCKS5_METHOD_Valid(u_int32_t method)
{
	int valid = -1;

	switch (method)
	{
	case SOCKS5_AUTH_NONE:
	case SOCKS5_AUTH_GSSAPI:
	case SOCKS5_AUTH_PASSWORD:
	case ISOCKS5_AUTH_ANA_ASSIGNED:
	case SOCKS5_AUTH_INVALID:
		valid = 1;
	};

	return valid;
}

int socks5_server_parse_version_method(SOCKS5_METHOD_e *method, const char *data, const u_int32_t data_len)
{
	char version_method[128] = {0};
	int ret = -1;

	memcpy(version_method, data, data_len);
	// check sock version
	if (version_method[0] == 0x05)
	{
		// find valid method,and return method.
		for (int methods_cnt = 0; methods_cnt < version_method[1]; methods_cnt++)
		{
			if (ret = SOCKS5_METHOD_Valid(version_method[2 + methods_cnt]) > -1)
			{
				*method = ret;
			}
		}
		// not found,return -1
		if (ret < 0)
		{
			return -1;
		}
	}

	return 0;
}

/**
	+----+--------+
	|VER | METHOD |
	+----+--------+
	| 1  |   1    |
	+----+--------+
 */

/**
 * @description:
 * @param {char} *data
 * @param {u_int8_t} *data_len
 * @param {SOCKS5_METHOD_e} methods
 * @return {*}
 */
int socks5_server_package_method_reply(char *data, u_int8_t *data_len, const SOCKS5_METHOD_e methods)
{
	*data = 0x05;
	*(data + 1) = methods;

	*data_len = 2;

	return 0;
}

/*
	+----+-----+-------+------+----------+----------+
	|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
	+----+-----+-------+------+----------+----------+
	| 1  |  1  | X'00' |  1   | Variable |    2     |
	+----+-----+-------+------+----------+----------+
*/
int socks5_client_package_request(char *data, u_int32_t *data_len, const SOCKS5_REQUEST_t *req)
{
	char request[256] = {0};
	u_int32_t len = 0;
	// VER
	request[len] = 0x05;
	len++;
	// CMD
	request[len] = req->cmd;
	len++;
	// RSV
	request[len] = 0x0;
	len++;
	// ATYP
	request[len] = req->atyp;
	len++;
	// DST.ADDR
	switch (req->atyp)
	{
	case SOCKS5_ATYP_IPv4:
		memcpy(&request[len], &(req->dst_addr.addr_ipv4), SOCKS5_ADDR_IPV4_LENGTH);
		len += SOCKS5_ADDR_IPV4_LENGTH;
		break;
	case SOCKS5_ATYP_IPv6:
		memcpy(&request[len], req->dst_addr.addr_ipv6, SOCKS5_ADDR_IPV6_LENGTH);
		len += SOCKS5_ADDR_IPV6_LENGTH;
		break;
	case SOCKS5_ATYP_DOMAIN:
		memcpy(&request[len], &(req->dst_addr.addr_domain.domain_len), 1);
		len += 1;
		memcpy(&request[len], req->dst_addr.addr_domain.domain, req->dst_addr.addr_domain.domain_len);
		len += req->dst_addr.addr_domain.domain_len;
		break;
	}
	// DST.PORT 2 Byte
	memcpy(&request[len], &(req->dst_port), 2);
	len += 2;

	memcpy(data, request, len);
	*data_len = len;

	return 0;
}

int socks5_server_parse_request(SOCKS5_REQUEST_t *req, const char *data, const u_int32_t data_len)
{
	u_int32_t offset = 0;
	// VER
	req->version = *data;
	offset++;
	// CMD
	req->cmd = *(data + offset);
	offset++;
	// RSV
	offset++;
	// ATYP
	req->atyp = *(data + offset);
	offset++;
	// DST.ADDR
	switch (req->atyp)
	{
	case SOCKS5_ATYP_IPv4:
	{
		uint32_t network_addr = 0;
		memcpy(&network_addr, data + offset, SOCKS5_ADDR_IPV4_LENGTH);
		req->dst_addr.addr_ipv4 = ntohl(network_addr);
		offset += SOCKS5_ADDR_IPV4_LENGTH;
		break;
	}
	case SOCKS5_ATYP_IPv6:
		memcpy(req->dst_addr.addr_ipv6, data + offset, SOCKS5_ADDR_IPV6_LENGTH);
		offset += SOCKS5_ADDR_IPV6_LENGTH;
		break;
	case SOCKS5_ATYP_DOMAIN:
		memcpy(&(req->dst_addr.addr_domain.domain_len), data + offset, 1);
		offset += 1;
		memcpy(req->dst_addr.addr_domain.domain, data + offset, req->dst_addr.addr_domain.domain_len);
		offset += req->dst_addr.addr_domain.domain_len;
		break;
	}
	// DST.PORT
	uint16_t network_short = 0;
	memcpy(&network_short, data + offset, 2);
	req->dst_port = ntohs(network_short); // network to host short
	offset += 2;
	if (req->atyp == SOCKS5_ATYP_DOMAIN)
		debug_printf("req:\nversion:0x%x\ncmd:0x%x\naddress_type:0x%x\naddr:%s\nport:%d\n", req->version, req->cmd, req->atyp, req->dst_addr.addr_domain.domain, req->dst_port);
	else
		debug_printf("req:\nversion:0x%x\ncmd:0x%x\naddress_type:0x%d\naddr:%ld\nport:%d\n", req->version, req->cmd, req->atyp, req->dst_addr.addr_ipv4, req->dst_port);

	return 0;
}
/*
	+----+-----+-------+------+----------+----------+
	|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
	+----+-----+-------+------+----------+----------+
	| 1  |  1  | X'00' |  1   | Variable |    2     |
	+----+-----+-------+------+----------+----------+
*/
int socks5_server_package_request_reply(char *data, u_int32_t *data_len, const SOCKS5_REQUEST_REPLY_t *reply)
{
	char replies[256] = {0};
	u_int32_t offset = 0;
	// VER
	replies[offset] = 0x05;
	offset++;
	// REP
	replies[offset] = reply->rep;
	offset++;
	// RSV
	replies[offset] = 0x0;
	offset++;
	// ATYP
	replies[offset] = reply->atyp;
	offset++;
	// DST.ADDR
	switch (reply->atyp)
	{
	case SOCKS5_ATYP_IPv4:
	{
		uint32_t network_addr = 0;
		network_addr = htonl(reply->bndAddr.addr_ipv4);
		memcpy(&replies[offset], &network_addr, SOCKS5_ADDR_IPV4_LENGTH);
		offset += SOCKS5_ADDR_IPV4_LENGTH;
		break;
	}
	}
	// DST.PORT
	u_int16_t network_port = htons(reply->bndPort);
	memcpy(&replies[offset], &network_port, sizeof(network_port));
	offset += sizeof(network_port);

	memcpy(data, replies, offset);
	*data_len = offset;
	return 0;
}
