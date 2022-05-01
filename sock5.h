//
// Created by cayun on 2019-03-15.
//

#ifndef MZZ_BRIDGE_C_SOCK5_H
#define MZZ_BRIDGE_C_SOCK5_H

#include "common.h"

#define SOCK5_VALIDATE_REQUEST_MAX_LENGTH 3
#define SOCK5_BUILD_REQUEST_MAX_LENGTH 263

#define SOCKS5_VERSION                                          0x05
#define SOCKS5_AUTH_PASSWORD_USERNAME_MAX_LENGTH                256
#define SOCKS5_AUTH_PASSWORD_PASSWORD_MAX_LENGTH                256
#define SOCKS5_DST_ADDR_DOMAIN_MAX_LEN                          256


typedef enum{
    SOCKS5_AUTH_NONE        = 0X00,    
    SOCKS5_AUTH_GSSAPI      = 0X01,
    SOCKS5_AUTH_PASSWORD    = 0X03,
    SOCKS5_AUTH_INVALID     = 0XFF,
}SOCKS5_AUTH_e;

/**
+----+----------+----------+
|VER | NMETHODS | METHODS  |
+----+----------+----------+
| 1  |    1     | 1 to 255 |
+----+----------+----------+
 */
typedef struct {
    uint8_t version;/*0x05*/
    uint8_t methodNum;
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
    uint8_t version;
    SOCKS5_AUTH_e method;/*0xFF is none*/
}SOCKS5_METHOD_REPLY_t;


typedef struct  {
    uint8_t version;
    uint8_t username_len;/*1~255*/
    uint8_t username[SOCKS5_AUTH_PASSWORD_USERNAME_MAX_LENGTH];
    uint8_t pwd_len;/*1~255*/
    uint8_t pwd[SOCKS5_AUTH_PASSWORD_PASSWORD_MAX_LENGTH];
}SOCKS5_AUTH_PASSWORD_t;

static const int socks5_auth_reply_ver = 0x01;
static const int socks5_auth_reply_passed = 0x00;

typedef struct {
    uint8_t ver;
	uint8_t status;
}SOCKS5_AUTH_REPLY_t;

typedef struct {
	uint32_t addr[4];
} socks5_dst_addr_ipv4_t;

typedef struct {
	uint8_t addr[16];
} socks5_dst_addr_ipv6_t;

typedef struct {
    uint8_t domain_len;
	uint8_t domain[SOCKS5_DST_ADDR_DOMAIN_MAX_LEN];
	/* uint16_t port; */
} socks5_dst_addr_domain_t;

/*
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
*/
typedef struct {
    uint8_t version;
    uint8_t cmd;
    uint8_t rsv;
    uint8_t atyp; /*IP V4 address: X'01',DOMAINNAME: X'03',IP V6 address: X'04'*/
    union  {
        socks5_dst_addr_ipv4_t addr_ipv4;
        socks5_dst_addr_domain_t addr_domain;
        socks5_dst_addr_ipv6_t addr_ipv6;
    } dst_addr;
    uint16_t dstPort;
}socks5_build_req_t;

/*
+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
*/
typedef struct {
    uint8_t version;
    uint8_t rep;
    uint8_t rsv;
    uint8_t atyp;
    uint8_t addrLength; // 地址长度
    union {
        socks5_dst_addr_ipv4_t addr_ipv4;
        socks5_dst_addr_domain_t addr_domain;
        socks5_dst_addr_ipv6_t addr_ipv6;
    } bndAddr;
    uint16_t bndPort;
}socks5_reply_t;

/**
 * 读取sock5验证请求
 * @param buffer
 * @return
 */
struct Sock5ValidateRequest Sock5ValidateRequest_read(uint8_t *buffer);

/**
 * 获取Sock5ValidateResponse的总长度
 * @param response
 * @return
 */
size_t Sock5ValidateResponse_getLength(struct Sock5ValidateResponse response);

/**
 * 将sock5验证响应转换为字符串
 * @param response
 * @return
 */
char* Sock5ValidateResponse_toString(struct Sock5ValidateResponse response);

/**
 * 读取sock5建立连接请求
 * @param buffer
 * @return
 */
struct Sock5BuildRequest Sock5BuildRequest_read(uint8_t *buffer);

/**
 * 获取Sock5BuildResponse的总长度
 * @param response
 * @return
 */
size_t Sock5BuildResponse_getLength(struct Sock5BuildResponse response);

/**
 * 将sock5建立连接请求转换为字符串
 * @param response
 * @return
 */
char* Sock5BuildResponse_toString(struct Sock5BuildResponse response);

#endif //MZZ_BRIDGE_C_SOCK5_H
