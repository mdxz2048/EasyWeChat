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

	*data_len = len + 1;

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
int socks5_server_package_method_reply(char *data, u_int8_t *data_len, const SOCKS5_METHOD_e methods)
{
	*data = 0x05;
	*(data + 1) = methods;

	*data_len = 2;

	return 0;
}