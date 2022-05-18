#ifndef _TCP_CLIENT_H
#define _TCP_CLIENT_H
#include "sock5.h"

int tcp_create_client(SOCKS5_ATYP_e address_type, ADDR_u *dst_addr, u_int16_t dst_port);


#endif