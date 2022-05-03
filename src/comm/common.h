#ifndef __COMMON__H
#define __COMMON__H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define debug_printf(format,...)     printf("\033[30m\033[32m"format"\033[0m\n", ##__VA_ARGS__)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * @description: Get ip from domain name
 * @param {char} *hostname
 * @param {char} *ip
 * @return {*}
 */
int comm_hostname_to_ip(char *hostname , char *ip);


#endif