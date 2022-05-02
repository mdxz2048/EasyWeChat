#include <stdio.h>

/*打印编译信息*/
static char g_soft_info[2048] = {0};
static char g_app_version[] = "easyWeChat_0.0.1";


void printf_soft_info()
{
#ifdef APP_COMPILE_DATE
    sprintf(g_soft_info,
            "+--------------------------------------------------+\n\
|               CAERI FOR RSU(ITS2020)             |\n\
|                                                  |\n\
|           编译时间：%s %s          |\n\
|           软件版本：%s              |\n\
|           作者:MDXZ                               |\n\
|                                                  |\n\
+--------------------------------------------------+\n",
            APP_COMPILE_DATE, APP_COMPILE_TIME, g_app_version);
#endif
    printf("%s", g_soft_info);
}

int main(int argc, char **argv)
{
    printf_soft_info();

    return 0;
}