/********************************************************************
	> File Name:	taskque.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月30日 星期一 14时15分17秒
 *******************************************************************/
#include "taskque.h"
#include "Callback_function.h"

#define Clean_list(head,type,member)  do{\
	type *pos = list_entry((*head)->member.next, type, member);\
	while( pos != *head){\
		list_del_entry(&pos->member);\
		printf("free %p\n",pos);\
		free(pos);\
		pos = list_entry((*head)->member.next, type, member);\
	}\
	free(*head);\
	*head = NULL;\
}while(0)

typedef struct{u8 cmdname;u8 level;} Que_classify_t;
static Que_classify_t  Que_classify[][255] = {
	{	//0未使用
		{0xff,0xff},
	},
	{	//1 ----CC-08集中器本身相关	协议定义
		{0x80,0x04},{0xA2,0x01},{0xA3,0x01},{0xA4,0x01},{0xA7,0x02},
		{0xC0,0x02},{0xC1,0x02},{0xC2,0x02},{0xC3,0x02},
		{0xF1,0x03},{0xF2,0x03},{0xF3,0x03},{0xFE,0x03},{0xFF,0xff},
	},
	{	//2 ----CC-08集中器本身相关	协议定义
		{0x51,0x01},{0x52,0x02},{0xff,0xff},
	},
	{	//3 -----   485接口相关		协议定义
		{0X01,0x02},{0x02,0x02},{0x03,0x02},
		{0xE1,0x03},{0xff,0xff},
	}
};

Proclist_t  ProcessFunc[][64]={
	{
		{0xff, NULL},
	},
	/* 1 Task_type_cc */
	{
		{0x80,CallBack_answer},
		{0xA2,CallBack_Reset},
		{0xA3,CallBack_Config},
		{0xff, NULL},
	},
	/* 2 Task_type_ethernet */
	{
		{0x51,CallBack_Response},
		{0x52,CallBack_Response},
 		{0xff, NULL},
	},
	/* 3 Task_type_device485 */
	{
		{0x01,CallBack_single},
		{0x02,CallBack_group},
		{0x03,CallBack_broadcast},
		{0xE1,CallBack_meter},
		{0xff, NULL},

	}
};

static s32 get_Quetype(struct Queue_t *this,u8 ctrl)
{
	for(int i=0;i<sizeof(Que_classify)/sizeof(Que_classify[0]);++i){
		for(int j=0;Que_classify[i][j].cmdname != 0xff;++j){
			if(ctrl == Que_classify[i][j].cmdname)  return i | (Que_classify[i][j].level <<8);
		}
	}
	return -1;
}

static void Clean_Que(Queue_t *Que)
{
	assert_param(Que,NULL,;);
	#if 0
	Taskque_t *_pos = Que->Que_header;
	do{
		printf("bdvjsvjsj\n");
		Clean_list(&_pos->Node_header,Node_t,entries);
		_pos = list_entry(_pos->entries.next,Taskque_t,entries);

	}while(_pos != Que->Que_header);
	#endif
	Clean_list(&Que->Que_header,Taskque_t,entries);
}
static int Queue_Init(Queue_t *this,void *parent)
{
	assert_param(this,"Queue init this pointer is NULL!",FAIL);
	Taskque_t *temp = NULL;
	this->parent = parent;
	/* create Queue list */
	for(int i=0; i <= Task_type_device485;++i){
		/* calloc for Queue header */
		temp = calloc(1,sizeof(Taskque_t));
		if(!temp){
			debug(DEBUG_Queue,"calloc for task queue error!\n");
			return FAIL;
		}
		INIT_LIST_HEAD(&temp->entries);
		temp->queue_type = i;
		pthread_mutex_init(&temp->task_queue_lock,NULL);
		/* calloc for task node header */
		temp->Node_header = calloc(1,sizeof(Node_t));
		if( !temp->Node_header)  return FAIL;
		INIT_LIST_HEAD(&temp->Node_header->entries);
		temp->Node_header->task_type = 0;
		temp->Node_header->task_level = 0;
		temp->Node_header->package_len = 0;
		temp->Node_header->tm = time(NULL);

		if(this->Que_header == NULL){
			this->Que_header = temp;
			temp = NULL;
		}else{
			list_add_tail(&temp->entries, &this->Que_header->entries);
		}
	}
	return SUCCESS;
}


static void Queue_Relese(Queue_t *this)
{
	Clean_Que(this);
}

static s32 Task_Append(Queue_t *this,u32 Que_type,u32 task_level,void *pakect,int size)
{
	assert_param(pakect,NULL,FAIL);

	Node_t *node = malloc(size+sizeof(Node_t));
	if(!node)  return FAIL;
	node->task_type = Que_type;
	node->task_level = task_level;
	node->package_len = size;
	node->tm = time(NULL);
	memcpy(node->package,pakect,size);

	Taskque_t *pos = NULL;
	list_for_each_entry(pos,&this->Que_header->entries,entries){
		if(Que_type == pos->queue_type) {
			pthread_mutex_lock(&pos->task_queue_lock); 	//get the task lock
			list_add_tail(&node->entries,&pos->Node_header->entries);
			pthread_mutex_unlock(&pos->task_queue_lock);	//relese the task lock
			return SUCCESS;
		}
	}
	free(node);
	return FAIL;
}

static s32 Select_Task(Queue_t *this,Node_t **node)
{
	Taskque_t *pos = NULL;
	list_for_each_entry(pos,&this->Que_header->entries,entries){
		if(pos->Node_header->entries.next != &pos->Node_header->entries){
			*node = list_first_entry(&pos->Node_header->entries,Node_t,entries);
			pthread_mutex_lock(&pos->task_queue_lock); 	//get the task lock
			list_del_entry(&(*node)->entries);
			pthread_mutex_unlock(&pos->task_queue_lock);	//relese the task lock
			return SUCCESS;
		}
	}
	return FAIL;
}

static s32 Task_Exec(Queue_t *this)
{
	Node_t *node = NULL;
	if(SUCCESS != Select_Task(this,&node) || !node) goto out;
	//printf("task type=%d,task level=%d,time:%s\n",node->task_type,node->task_level,ctime(&node->tm));
	PureCmdArry_t *p_node = (PureCmdArry_t*)node->package;
	Proclist_t *plist = ProcessFunc[node->task_type];
	int return_values = 0;
	while(0xff != plist->ctrl){
		if(p_node->ctrl == plist->ctrl){
			/* 执行函数 */
			if(plist->pf){
				return_values = plist->pf(node,this->parent);
			}
		}
		++plist;
	}
	free(node);
	return return_values;
out:
	return FAIL;
}


Queue_t  Que = {
	.Que_header = NULL,
	.Task_Append = Task_Append,
	.Task_Exec =Task_Exec,
	.Que_init = Queue_Init,
	.Que_release = Queue_Relese,
	.get_Quetype = get_Quetype,
};
