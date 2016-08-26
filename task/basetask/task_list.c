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
#include "task_list.h"
#include "task.h"
#include "communication.h"
#include "link_method.h"
#include "cmd_process.h"
//#include "softwareinit.h"
#include "loadfile.h"
#include "log.h"
#define CMDLCSTAIL 4
extern volatile UCHAR 	TOTAL_TASK_IN_EACH_QUEUE[QUEUE_TOTAL_QUANTITY];
extern volatile SINT 	TOPGLOBALMEMALLOCTIMES;
extern volatile SINT 	TOPGLOBAL_HEAP_SIZE;
extern volatile UCHAR 	TOPDeviceOprateRetryTMS[5];
extern volatile UCHAR 	TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];	
extern volatile UCHAR 	TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];			
extern volatile UCHAR 	TOP_LOCALREALISTIC_DIINFOR_STAT[DIDO_DEVICE_QUANTITY];	
extern volatile CHAR 	TOPDIDO_STATUS[DIDO_DEVICE_QUANTITY];
extern volatile CHAR 	TOPMETER_STATUS[METER_DEVICE_QUANTITY];

struct task_queue 	taskQueue[QUEUE_TOTAL_QUANTITY];		//QUEUE_TOTAL_QUANTITY


const ListCmdProcessFunc DetlCmdProcessFunc[][200] = {
	//0 ---协调器485有关的控制命令
	{
		{0x42, CallBack_Open},		//0x42,
		{0x43, CallBack_Close},		
		{0x45, CallBack_Light},
		{0x47, CallBack_Demand},
		{0x48, CallBack_RtData},	
		{ALL_SUPPT_CMD_END, 		NULL},	
	},
	//1 ---GPRS与GPRS有关的控制命令
	{
		{GPRS_TEST1_CMD, CallBackGPRSTest1},			//0x31,
			
		{ALL_SUPPT_CMD_END, 		NULL},
	},
	//2 ---CC与CC相关的控制指令
	{
		{CC_SERVER_ACK_CMD,CallBackServerFeedback},		//0X80,服务器回复
		//{CC_LOGON_CMD,CallBackLogon},				//0xA1,
		{CC_OPRATE_CMD,CallBackReset},				//0XA2,上位机控制集中器
		{CC_PARA_SET_CMD,CallBackCCGlobalParaSetGet},		//0XA3,集中器参数设置
		{CC_UPDATE_CMD,CallBackApplicUpDate},			//0XA4,集中器远程升级
		{CC_TIME_CTRL_CMD,CallBackTimerControlTaskRLT},		//0XC1,定时器控制协议
		
 		{ALL_SUPPT_CMD_END, NULL},					//0XFF结束
	},
	//3 ---485与外接485设备相关的控制指令
	{
		{CC_METER_READ_CMD,CallBackMetterInfoColletRD},		//0xD1,读取电表协议
		{CC_METER_SET_CMD,CallBackMetterInfoColletSet},		//0xD2,设置电表参数
		{CC_METER_RD_EXTR_CMD,CallBackMetterInfoColletExtrRD},	//0xD3,读外购电表
		{CC_METER_SET_EXTR_CMD,CallBackMetterInfoColletExtrSet},	//0xD4,？？？ack OK 20120611 date

		{0x01,CallBackMetterInfoBroadcast},				//0x01 test by Austzhu 2016.3.29
		{0x02,CallBackMetterInfoBroadcast},
		{0x03,CallBackMetterInfoBroadcast},
		{CC_DIDO_SET_CMD,CallBackMetterDIDO},			//0XE1,	DIDO扩展继电器协议		
		
 		{ALL_SUPPT_CMD_END, NULL},
	},
	//4 ---ETH与硬件网口相关的应答
	{
		{CC_ETH_ShortACK_CMD,CallBackETHShotAck},			//0x51,
		{CC_ETH_LongContex_CMD,CallBackETHLongContexBack},	//0x52,
		
 		{ALL_SUPPT_CMD_END, NULL},
	}
};

	

/*
判断队列是否为空
*/
UINT isEmpty(struct task_queue *queue)
{
	if(queue->header.lh_first)
		return 0;

	return 1;
}

/*
从队列中删除一个任务节点
*/
UINT DelTask(struct task_queue *queue,struct task_node *node,UCHAR queuetype)
{
	if(!node) return TASK_QUEUE_NO_NODE;

	LIST_REMOVE(node, entries);
	queue->i--;
	
	TOTAL_TASK_IN_EACH_QUEUE[queuetype] = queue->i;
	
	free(node);
	TOPGLOBAL_HEAP_SIZE -= sizeof(struct task_node);
	
	debug(DEBUG_LOCAL_HEAPMEM_INCREASE,"HEAPMEM_INCREASE in function DelTask and reduse size is %d\n",sizeof(struct task_node));

	return SUCCESS;
}

