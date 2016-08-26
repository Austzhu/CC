/********************************************************************
	> File Name:	main.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com
	> Created Time:	2016年06月21日 星期二 10时00分37秒
 *******************************************************************/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "include.h"
#include "loadfile.h"
#include "database.h"


extern BurglarAlarmGroupGL BurglarAlarmElemList;
extern struct task_queue taskQueue[QUEUE_TOTAL_QUANTITY];
extern GlobalCCparam CCparamGlobalInfor;
extern struct TOPTimerTaskListStruct TOPTimerTaskList;		//1 :合；0: 分											
extern TimerTaskToSaveFormatStruct TimerTaskList[5];


extern volatile s8 	TOPSHOWPERTICK ;
extern volatile s32 	TOPSocketConnectState;							//socket connect state  step-1
extern volatile s32 	TOPCCRegistr2ServerState; 							//设备登录初始状态
extern volatile s32 	TOPHEARTBEATACK;
extern volatile s32 	TOPGLOBALMEMALLOCTIMES;
extern volatile s32 	TOPGLOBAL_HEAP_SIZE;							//申请堆的大小
extern volatile s32 	TOPTotalTaskQuantity;
extern volatile s32 	TOPCCQueueTaskQuantity;
extern volatile s32 	TOPPLCQueueTaskQuantity;
extern volatile s32 	TOPGPRSQueueTaskQuantity;
extern volatile s32 	TOPD485QueueTaskQuantity;
extern volatile s32 	TOPEthQueueTaskQuantity;
extern volatile s8 	TOPDIDO_STATUS[DIDO_DEVICE_QUANTITY];
extern volatile s8 	TOPMETER_STATUS[METER_DEVICE_QUANTITY];
extern volatile s32 	TOPPLCSendRecivModState;
extern volatile s32 	TOPPowerModState;
extern volatile s32 	TOPBatteryCellModState;
extern volatile s32 	TOPMeterModState;
extern volatile s32 	TOPDIDOModState;
extern volatile s32 	TOPCCModState;
extern volatile s32 	TOPControlMod;
extern volatile u8 	TOPDeviceOprateRetryTMS[QUEUE_TOTAL_QUANTITY];			//任务失败，是否要重复执行
extern volatile u8 	TOTAL_TASK_IN_EACH_QUEUE[QUEUE_TOTAL_QUANTITY];											
extern volatile u8 	TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];			//定义DIDO的线路状态
extern volatile u8 	TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];			
extern volatile u8 	TOP_LOCALREALISTIC_DIINFOR_STAT[DIDO_DEVICE_QUANTITY];
extern volatile u8 	TOP_SERVERREQUI_LEDGRP_STAT[5];
extern TOPDevicePowerStruct 		TOPDevicePowerInfor;						//global power information
extern TOPDeviceMeterStatStruct 	TOPDeviceMeterStat;						//global Metter device status											
extern struct MetterElecInforStruc 	TOPMetterElecInformation;
extern TOPDeviceGPRSMDStatStruct 	TOPDeviceGPRSModuleInfor;

extern pthread_mutex_t mutex_ether;
extern pthread_mutex_t mutex_task;
#ifdef SingleCheckThread
 extern pthread_mutex_t mutex_serial;
#endif   



#endif


