/******************************************************************
** 文件名:
** Copyright (c) 1998-1999 *********公司技术开发部
** 创建人:
** 日　期:
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:
*******************************************************************/
#ifndef __TASK_H__
#define __TASK_H__

#include "base_type.h"
#include "task_list.h"

//#define PLC_TYPE_QUEUE 			0
#define Coordinate_Queue 			0 	//协调器队列，Austzhu 2016.4.13
#define GPRS_TYPE_QUEUE			1
#define CC_TYPE_QUEUE   			2
#define DEVICE485_TYPE_QUEUE 		3
#define ETH_NET_TYPE_QUEUE 		4
#define QUEUE_TOTAL_QUANTITY		5

typedef struct PureCmdArry{
	UCHAR ctrl;		//控制域C
	UCHAR len;		//data length
	UCHAR data[1];
} PureCmdArry;


typedef struct {
	UCHAR ctrl;
	UINT (*pf)(UCHAR ctrl,UCHAR itf, struct task_node *node);
} ListCmdProcessFunc;

#endif
