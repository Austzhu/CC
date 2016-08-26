#include "task_que.h"

static Task_Que Queue[]={
	[server2cc]	={ },
	[cc2server]	={ },
	[cc2coordinate]	={ }
};
const ListCmdProcessFunc DetlCmdProcessFunc[][200] = {
	{		
		{0xff, NULL},
	},
	
};

static u32 Task_max = sizeof(Queue)/sizeof(Queue[0]);
#define ISQUEUE(QUE)  { if(QUE <0 || QUE > Task_max ) \
				return -1;}

/**
 * 创建头节点
 * @param  Qtype 队列类型
 * @return       
 * 	成功 节点指针;  失败 NULL;
 */
static Node* Creat_Head_Node(Que_type Qtype)
{
	Node* pdata = (Node*)malloc(sizeof(Node));
	if(NULL == pdata){
		perror("malloc head_node error!");
		return NULL;
	}
	pdata->value = NULL;
	pdata->datalen = 0;
	pdata->task_type = Qtype;
	pdata->task_level = -1;
	INIT_LIST_HEAD(&(pdata->enter));
	return pdata;
}

/**
 * 创建队列
 * @param  Qtype 队列类型
 * @return       
 * 	成功 0;  失败 -1;
 */
static s32 Create_Queue(Que_type Qtype)
{
	ISQUEUE(Qtype);
	Queue[Qtype].que_type = Qtype;
	pthread_mutex_init (&(Queue[Qtype].lock), NULL);
	Queue[Qtype].task_header = Creat_Head_Node(Qtype);

	if(NULL == Queue[Qtype].task_header){
		perror("create head error!");
		return -1;
	}
	return 0;
}

/**
 * 把节点按照task的任务等级插入到队列中去
 * @param  Qtype 队列类型
 * @param  data  要插入的节点
 * @return       
 * 	成功 0 ; 失败 -1
 */
static s32 Insert_Node(Que_type  Qtype, const Node* data)
{
	ISQUEUE(Qtype);
	if(NULL == data ){
		return -1;
	}

	u32 i = 0;
	u32 num = (u32)(Queue[Qtype].task_header->value);
	Node* pdata = (Node*)malloc(sizeof(Node));
	Node* index  = list_next_entry(Queue[Qtype].task_header,enter);
	if(NULL == pdata){
		perror("create head error!");
		return -1;
	}
	pdata->value = NULL;
	while( !pdata->value && i++<3){
		pdata->value = malloc(data->datalen);
	}
	
	if( NULL == pdata->value){
		perror("malloc data error !");
		free(pdata);
		return -1;
	}
	/* 给节点初始化 */
	memcpy(pdata->value,data->value,data->datalen);
	pdata->datalen = data->datalen;
	pdata->task_level = data->task_level;
	pdata->task_type =data->task_type;

	/* 要根据节点的等级查找插入点 */
	while( pdata->task_level  >= index->task_level  && index != Queue[Qtype].task_header){
		index = list_next_entry( index,enter);
	}
	list_add_tail(&(pdata->enter), &(index->enter) );

	/**
	 * 头节点的数据指针value用来存放节点的个数，
	 * 不用于存取数据(就是把指针变量当成int变量使用)
	 */
	Queue[Qtype].task_header->value = (void*)(++num);

	return 0;
}

/**
 * 初始化队列
 * @return  0
 */
static s32 Init_Queue(void)
{
	Create_Queue(server2cc);
	Create_Queue(cc2server);
	Create_Queue(cc2coordinate);
	return 0;
}
/**
 * 判断链表是否为空
 * Qtype 队列类型
 * 返回值：
 * 	空  1,  非空 0
 */
static s32 Is_Empty(Que_type  Qtype)
{
	ISQUEUE(Qtype);
	return list_empty_careful( &Queue[Qtype].task_header->enter );
}

/**
 * 从队列节点中删除 task节点
 * @param  Qtype 队列类型
 * @param  task 要删除的节点
 * @return      
 * 	成功 0; 失败 -1;
 */
