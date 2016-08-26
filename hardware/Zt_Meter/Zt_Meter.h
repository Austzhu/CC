/********************************************************************
	> File Name:		Zt_Meter.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com
	> Created Time:	2016年07月04日 星期一 16时19分48秒
 *******************************************************************/
#ifndef __ZT_METER_H__
#define __ZT_METER_H__
#include "cmd_process.h"

#define DIDO_nLenALL 	10 	//操作输出继电器全开或者全关的数据长度
#define DIDO_nLenOC 		8	//打开或关闭输出继电器的数据长度
#define DIDO_nLenRead 	8	//查询继电器或者DI的状态数据长度

/* 定义几路DI和几路DO */
#define DIDO_8DO		8
#define DIDO_8DI		8

#define DIDO_All 		0xFF

struct DAM0808{
	u8 Addr;
	u8 Cmd;
	u8 Data[8];
};

enum DIDO_RespondCmd{
	Respond_Open,
	Respond_Close,
	Respond_CheckDO,
	Respond_CheckDI
};

extern s32 DIDO_Open(struct task_node *node);
extern s32 DIDO_Close(struct task_node *node);
extern s32 DIDO_ReadDO(struct task_node *node);
extern s32 DIDO_ReadDI(struct task_node *node);

#endif //#ifndef __ZT_METER_H__
