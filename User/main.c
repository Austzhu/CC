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

static void *KeepaliveThread(void *args)
{
	while(g_appity.pthread_start) g_appity.TopUser_Keepalive(&g_appity);
	#ifdef  CFG_exitMessage
		printf("Top KeepaliveThread exit!\n");
	#endif
	return NULL;
}

static void *RecvInsertQueueThread(void *args)
{
	while(g_appity.pthread_start) g_appity.TopUser_InsertQue(&g_appity);
	#ifdef  CFG_exitMessage
		printf("Top RecvInsertQueueThread exit!\n");
	#endif
	return NULL;
}

static void *UserQueProcThread(void *args)
{
	while(g_appity.pthread_start) g_appity.TopUser_ProcQue(&g_appity);
	#ifdef  CFG_exitMessage
		printf("Top UserQueProcThread exit!\n");
	#endif
	return NULL;
}

int main(int argc,char *argv[])
{
	pthread_t thread_Keepalive 	= -1;
	pthread_t thread_RecvInsert 	= -1;
	pthread_t thread_UserProc 	= -1;
	if(SUCCESS != g_appity.TopUser_Init(&g_appity)){
		debug(DEBUG_app,"system init error!\n");
		g_appity.TopUser_relese(&g_appity);
	}


	pthread_create(&thread_Keepalive,NULL,KeepaliveThread,NULL);sleep(1);
	pthread_create(&thread_RecvInsert,NULL,RecvInsertQueueThread,NULL);
	pthread_create(&thread_UserProc,NULL,UserQueProcThread,NULL);

	if(thread_Keepalive < 0 || thread_RecvInsert < 0 || thread_UserProc < 0 ){
		debug(1,"one of pthread_create error!\n");
		g_appity.TopUser_relese(&g_appity);
	}

	pthread_join(thread_Keepalive,NULL);
	pthread_join(thread_RecvInsert,NULL);
	pthread_join(thread_UserProc,NULL);
	return 0;
}

