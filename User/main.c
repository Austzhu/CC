/******************************************************************
 ** 文件名:	main.c
 ** Copyright (c) 2012-2014 *********公司技术开发部
 ** 创建人:	wguilin
 ** 日　期:	2012.03
 ** 修改人:
 ** 日　期:
 ** 描　述:	16  进制打印
 ** ERROR_CODE:
 **
 ** 版　本:	V1.0
*******************************************************************/
#include "Interface.h"

int main(int argc,char *argv[])
{
	int res = g_appity.TopUser_Init(&g_appity);
	if(res != SUCCESS){
		debug(DEBUG_app,"system init error!\n");
		g_appity.TopUser_relese(&g_appity);
	}
	while(1){
		sleep(1000);
	}
}

