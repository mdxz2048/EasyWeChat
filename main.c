/*
 * @Author: MDXZ
 * @Date: 2022-05-01 17:18:37
 * @LastEditTime: 2022-05-03 07:59:39
 * @LastEditors: MDXZ
 * @Description:
 * @FilePath: /EasyWechat/main.c
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

/*打印编译信息*/
static char g_soft_info[2048] = {0};
static char g_app_version[] = "V_0.0.1";

void printf_soft_info()
{
#ifdef APP_COMPILE_DATE
    sprintf(g_soft_info,
            "+--------------------------------------------------+\n\
|                    easyWeChat                    |\n\
|                                                  |\n\
|           编译时间：%s %s          |\n\
|           软件版本：%s                      |\n\
|           作者:     MDXZ                         |\n\
|                                                  |\n\
+--------------------------------------------------+\n",
            APP_COMPILE_DATE, APP_COMPILE_TIME, g_app_version);
#endif
    printf("%s", g_soft_info);
}

int main(int argc, char **argv)
{
    printf_soft_info();
    if (argc < 2)
    {
        printf("Please provide a hostname to resolve");
        exit(1);
    }

    char *hostname = argv[1];
    char ip[100];

    comm_hostname_to_ip(hostname, ip);
    printf("ip: %s\n", ip);

    return 0;
}