UINT SelectNode(struct task_queue *queue,struct task_node *recv,UCHAR queuetype)
{
	if ((queuetype > QUEUE_TOTAL_QUANTITY) || (queuetype < 0)){
		printf("Task QueueType err ! \terrno : %d\n",TASK_QUEUE_NOSUCH_QUEUE_ERR);
		return TASK_QUEUE_NOSUCH_QUEUE_ERR;
	}
	
	if(!isEmpty(queue)){

		TOTAL_TASK_IN_EACH_QUEUE[queuetype] = queue->i;
		memcpy(recv,queue->header.lh_first,sizeof(struct task_node));
		DelTask(queue, queue->header.lh_first,queuetype);
		return SUCCESS;
	}	
	//debug(DEBUG_LOCAL_TASK,"Task Queue is Empty!\n");
	return TASK_QUEUE_EMPTY;
}

//SelectTask(&taskQueue[queuetype],mode,&node,queuetype);
UINT SelectTask(struct task_queue *queue,UINT mode, struct task_node *recv,UCHAR queuetype)
{
	if(queue){
		switch(mode){
			case 1:
				// return selectHighLevel(queue,recv);
				break;

			case 2:
				// return selectLowLevel(queue,recv);
				break;

			case 3:
				return SelectNode(queue,recv,queuetype);
				break;

			default:
				break;
		}
	}

	return 1;
}

UINT init_task_queue(struct task_queue *taskqueue)
{
	if(taskqueue){
		LIST_INIT(&taskqueue->header);
		taskqueue->i = 0;
		return 0;
	}

	return TASK_QUEUE_INIT_ERR;
}

UINT appendTask(struct task_queue *queue,struct task_node *node,char queue_type)
{
	if(NULL == node) return FAIL;
	++queue->i;
	queue->queue_type = queue_type;
	LIST_INSERT_HEAD(&queue->header, node, entries);
	return SUCCESS;
}


struct task_node * mallocNode(UCHAR task_type,UCHAR level,UCHAR *data, UINT datalen,time_t t)
{
	debug(DEBUG_LOCAL_TASK,"in function:[%s] level input is %d\n",__func__,level);
	struct task_node *ptr = (struct task_node *)malloc(sizeof(struct task_node));
	if(!ptr){
		return NULL;
	}
	
	TOPGLOBALMEMALLOCTIMES++;
	TOPGLOBAL_HEAP_SIZE += sizeof(struct task_node);
	ptr->task_type = task_type;
	ptr->level = level;
	memcpy(ptr->pakect,data,datalen);
	ptr->t = t;
	return ptr;
}

UINT TopUserQueProc(UCHAR itf)
{	printf("********4\n");
	SelctExcuteOrAppend(GPRS_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_GPRS,itf);
	SelctExcuteOrAppend(CC_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_CC,itf);
	SelctExcuteOrAppend(DEVICE485_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_485,itf);
	SelctExcuteOrAppend(ETH_NET_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_NET,itf);
	SelctExcuteOrAppend(Coordinate_Queue,3,Coordinate_Task,Task_Level_Coor,itf);

	return SUCCESS;
}

