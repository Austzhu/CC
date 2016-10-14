/********************************************************************
	> File Name:		SingleOpt.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com
	> Created Time:	2016年06月24日 星期五 10时08分27秒
 *******************************************************************/
#ifndef __SingleOpt_h__
#define __SingleOpt_h__
#include "cmd_process.h"
#include "log.h"
#include "database.h"
#include "coordinate.h"

#define SingleRecvTimeout 	30
#define SingleSendTimeout 	5000000
#define GetSingleCunt 		30 		//一次读取单灯回复的状态信息的单灯个数

#define Single_Header 		0xFF		//单灯数据包头
#define SingleCtrl_Single 		0x01 		//单灯单播控制码
#define SingleCtrl_Group 		0x02 		//单灯组播控制码
#define SingleCtrl_Broadcast	0x04 		//单灯广播控制码
#define CoordiCtrl_Config 	0x10 		//协调器设置控制码
#define CoordiCtrl_Query 		0x20 		//协调器查询控制码
#define CoordiCtrl_GetData	0x40 		//协调器获取数据控制码
#define CoordiCtrl_electric 	0x50 		//读取电压电流值
#define CoordiCtrl_Qelec 		0x30		//查询电流电压
#define ResponseCtrl 			0x80 		//回复控制码

enum ConfigM{
	Single_ConfigMapAddr,
	Single_ConfigGroup,
	Coordi_ConfigMapAddr,
	Coordi_ConfigGroup,
};

enum Action{
	Action_GroupOpen,
	Action_GroupClose,
	Action_BroadcastOpen,
	Action_BroadcastClose
};

typedef struct {//查询时单灯地址表结构
	vu8 Coordi_Addr;
	vu8 Single_Addr[2];
	vu8 Single_State;
	vu8 Single_Light;
} SingleList_t;

extern s32 SingleShortAckToServer(int cmd,u8 ctrl, u8 Resault,...);
extern s32 Single_Update(struct task_node *node);
extern s32 Coordinator_Update(struct task_node *node);
extern void Display_package(const char *Message,void *package,int len);
extern s32 Single_Config(int cmd,void *PSingle);

extern s32 SingleOpen(struct task_node *node);
extern s32 GroupOpen(struct task_node *node);
extern s32 BroadcastOpen(struct task_node *node);
extern s32 SingleClose(struct task_node *node);
extern s32 GroupClose(struct task_node *node);
extern s32 BroadcastClose(struct task_node *node);
extern s32 SingleQuery(struct task_node *node);
extern s32 GroupQuery(struct task_node *node);
extern s32 BroadcastQuery(struct task_node *node);
extern s32 Broadcas_electric(struct task_node *node);


// extern s32 SingleLight(struct task_node *node);
extern s32 GroupLight(struct task_node *node);
extern s32 BroadcastLight(struct task_node *node);
#endif
