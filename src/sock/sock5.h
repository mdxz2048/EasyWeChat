/*
 * @Author      : MDXZ
 * @Date        : 2022-05-01 17: 06: 40
 * @LastEditTime: 2022-05-04 13:14:08
 * @LastEditors: MDXZ
 * @Description :
 * @FilePath: /EasyWechat/src/sock/sock5.h
 *
 */

#ifndef MDXZ_BRIDGE_C_SOCK5_H
#define MDXZ_BRIDGE_C_SOCK5_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define SOCK5_VALIDATE_REQUEST_MAX_LENGTH 3
#define SOCK5_BUILD_REQUEST_MAX_LENGTH 263

#define SOCKS5_VERSION 0x05
#define SOCKS5_AUTH_PASSWORD_USERNAME_MAX_LENGTH 256
#define SOCKS5_AUTH_PASSWORD_PASSWORD_MAX_LENGTH 256
#define SOCKS5_ADDR_IPV4_LENGTH 4
#define SOCKS5_ADDR_IPV6_LENGTH 16
#define SOCKS5_ADDR_DOMAIN_MAX_LEN 256

typedef enum
{
    SOCKS5_AUTH_NONE = 0X00,
    SOCKS5_AUTH_GSSAPI = 0X01,
    SOCKS5_AUTH_PASSWORD = 0X02,
    ISOCKS5_AUTH_ANA_ASSIGNED = 0x03,
    SOCKS5_AUTH_INVALID = 0XFF,
} SOCKS5_METHOD_e;

typedef enum
{
    SOCKS5_CMD_CONNECT = 0X01,
    SOCKS5_CMD_BIND = 0X02,
    SOCKS5_CMD_UDP = 0X03,
} SOCKS5_CMD_e;

typedef enum
{
    SOCKS5_ATYP_IPv4 = 0X01,
    SOCKS5_ATYP_DOMAIN = 0X03,
    SOCKS5_ATYP_IPv6 = 0X04,
} SOCKS5_ATYP_e;

typedef enum
{
    SOCKS5_REP_SUCCEEDED = 0X00,
    SOCKS5_REP_SERVER_FAILURE = 0x01,
    SOCKS5_REP_CONNECTION_NOT_ALLOWED = 0X02,
    SOCKS5_REP_NETWORK_UNREACHABLE = 0X03,
    SOCKS5_REP_HOST_UNREACHABLE = 0X04,
    SOCKS5_REP_CONNECTION_REFUSED = 0X05,
    SOCKS5_REP_TTL_EXPIRED = 0X06,
    SOCKS5_REP_COMMAND_NOT_SUPPORTED = 0X07,
    SOCKS5_REP_ADDRESS_TYPE_NOT_SUPPORTED = 0X08,
} SOCKS5_REP_e;

/**
+----+----------+----------+
|VER | NMETHODS | METHODS  |
+----+----------+----------+
| 1  |    1     | 1 to 255 |
+----+----------+----------+
 */
typedef struct
{
    u_int8_t version; /*0x05*/
    u_int8_t methodNum;
    SOCKS5_METHOD_e methods[1];
} SOCKS5_METHOD_REQ_t;

/*
+----+--------+
|VER | METHOD |
+----+--------+
| 1  |   1    |
+----+--------+
*/
typedef struct
{
    u_int8_t version;
    SOCKS5_METHOD_e method; /*0xFF is none*/
} SOCKS5_METHOD_REPLY_t;

typedef struct
{
    u_int8_t version;
    u_int8_t username_len; /*1~255*/
    u_int8_t username[SOCKS5_AUTH_PASSWORD_USERNAME_MAX_LENGTH];
    u_int8_t pwd_len; /*1~255*/
    u_int8_t pwd[SOCKS5_AUTH_PASSWORD_PASSWORD_MAX_LENGTH];
} SOCKS5_AUTH_PASSWORD_t;

static const int socks5_auth_reply_ver = 0x01;
static const int socks5_auth_reply_passed = 0x00;

typedef struct
{
    u_int8_t ver;
    u_int8_t status;
} SOCKS5_AUTH_REPLY_t;

typedef struct
{
    u_int8_t addr[16];
} socks5_dst_addr_ipv6_t;

typedef struct
{
    u_int8_t domain_len;
    u_int8_t domain[SOCKS5_ADDR_DOMAIN_MAX_LEN];
    /* u_int16_t port; */
} socks5_dst_addr_domain_t;

typedef union
{
    uint32_t addr_ipv4;
    socks5_dst_addr_domain_t addr_domain;
    char addr_ipv6[6];
} ADDR_u;

/*
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
*/

typedef struct
{
    u_int8_t version;
    SOCKS5_CMD_e cmd;
    u_int8_t rsv;
    SOCKS5_ATYP_e atyp; /*IP V4 address: X'01',DOMAINNAME: X'03',IP V6 address: X'04'*/
    ADDR_u dst_addr;
    u_int16_t dst_port;
} SOCKS5_REQUEST_t;

/*
+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
*/
typedef struct
{
    u_int8_t version;
    SOCKS5_REP_e rep;
    u_int8_t rsv;
    SOCKS5_ATYP_e atyp;
    ADDR_u bndAddr;
    u_int16_t bndPort;
} socks5_reply_t;

typedef struct CLIENT_SOCKET
{
    bool isvalid;      //该客户端有效标志位
    int socket_client; //客户端socket
    int socket_srv;
    int socket_dst;
    SOCKS5_METHOD_e method;
    SOCKS5_AUTH_PASSWORD_t username_password;
} CLIENT_SOCKET_t;

int socks5_client_package_version_method(char *data, u_int8_t *data_len, const u_int8_t nmethods_num, const SOCKS5_METHOD_e methods);
int socks5_server_parse_version_method(SOCKS5_METHOD_e *method, const char *data, const u_int32_t data_len);
/**
 * @description:
 * @param {char} *data
 * @param {u_int8_t} *data_len
 * @param {SOCKS5_METHOD_e} methods
 * @return {*}
 */
int socks5_server_package_method_reply(char *data, u_int8_t *data_len, const SOCKS5_METHOD_e methods);
int socks5_client_package_request(char *data, u_int32_t *data_len, const SOCKS5_REQUEST_t *req);
int socks5_server_parse_request(SOCKS5_REQUEST_t *req, const char *data, const u_int32_t data_len);

#endif // MZZ_BRIDGE_C_SOCK5_H
