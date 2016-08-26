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
#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__
#include "include.h"

#define DATASIZE 	300				//SIZE   :  256+12 

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

//	LIST_INSERT_HEAD(&queue->header, node, entries);
#define LIST_INSERT_HEAD(head,elem,field)	do{							\
		/*if list is empty*/ 	if(((elem)->field.le_next = (head)->lh_first) != NULL)		\
						(head)->lh_first->field.le_prev = &(elem)->field.le_next;	\
						(head)->lh_first = (elem);				\
						(elem)->field.le_prev = &(head)->lh_first;		\
}while(0)


#define	LIST_REMOVE(elm, field) do {				\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev = 		\
		 (elm)->field.le_prev;				\
		*(elm)->field.le_prev = (elm)->field.le_next;	\
} while (/*CONSTCOND*/0)


#define LIST_ENTRY(type) 		\
	struct {				\
	struct type *le_next;		\
	struct type **le_prev;		\
}

#define LIST_HEAD(name,type)		\
struct name {				\
	struct type *lh_first;		\
}

#define LIST_INIT(head)	do{ 	\
	(head)->lh_first = NULL;	\
}while(0)


typedef struct task_node {
	LIST_ENTRY(task_node) entries;
	UCHAR	task_type;			/*task type*/
	UCHAR	level;
	time_t	t;
	UCHAR	pakect[DATASIZE];		//PAKECT[0] = CMD;[1]=LEN;[2]=DATA[0];.....=CS ;= 0X16
}task_node;

typedef struct task_queue{
	LIST_HEAD(queueheader, task_node) header;
	char queue_type;
	UINT i;
	pthread_mutex_t task_queue_lock;
}task_queue;


extern UINT appendTask(struct task_queue *queue,struct task_node *node,char queue_type);
extern UINT selectTask(struct task_queue *queue,UINT mode, struct task_node *recv,UCHAR queuetype);
extern UINT init_task_queue(struct task_queue *taskqueue);
extern UINT TopUserQueProc(UCHAR itfs);
extern UINT RecInsertQueue(UCHAR itf);
extern UINT InitTaskQueue(void);
extern UINT SelctExcuteOrAppend(UCHAR queuetype,UINT mode,UCHAR tasktype,UCHAR tasklevel,UCHAR itf);
extern UINT TaskGenerateAndAppend(UCHAR queuetype,UCHAR *data,UCHAR tasksource,UCHAR tasklevel);


#endif
