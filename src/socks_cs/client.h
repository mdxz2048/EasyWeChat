#ifndef SOCKS_CLIENT_H
#define SOCKS_CLIENT_H

#include "sock5.h"

int create_local_client(char *server_ip, u_int16_t server_port, uint32_t username_len, char *username, uint32_t password_len, char *password);

#endif