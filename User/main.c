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
#include "main.h"

volatile s8 	TOPSHOWPERTICK 		= 0;
volatile s32 	TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;			//socket connect state  step-1
volatile s32 	TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;				//设备登录初始状态
volatile s32 	TOPHEARTBEATACK 		= FAIL;
volatile s32 	TOPGLOBALMEMALLOCTIMES;
volatile s32 	TOPGLOBAL_HEAP_SIZE;							//申请堆的大小
volatile s32 	TOPTotalTaskQuantity;
volatile s32 	TOPCCQueueTaskQuantity;
volatile s32 	TOPPLCQueueTaskQuantity;
volatile s32 	TOPGPRSQueueTaskQuantity;
volatile s32 	TOPD485QueueTaskQuantity;
volatile s32 	TOPEthQueueTaskQuantity;
volatile s8 	TOPDIDO_STATUS[DIDO_DEVICE_QUANTITY];
volatile s8 	TOPMETER_STATUS[METER_DEVICE_QUANTITY];
volatile s32 	TOPPLCSendRecivModState;
volatile s32 	TOPPowerModState;
volatile s32 	TOPBatteryCellModState;
volatile s32 	TOPMeterModState;
volatile s32 	TOPDIDOModState;
volatile s32 	TOPCCModState;
volatile s32 	TOPControlMod;
volatile u8 	TOPDeviceOprateRetryTMS[QUEUE_TOTAL_QUANTITY];			//任务失败，是否要重复执行
volatile u8 	TOTAL_TASK_IN_EACH_QUEUE[QUEUE_TOTAL_QUANTITY];
volatile u8 	TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];	//定义DIDO的线路状态
volatile u8 	TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];
volatile u8 	TOP_LOCALREALISTIC_DIINFOR_STAT[DIDO_DEVICE_QUANTITY];
volatile u8 	TOP_SERVERREQUI_LEDGRP_STAT[5];
TOPDevicePowerStruct 			TOPDevicePowerInfor;						//global power information
TOPDeviceMeterStatStruct 		TOPDeviceMeterStat;							//global Metter device status
struct MetterElecInforStruc 		TOPMetterElecInformation;
TOPDeviceGPRSMDStatStruct 	TOPDeviceGPRSModuleInfor;

#ifdef UsePthread
pthread_mutex_t mutex_ether;
pthread_mutex_t mutex_task;
#ifdef SingleCheckThread
 	pthread_mutex_t mutex_serial;
#endif    //fdef SingleCheckThread
#endif   //fdef UsePthread


 void CC_Init(void)
{
	g_appity.app_Init(&g_appity);
	//appitf_init(&g_appity);
	 if( access("cc_corl.db",F_OK)){
		system("./config/Create_Database.sh &");	//重新建数据库
		sleep(1);
	}
	InitTaskQueue();	//初始化队列
	UartForCoordi();	//串口的配置
	InitTimeTASK();		//定时任务的初始化
 #	ifdef UsePthread
	pthread_mutex_init(&mutex_ether,NULL);
	pthread_mutex_init(&mutex_task,NULL);
 #	ifdef SingleCheckThread
 		pthread_mutex_init(&mutex_serial,NULL) ;
 #	endif
 #	endif
}

#ifdef UsePthread

 static void *KeepaliveThread(void *args)
 {
 	while(1){
 		TopUserKeepalive(CCparamGlobalInfor.ItfWay);
 		sleep(CCparamGlobalInfor.HeartBCycle);
 	}
 	return NULL;
 }
static void *RecvInsertQueueThread(void *args)
{
	while(1){
		pthread_mutex_lock(&mutex_task);	//获取task锁
		g_appity.TopUserInsertQue(&g_appity);
		pthread_mutex_unlock(&mutex_task);	//解task锁
		usleep(500000);
	}
	return NULL;
}
static void *UserQueProcThread(void *args)
{
	while(1){
		pthread_mutex_lock(&mutex_task);	//获取task锁
		g_appity.TopUserProcQue(&g_appity);
		pthread_mutex_unlock(&mutex_task);	//解task锁
		usleep(250000);
	}
	return NULL;
}

int main(int argc,char *argv[])
{
	pthread_t thread_Keepalive = -1;
	pthread_t thread_RecvInsert = -1;
	pthread_t thread_UserProc = -1;

	g_appity.app_Init(&g_appity);
	pthread_create(&thread_Keepalive,NULL,KeepaliveThread,NULL);sleep(1);
	pthread_create(&thread_RecvInsert,NULL,RecvInsertQueueThread,NULL);
	pthread_create(&thread_UserProc,NULL,UserQueProcThread,NULL);
	if(thread_Keepalive < 0 || thread_RecvInsert < 0 || thread_UserProc < 0 ){
		debug(1,"one of pthread_create error!\n");
		exit(-1);
	}
	pthread_join(thread_Keepalive,NULL);
	pthread_join(thread_RecvInsert,NULL);
	pthread_join(thread_UserProc,NULL);

	return 0;
}


#else


int main(int argc,char *argv[])
{
	CC_Init();
	while(1){
		TopUserKeepalive(CCparamGlobalInfor.ItfWay);
		RecInsertQueue(CCparamGlobalInfor.ItfWay);
		TopUserQueProc(CCparamGlobalInfor.ItfWay);
	}
	return 0;
}


#endif
