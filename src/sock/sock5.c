/*
 * @Author       : mdxz2048
 * @Date         : 2022-05-17 19:09:03
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2022-05-18 09:22:11
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
int socks5_client_package_request(char *data, u_int32_t *data_len, const SOCKS5_CMD_e cmd, const SOCKS5_ATYP_e address_type, const char *dst_addr, const u_int32_t dst_addr_len, const u_int16_t dst_port)
{
	char request[256] = {0};
	u_int32_t len = 0;
	// VER
	request[len] = 0x05;
	// CMD
	len++;
	request[len] = cmd;
	// RSV
	len++;
	request[len] = 0x0;
	// ATYP
	len++;
	request[len] = address_type;
	// DST.ADDR
	len++;
	memcpy(&request[len], dst_addr, dst_addr_len);
	// DST.PORT
	len += dst_addr_len;
	memcpy(&request[len], dst_port, sizeof(dst_port));
	len += sizeof(dst_port);

	memcpy(data, request, len);
	*data_len = len;

	return 0;
}

/*
	+----+-----+-------+------+----------+----------+
	|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
	+----+-----+-------+------+----------+----------+
	| 1  |  1  | X'00' |  1   | Variable |    2     |
	+----+-----+-------+------+----------+----------+
*/
int socks5_server_package_request_reply(char *data, u_int32_t *data_len, const SOCKS5_REP_e rep, const SOCKS5_ATYP_e address_type, const char *bound_addr, const u_int32_t bound_addr_len, const u_int16_t bound_port)
{
	char replies[256] = {0};
	u_int32_t len = 0;
	// VER
	replies[len] = 0x05;
	// REP
	len++;
	replies[len] = rep;
	// RSV
	len++;
	replies[len] = 0x0;
	// ATYP
	len++;
	replies[len] = address_type;
	// DST.ADDR
	len++;
	memcpy(&replies[len], bound_addr, bound_addr_len);
	// DST.PORT
	len += bound_addr_len;
	memcpy(&replies[len], bound_port, sizeof(bound_port));
	len += sizeof(bound_port);

	memcpy(data, replies, len);
	*data_len = len;
	return 0;
}
