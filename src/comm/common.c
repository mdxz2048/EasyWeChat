/*
 * @Author: MDXZ
 * @Date: 2022-05-03 07:51:32
 * @LastEditTime: 2022-05-03 07:58:14
 * @LastEditors: MDXZ
 * @Description: 
 * @FilePath: /EasyWechat/src/comm/common.c
 * 
 */
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h>	//hostent
#include<arpa/inet.h>
#include "common.h"
/**
 * @description: Get ip from domain name
 * @param {char} *hostname
 * @param {char} *ip
 * @return {*}
 */
int comm_hostname_to_ip(char *hostname , char *ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
		
	if ( (he = gethostbyname( hostname ) ) == NULL) 
	{
		// get the host info
		herror("gethostbyname");
		return -1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	
	return -1;
}