UINT RecInsertQueue(UCHAR itf)
{
	UCHAR recvbuf[300];
	/* 网络协议解析（协议命令解码）nick 20160321 */
	if( SUCCESS == faal_rcvpkt(ITF_WAY_ETHER, recvbuf) ) { 
	 #ifdef Config_Log
		Write_log(CC, recvbuf );
	 #endif
		if(!WhetherCMDExist(GPRS_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
			if( TaskGenerateAndAppend(GPRS_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_GPRS) ){
				debug(DEBUG_TaskAppend,"Task append to GPRS_TYPE_QUEUE fail!\n");
			}

		}else if(!WhetherCMDExist(CC_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
			if( TaskGenerateAndAppend(CC_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_CC) ){
				debug(DEBUG_TaskAppend,"Task append to CC_TYPE_QUEUE fail!\n");
			}
			printf("********2\n");

		}else if(!WhetherCMDExist(DEVICE485_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){		
			if( TaskGenerateAndAppend(DEVICE485_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_485) ){
				debug(DEBUG_TaskAppend,"Task append to DEVICE485_TYPE_QUEUE fail!\n");
			}

		}else if(!WhetherCMDExist(ETH_NET_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
			if( TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_NET) ){
				debug(DEBUG_TaskAppend,"Task append to ETH_NET_TYPE_QUEUE fail!\n");
			}
		}	
	 }
	 printf("********3\n");	
	return SUCCESS;
}

UINT InitTaskQueue(void)
{
	UCHAR ii;
	/***SOFT PARA INIT***/
	//GLOBAL SHOW INFOR INIT START
	TOPGLOBALMEMALLOCTIMES = 0;
	TOPGLOBAL_HEAP_SIZE = 0;


	//device retry times stat and control
	for (ii=0;ii<QUEUE_TOTAL_QUANTITY;ii++){
		TOPDeviceOprateRetryTMS[ii] = 0;
	}

	//DIDO infor init
	for (ii = 0;ii<DIDO_DEVICE_QUANTITY;ii++){
		TOP_SERVERREQUI_DIDOCIR_STAT[ii]=0;
		TOP_LOCALREALISTIC_DIDOCIR_STAT[ii]=0;
		TOP_LOCALREALISTIC_DIINFOR_STAT[ii]=0;
		TOPDIDO_STATUS[ii] = FAIL;
	}
	//METER INFOR INIT
	for (ii=0;ii<METER_DEVICE_QUANTITY;ii++){
		TOPMETER_STATUS[ii] = FAIL;
	}

	//for timer task
	for(ii=0;ii<sizeof(taskQueue)/sizeof(struct task_queue);ii++){
		TOTAL_TASK_IN_EACH_QUEUE[ii] = 0;	//GLOBAL TOTAL TASK INIT 
		init_task_queue((struct task_queue *)&taskQueue[ii]);
	}

	return SUCCESS;
}
//SelctExcuteOrAppend(	 PLC_TYPE_QUEUE,	   3,		  NET_TASK,	  TASK_LEVEL_PLC,  itf);
UINT SelctExcuteOrAppend(UCHAR queuetype,UINT mode,UCHAR tasktype,UCHAR tasklevel,UCHAR itf)
{
	volatile UCHAR ExcullSucces = 1;
	PureCmdArry *prcv;
	struct task_node node;
	u32 res = 1,i = 0;
	struct task_node *NodeAppend = NULL;
 printf("********5\n");
	if(queuetype<0 || queuetype>5){
		perror("queuetype error");
		return FAIL;
	}
	
	res=SelectTask(&taskQueue[queuetype],mode,&node,queuetype);

	debug(DEBUG_LOCAL_TASK_EXCUT,"SelectTask Queue is %d.\n",queuetype);
	
	if(TASK_QUEUE_EMPTY == res){
		return SUCCESS;
	}

	if(!res)  { 
		prcv = (PureCmdArry *)(node.pakect);
		ListCmdProcessFunc *plist = (ListCmdProcessFunc *)DetlCmdProcessFunc[queuetype];
		while(0xff != plist->ctrl) {
			if(prcv->ctrl == plist->ctrl) {
				debug(DEBUG_LOCAL_TASK,"%s#####Get Ctrl:0x%02x.\n",__func__,plist->ctrl);
				if(NULL != plist->pf) {	
					debug(DEBUG_LOCAL_TASK,"%s#####Do Function is not null!\n",__func__);
					/* 调用执行任务 */
					if( !(plist->pf)(prcv->ctrl,itf,&node) ){
						debug(DEBUG_LOCAL_TASK,"%s#####Exec Task Success!\n",__func__);
						ExcullSucces = 0;
						if (TOTAL_TASK_IN_EACH_QUEUE[queuetype] < 0){
							TOTAL_TASK_IN_EACH_QUEUE[queuetype] = 0;
						}
						TOPDeviceOprateRetryTMS[queuetype] = 0;
						return SUCCESS;	
					}
					break;
				} //if(NULL != plist->pf)
			} //if(prcv->ctrl == plist->ctrl)
			++plist;
		} //while(0xff != plist->ctrl)	

		/* 如果执行失败，把任务加到队列中等待下次继续执行,并且只重复3次*/
		if( ExcullSucces && (TOPDeviceOprateRetryTMS[queuetype] < 3)){
			++TOPDeviceOprateRetryTMS[queuetype];
			while(!NodeAppend && i++<3){
				NodeAppend=mallocNode((UCHAR)tasktype,tasklevel,(UCHAR *)prcv,(UINT)(CMDLCSTAIL+prcv->len),time((time_t*)NULL));
			}
			if(!NodeAppend){
				perror("NodeAppend malloc fail!");
				return FAIL;
			}
			
			if(!appendTask(&taskQueue[queuetype],NodeAppend,queuetype)){
				debug(DEBUG_LOCAL_TASK,"Task appended to queue %d again!\n",queuetype);
			}
		}		

		return SUCCESS;
	} //END OF if(!res)
	return SUCCESS;//need to confirm!!!!	
}

UINT TaskGenerateAndAppend(UCHAR queuetype, UCHAR *data,UCHAR tasksource,UCHAR tasklevel)//start
{
	s32 i = 0;
	struct task_node *node = NULL;
	debug(DEBUG_LOCAL_TASK,"\n----------QUEUE type is %d----------\n",queuetype);
	/* 申请节点，如果失败，重复3次 */
	while( !node && i++<3){
		node = mallocNode((UCHAR)tasksource,tasklevel,data, (UINT)(CMDLCSTAIL+((PureCmdArry *)data)->len), time((time_t*)NULL));
	}
	if(!node){
		perror("mallocNode fail!");
		return FAIL;
	}
	
	if(!appendTask(&taskQueue[queuetype],node,queuetype)){
		debug(DEBUG_LOCAL_TASK,"Task appended to Queue %d success!\n",queuetype);
	}

	printf("********1\n");
	
	return SUCCESS;
}
