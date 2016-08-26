#ifndef __TASK_QUE_H__
#define __TASK_QUE_H__
#include "frame.h"
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
/**
 * 任务类型枚举定义
 */
typedef enum{
	Task_net,
	Task_485
} Task_Type;

/**
 * 任务等级枚举
 */
typedef enum{
	Task_level_1,
	Task_level_2,
	Task_level_3,
	Task_level_4
}Task_Level;

/**
 * 队列类型
 */
typedef enum {
	server2cc,	//服务器下行
	cc2server,	//服务器上行
	cc2coordinate,	//协调器下行
}Que_type;
/**
 * 任务节点结构
 */
typedef struct node{
	void* 		value;		//节点的数据，指向data数据
	u32 		datalen;	//数据的长度
	Task_Type 	task_type;	//任务类型
	Task_Level 	task_level;	//任务等级
	struct list_head enter;		//链接节点(双链表)
} Node;

/**
 * 队列节点
 */
typedef struct {
	Node *			task_header;		//任务队列的头指针
	Que_type 		que_type;		//队列类型
	pthread_mutex_t 	lock;	//互斥量
}Task_Que;

typedef struct PureCmdArry{
	UCHAR ctrl;		//控制域C
	UCHAR len;		//data length
	UCHAR data[1];
} PureCmdArry;

typedef struct {
	UCHAR ctrl;
	UINT (*pf)(Node *node);
} ListCmdProcessFunc;

typedef enum{
	INIT_QUE ,
	INSERT_NODE ,
	IS_EMPTY ,
	SELECT_TASK ,
} List_Interface;

/**
 *  根据cmd传入的命令调用不同的函数
 */
extern s32 List_Operation(List_Interface cmd, ...);

#endif