static s32 Delete_Task(Que_type  Qtype,  Node*task)
{
	ISQUEUE(Qtype);
	if(NULL == task ){
		return -1;
	}
	/* 释放数据域内存 */
	free(task->value);
	/*  删除链表关系 */
	list_del_entry(&task->enter);
	/* 释放节点内存 */
	free(task);
	/* 节点数减1 */
	(Queue[Qtype].task_header->value) = (void*)( (u32)(Queue[Qtype].task_header->value) -1);
	return 0;
}

/**
 * 从队列起始位置取出一个节点拷贝到task中，并且从队列中移除这个节点
 * @param  Qtype 队列类型
 * @param  task  作为输出型参数，传进来的时候，task->value 一定要指向一个可以使用的内存地址，否则会出现段错误
 * @return      
 * 	成功 0; 失败 -1;
 */
static s32 Select_Task(Que_type  Qtype,  Node*task)
{
	ISQUEUE(Qtype);
	if(NULL == task ){
		return -1;
	}
	if(!Is_Empty(Qtype)){
		Node *pdata 	= list_next_entry(Queue[Qtype].task_header,enter);
		task->datalen 	= pdata->datalen;
		task->task_type	= pdata->task_type;
		task->task_level	= pdata->task_level;
		if(NULL == task->value){
			perror("can not malloc memory for value!");
			return -1;
		}
		memcpy(task->value,pdata->value,pdata->datalen);
		/* 将任务从队列中删除 */
		Delete_Task(Qtype,pdata);
		return 0;
	}
	return 1;	//表示队列空，最后修改为宏定义
}

/**
 * 执行队列中的一个任务
 * @param  Qtype 队列类型
 * @return      
 * 	成功 0; 失败 -1;
 */
static s32 Task_ExcuteOrAppend(Que_type  Qtype)
{
	ISQUEUE(Qtype);
	vu32 ExcSuccess =1;
	PureCmdArry *prcv;
	u8 data[100] = {0};	//用于存放数据包内容
	Node task;
	task.value = data;
	if( 0 == Select_Task(Qtype,&task)){
		/* 数据准备完成，可以执行了 */
		prcv = (PureCmdArry *)task.value;
		ListCmdProcessFunc *plist = (ListCmdProcessFunc *)DetlCmdProcessFunc[Qtype];
		while(0xff != plist->ctrl) {
			if(prcv->ctrl == plist->ctrl) {
				if(NULL != plist->pf) {	
					/* 调用执行任务 */
					if( !(plist->pf)(&task) ){
						ExcSuccess = 0;
						return 0;	
					}
					break;
				} //if(NULL != plist->pf)
			} //if(prcv->ctrl == plist->ctrl)
			++plist;
		} //while(0xff != plist->ctrl)
		if(ExcSuccess){  //这里要增加一个判断重复加入次数，不能无限加回
			/* 执行失败,把节点加回队列 */
			Insert_Node(Qtype,&task);
		}
	}
	return 0;
}


s32 List_Operation(List_Interface cmd, ...)
{
	u32  	Qtype;
	Node 	*task;
	va_list 	arg_ptr;
	va_start(arg_ptr, cmd);
	switch(cmd){
		case INIT_QUE:
			va_end(arg_ptr); 
			return Init_Queue();
		case INSERT_NODE:
			Qtype 	= va_arg(arg_ptr, u32);
			task 	= va_arg(arg_ptr, Node*);
			va_end(arg_ptr); 
			return Insert_Node(Qtype,task);
		case IS_EMPTY:
			Qtype 	= va_arg(arg_ptr, u32);
			va_end(arg_ptr); 
			return Is_Empty(Qtype);
		case SELECT_TASK:
			Qtype 	= va_arg(arg_ptr, u32);
			task 	= va_arg(arg_ptr, Node*);
			va_end(arg_ptr); 
			return Select_Task(Qtype,task);
		default : printf("can not find the cmd\n");
			break;
	}
	return  -1;
}


// int main(int argc, char const *argv[])
// {
// 	List_Operation(INIT_QUE);
// 	List_Operation(IS_EMPTY,server2cc);
// 	return 0;
// }