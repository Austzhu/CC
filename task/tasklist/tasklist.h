/********************************************************************
	> File Name:	tasklist.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com 
	> Created Time:	2016年05月30日 星期一 14时15分26秒
 *******************************************************************/
#ifndef __Tasklist_h__
#define __Tasklist_h__
#include "include.h"
#include "list.h"

#define DATASIZE 				300
#define Coordinate_Queue 			0 	//协调器队列，Austzhu 2016.4.13
#define GPRS_TYPE_QUEUE			1
#define CC_TYPE_QUEUE   			2
#define DEVICE485_TYPE_QUEUE 		3
#define ETH_NET_TYPE_QUEUE 			4
#define QUEUE_TOTAL_QUANTITY		5

typedef struct task_node {
	struct list_head  entries;
	u8	task_type;			/*task type*/
	u8	level;
	time_t	t;
	u8	pakect[DATASIZE];		//PAKECT[0] = CMD;[1]=LEN;[2]=DATA[0];.....=CS ;= 0X16
}task_node;

typedef struct task_queue{
	task_node *header;
	u8 queue_type;
	u32 i;
	pthread_mutex_t task_queue_lock;
}task_queue;

typedef struct PureCmdArry{
	UCHAR ctrl;		//控制域C
	UCHAR len;		//data length
	UCHAR data[1];
}PureCmdArry;

typedef struct {
	UCHAR ctrl;
	UINT (*pf)(u8 ctrl,  u8 itf, struct task_node *node);
} ListCmdProcessFunc;

enum{
	 TIMER_TASK,
	 SMS_TASK,
	 NET_TASK,
	 INNER_FAIL_TASK,
	 Coordinate_Task
};

enum{
	Task_Level_Coor,
	TASK_LEVEL_GPRS,
	TASK_LEVEL_NET,
	TASK_LEVEL_CC,
	TASK_LEVEL_PLC,
	TASK_LEVEL_485,
};





extern s32 TopUserQueProc(u8 itf);
extern s32 SelctExcuteOrAppend(u8 queuetype,  u32 mode,  u8 tasktype,  u8 tasklevel,  u8 itf);
extern s32 RecInsertQueue(u8 itf);
extern s32 TaskGenerateAndAppend(u8 queuetype, u8 *data,u8 tasksource,u8 tasklevel);
extern s32 SelectNode(struct task_queue *queue,struct task_node *recv,u8 queuetype);
extern s32 DelTask(struct task_queue *queue,  struct task_node *node,  u8 queuetype);
extern s32 SelectTask(struct task_queue *queue,  u32 mode, struct task_node *recv,  u8 queuetype);
extern s32 appendTask(struct task_queue *queue, struct task_node *node,  u8 queue_type);
extern struct task_node * mallocNode(u8 task_type,  u8 level,  u8 *data, u32 datalen,  time_t t);
extern void InitTaskQueue(void);
extern s32 init_task_queue(struct task_queue *taskqueue);
extern s32 isEmpty(struct task_queue *queue);

#endif
