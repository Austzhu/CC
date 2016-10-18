/********************************************************************
	> File Name:	taskque.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月30日 星期一 14时15分26秒
 *******************************************************************/
#ifndef __TASKQUE_H__
#define __TASKQUE_H__

#include "include.h"
#include "list.h"

/* define the type of tak */
enum Task_type{
	Task_type_cc = 1,
	Task_type_ethernet,
	Task_type_device485,
};

/* define  the level for task */
enum Task_level{
	Task_level_cc = 1,
	Task_level_device485,
	Task_level_ethernet,
};

/* define the struct for task node */
typedef struct {
	struct list_head  entries;
	time_t	tm;
	u32  task_type;
	u32  task_level;
	s32 package_len;
	u8	package[0];
} Node_t;

/* define the queue struct for task */
typedef struct {
	struct list_head  entries;
	Node_t *Node_header;
	u32 queue_type;
	pthread_mutex_t task_queue_lock;
} Taskque_t;

/* the task node data struct */
typedef struct {
	u8 ctrl;
	u8 len;
	u8 data[0];
} PureCmdArry_t;

/* the struct for call back function */
typedef struct {
	u8 ctrl;
	s32 (*pf)(Node_t*node);
} Proclist_t;


/* the class for Queue */
typedef struct Queue_t{
	Taskque_t *Que_header;
	Proclist_t (*process_function)[];

	s32 (*Task_Exec)(struct Queue_t*);
	s32 (*Task_Append)(struct Queue_t*,u32 Que_type,u32 Task_level,void *pakect,int size);
	s32 (*get_Quetype)(struct Queue_t*,void*package);
	void (*Que_init) (struct Queue_t*);
	void (*Que_release)(struct Queue_t*);
} Queue_t;

extern Queue_t Que;



#endif
