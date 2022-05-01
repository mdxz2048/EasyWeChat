//
// Created by mdxz (zhipeng2048@gmail.com)
//

#ifndef MZZ_BRIDGE_C_SOCK5_H
#define MZZ_BRIDGE_C_SOCK5_H

#include <sys/types.h>
#include <sys/socket.h>
#include "comm.h"



#define SOCK5_VALIDATE_REQUEST_MAX_LENGTH 3
#define SOCK5_BUILD_REQUEST_MAX_LENGTH 263

#define SOCKS5_VERSION                                          0x05
#define SOCKS5_AUTH_PASSWORD_USERNAME_MAX_LENGTH                256
#define SOCKS5_AUTH_PASSWORD_PASSWORD_MAX_LENGTH                256
#define SOCKS5_DST_ADDR_DOMAIN_MAX_LEN                          256


typedef enum{
    SOCKS5_AUTH_NONE          = 0X00,
    SOCKS5_AUTH_GSSAPI        = 0X01,
    SOCKS5_AUTH_PASSWORD      = 0X02,
    ISOCKS5_AUTH_ANA_ASSIGNED = 0x03,
    SOCKS5_AUTH_INVALID       = 0XFF,
}SOCKS5_AUTH_e;

/**
+----+----------+----------+
|VER | NMETHODS | METHODS  |
+----+----------+----------+
| 1  |    1     | 1 to 255 |
+----+----------+----------+
 */
typedef struct {
    u_int8_t version;/*0x05*/
    u_int8_t methodNum;
    SOCKS5_AUTH_e methods[1];
}SOCKS5_METHOD_REQ_t;


/*
+----+--------+
|VER | METHOD |
+----+--------+
| 1  |   1    |
+----+--------+
*/
typedef struct {
    u_int8_t version;
    SOCKS5_AUTH_e method;/*0xFF is none*/
}SOCKS5_METHOD_REPLY_t;


typedef struct  {
    u_int8_t version;
    u_int8_t username_len;/*1~255*/
    u_int8_t username[SOCKS5_AUTH_PASSWORD_USERNAME_MAX_LENGTH];
    u_int8_t pwd_len;/*1~255*/
    u_int8_t pwd[SOCKS5_AUTH_PASSWORD_PASSWORD_MAX_LENGTH];
}SOCKS5_AUTH_PASSWORD_t;

static const int socks5_auth_reply_ver = 0x01;
static const int socks5_auth_reply_passed = 0x00;

typedef struct {
    u_int8_t ver;
	u_int8_t status;
}SOCKS5_AUTH_REPLY_t;

typedef struct {
	u_int32_t addr[4];
} socks5_dst_addr_ipv4_t;

typedef struct {
	u_int8_t addr[16];
} socks5_dst_addr_ipv6_t;

typedef struct {
    u_int8_t domain_len;
	u_int8_t domain[SOCKS5_DST_ADDR_DOMAIN_MAX_LEN];
	/* u_int16_t port; */
} socks5_dst_addr_domain_t;

/*
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
*/
typedef struct {
    u_int8_t version;
    u_int8_t cmd;
    u_int8_t rsv;
    u_int8_t atyp; /*IP V4 address: X'01',DOMAINNAME: X'03',IP V6 address: X'04'*/
    union  {
        socks5_dst_addr_ipv4_t addr_ipv4;
        socks5_dst_addr_domain_t addr_domain;
        socks5_dst_addr_ipv6_t addr_ipv6;
    } dst_addr;
    u_int16_t dstPort;
}socks5_build_req_t;

/*
+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
*/
typedef struct {
    u_int8_t version;
    u_int8_t rep;
    u_int8_t rsv;
    u_int8_t atyp;
    u_int8_t addrLength; // 地址长度
    union {
        socks5_dst_addr_ipv4_t addr_ipv4;
        socks5_dst_addr_domain_t addr_domain;
        socks5_dst_addr_ipv6_t addr_ipv6;
    } bndAddr;
    u_int16_t bndPort;
}socks5_reply_t;

#endif //MZZ_BRIDGE_C_SOCK5_H
