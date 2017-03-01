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
	Task_count,
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
	uint32_t  task_type;
	uint32_t  task_level;
	int32_t package_len;
	uint8_t	package[0];
} Node_t;

/* define the queue struct for task */
typedef struct {
	struct list_head  entries;
	Node_t *Node_header;
	uint32_t queue_type;
	pthread_mutex_t task_queue_lock;
} Taskque_t;

/* the task node data struct */
typedef struct {
	uint8_t ctrl;
	uint8_t len;
	uint8_t data[0];
} PureCmdArry_t;


/* the struct for call back function */
typedef struct {
	uint8_t ctrl;
	int32_t (*pf)(Node_t*node,void*);
} Proclist_t;

struct appitf_t;
/* the class for Queue */
typedef struct Queue_t{
	int32_t Point_flag;
	Taskque_t *Que_header;
	struct appitf_t *parent;
	int32_t (*Task_Exec)(struct Queue_t*);
	int32_t (*Task_Append)(struct Queue_t*,u32,u32 ,void *,int);
	int32_t (*get_Quetype)(struct Queue_t*,u8 ctrl);
	void (*Que_release)(struct Queue_t*);
} Queue_t;

extern Queue_t *Queue_Init(Queue_t*,struct appitf_t *topuser);

#endif
