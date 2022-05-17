#ifndef SOCKS_SERVER_H
#define SOCKS_SERVER_H

#include "sock5.h"

void start_server(u_int32_t port, SOCKS5_METHOD_e method, u_int8_t username_len, u_int8_t *username, u_int8_t password_len, u_int8_t *password);

#endif