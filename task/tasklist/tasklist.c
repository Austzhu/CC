/********************************************************************
	> File Name:	tasklist.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月30日 星期一 14时15分17秒
 *******************************************************************/
#include "tasklist.h"
#include "communication.h"
#include "link_method.h"
#include "cmd_process.h"
#include "loadfile.h"
#include "log.h"
#include "Timetask.h"

#define CMDLCSTAIL 	4
extern volatile u8 	TOTAL_TASK_IN_EACH_QUEUE[QUEUE_TOTAL_QUANTITY];
extern volatile s32 	TOPGLOBALMEMALLOCTIMES;
extern volatile s32 	TOPGLOBAL_HEAP_SIZE;
extern volatile u8 	TOPDeviceOprateRetryTMS[5];
extern volatile u8 	TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];
extern volatile u8 	TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];
extern volatile u8 	TOP_LOCALREALISTIC_DIINFOR_STAT[DIDO_DEVICE_QUANTITY];
extern volatile s8 	TOPDIDO_STATUS[DIDO_DEVICE_QUANTITY];
extern volatile s8 	TOPMETER_STATUS[METER_DEVICE_QUANTITY];

struct task_queue  taskQueue[QUEUE_TOTAL_QUANTITY];
const ListCmdProcessFunc DetlCmdProcessFunc[][64] = {
	//0 ---协调器485有关的控制命令
	{	{0x42, CallBack_Open},
		{0x43, CallBack_Close},
		{0x45, CallBack_Demand},
		{0x47, CallBack_Light},
		{0x48, CallBack_RtData},
		{ALL_SUPPT_CMD_END, NULL},
	},	//1 ---GPRS与GPRS有关的控制命令
	{	{GPRS_TEST1_CMD, CallBackGPRSTest1},
		{ALL_SUPPT_CMD_END, NULL},
	},	//2 ---CC与CC相关的控制指令
	{	{CC_SERVER_ACK_CMD,CallBackServerFeedback},			//0X80,服务器回复
		//{CC_LOGON_CMD,CallBackLogon},					//0xA1,
		{CC_OPRATE_CMD,CallBackReset},					//0XA2,上位机控制集中器
		{CC_PARA_SET_CMD,CallBackCCGlobalParaSetGet},			//0XA3,集中器参数设置
		{CC_UPDATE_CMD,CallBackApplicUpDate},				//0XA4,集中器远程升级
		{CC_TIME_CTRL_CMD,CallBackTimerControlTaskRLT},			//0XC1,定时器控制协议
 		{ALL_SUPPT_CMD_END, NULL},						//0XFF结束
	},	//3 ---485与外接485设备相关的控制指令
	{	{CC_METER_READ_CMD,CallBackMetterInfoColletRD},			//0xD1,读取电表协议
		{CC_METER_SET_CMD,CallBackMetterInfoColletSet},			//0xD2,设置电表参数
		{CC_METER_RD_EXTR_CMD,CallBackMetterInfoColletExtrRD},		//0xD3,读外购电表
		{CC_METER_SET_EXTR_CMD,CallBackMetterInfoColletExtrSet},		//0xD4,ack OK 20120611 date
		{0x01,CallBackMetterInfoBroadcast},					//0x01 test by Austzhu 2016.3.29
		{0x02,CallBackMetterInfoBroadcast},
		{0x03,CallBackMetterInfoBroadcast},
		{CC_DIDO_SET_CMD,CallBackMetterDIDO},				//0XE1,	DIDO扩展继电器协议
 		{ALL_SUPPT_CMD_END, NULL},
	},	//4 ---ETH与硬件网口相关的应答
	{	{CC_ETH_ShortACK_CMD,CallBackETHShotAck},			//0x51,
		{CC_ETH_LongContex_CMD,CallBackETHLongContexBack},		//0x52,
 		{ALL_SUPPT_CMD_END, NULL},
	}
};

s32 isEmpty(struct task_queue *queue)
{
	return  list_empty(&(queue->header->entries));
}
s32 init_task_queue(struct task_queue *taskqueue)
{
	assert_param(taskqueue,"taskqueque is null!",FAIL);
	task_node *head = NULL;
	if( (head = (task_node*)malloc(sizeof(task_node)) ) == NULL){
		err_Print(DEBUG_list,"Creat head node err!\n");
		return FAIL;
	}
	taskqueue->header = head;
	taskqueue->i = 0;
	INIT_LIST_HEAD(&(head->entries));

	/**
	 *  第一个节点不存储数据
	 */
	head->task_type = 0xff;
	head->level	 = 0xff;
	head->t 	 = 0;
	return SUCCESS;
}

void InitTaskQueue(void)
{
	int ii = 0;
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

	for(ii=0;  ii <QUEUE_TOTAL_QUANTITY;  ++ii){
		if(SUCCESS != init_task_queue(&taskQueue[ii])){
			err_Print(DEBUG_list,"Init taskQue err:taskQueue[%d]\n",ii);
		}
	}
}

struct task_node * mallocNode(u8 task_type,  u8 level,  u8 *data, u32 datalen,  time_t t)
{
	task_node *ptr = (task_node *)malloc(sizeof(task_node));
	if(!ptr){
		err_Print(DEBUG_list,"Malloc Node err!\n");
		return NULL;
	}
	INIT_LIST_HEAD(&(ptr->entries));

	TOPGLOBALMEMALLOCTIMES++;
	TOPGLOBAL_HEAP_SIZE += sizeof(task_node);
	ptr->task_type = task_type;
	ptr->level = level;
	memcpy(ptr->pakect,data,datalen);
	ptr->t = t;
	return ptr;
}

s32 appendTask(struct task_queue *queue, struct task_node *node,  u8 queue_type)
{
	if(NULL == node) {
		err_Print(DEBUG_list,"Node is NULL!\n");
		return FAIL;
	}
	++(queue->i);
	queue->queue_type = queue_type;
	list_add_tail(&(node->entries), &(queue->header->entries));
	return SUCCESS;
}

s32 SelectTask(struct task_queue *queue,  u32 mode, struct task_node *recv,  u8 queuetype)
{

	if(!queue){
		err_Print(DEBUG_list,"Queue is NULL!\n");
		return FAIL;
	}


	switch(mode){
		case 1: break;
		case 2: break;
		case 3: return SelectNode(queue,recv,queuetype);
		default:break;
	}
	return FAIL;
}

s32 DelTask(struct task_queue *queue,  struct task_node *node,  u8 queuetype)
{
	if(!node) {
		err_Print(DEBUG_list,"Node is NULL!\n");
		return TASK_QUEUE_NO_NODE;
	}
	list_del_entry(&(node->entries));
	queue->i--;
	TOTAL_TASK_IN_EACH_QUEUE[queuetype] = queue->i;
	free(node);
	TOPGLOBAL_HEAP_SIZE -= sizeof(struct task_node);
	return SUCCESS;
}

void print_tasknode(task_node *node)
{
	int i=0;
	printf("**********show node start!\n");
	while(i < node->pakect[1]+4){
		debug(DEBUG_list,"%02x ",node->pakect[i++]);
	}printf("\n");
	printf("**********show node end!\n");
}

s32 SelectNode(struct task_queue *queue,struct task_node *recv,u8 queuetype)
{

	if ((queuetype > QUEUE_TOTAL_QUANTITY) || (queuetype < 0)){
		err_Print(DEBUG_list,"Con't find the Queue type:%d.\n",queuetype);
		return TASK_QUEUE_NOSUCH_QUEUE_ERR;
	}
	if(isEmpty(&taskQueue[queuetype])){
		return TASK_QUEUE_EMPTY;
	}

	TOTAL_TASK_IN_EACH_QUEUE[queuetype] = queue->i;
	task_node *ptr = list_first_entry(&(queue->header->entries),  task_node, entries);
	//print_tasknode(ptr);
	memcpy(recv, ptr ,sizeof(task_node));
	//print_tasknode(recv);
	return DelTask(queue, ptr ,queuetype);
}


s32 TaskGenerateAndAppend(u8 queuetype, u8 *data,u8 tasksource,u8 tasklevel)
{
	s32 i = 0;
	struct task_node *node = NULL;
	#if DebugPrint
		err_Print(1,"NO.2.5\n");
	#endif
	debug(DEBUG_LOCAL_TASK,"\n----------QUEUE type is %d----------\n",queuetype);
	/* 申请节点，如果失败，重复3次 */
	while( !node && i++<3){
		node = mallocNode((UCHAR)tasksource,tasklevel,data, (UINT)(CMDLCSTAIL+((PureCmdArry *)data)->len), time((time_t*)NULL));
	}
	if(!node){
		err_Print(DEBUG_list,"MallocNode error!\n");
		return FAIL;
	}
	#if DebugPrint
		err_Print(1,"NO.2.6\n");
	#endif
	//pthread_mutex_lock(&mutex_task);	//获取task锁
	if(SUCCESS != appendTask(&taskQueue[queuetype],node,queuetype)){
		//pthread_mutex_unlock(&mutex_task);	//解task锁
		err_Print(DEBUG_list,"AppendTask err!\n");
		return FAIL;
	}
	//pthread_mutex_unlock(&mutex_task);	//解task锁
	#if DebugPrint
		err_Print(1,"NO.2.7\n");
	#endif
	return SUCCESS;
}

s32 CheckCC_UID(faalpkt_t *pkt)
{
	int i = 0;
	u8 *Puid = NULL;
	while(i < 6){
		if( pkt->rtua[i]   != CCparamGlobalInfor.CCUID[i] ){
			Puid = CCparamGlobalInfor.CCUID;
			debug(DEBUG_TaskAppend,"Global UID:\t%02x %02x %02x %02x %02x %02x\n",Puid[0],Puid[1],Puid[2],Puid[3],Puid[4],Puid[5]);
			Puid = pkt->rtua;
			debug(DEBUG_TaskAppend,"Recv UID:\t%02x %02x %02x %02x %02x %02x\n",Puid[0],Puid[1],Puid[2],Puid[3],Puid[4],Puid[5]);
			return FAIL;
		}
		++i;
	}
	return SUCCESS;
}

s32 RecInsertQueue(u8 itf)
{
	#if DebugPrint
		err_Print(1,"NO.2\n");
	#endif
	UCHAR recvbuf[300];
	memset(recvbuf,0,sizeof(recvbuf));
	/* 网络协议解析（协议命令解码）nick 20160321 */
	if( SUCCESS == faal_rcvpkt(ITF_WAY_ETHER, recvbuf) ) {
		#if DebugPrint
			err_Print(1,"NO.2.0.1\n");
		#endif
		 #ifdef Config_Log
			Write_log(CC, recvbuf );
		 #endif
		#if DebugPrint
			err_Print(1,"NO.2.0.3\n");
		#endif

		/* 增加对集中器UID的判断 */
		if(SUCCESS != CheckCC_UID((faalpkt_t *)recvbuf) ){
			debug(DEBUG_TaskAppend,"CC UID Unmatch!\n");
			return FAIL;
		}
		if(!WhetherCMDExist(GPRS_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
			#if DebugPrint
				err_Print(1,"NO.2.1\n");
			#endif
			if( TaskGenerateAndAppend(GPRS_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_GPRS) )
				debug(DEBUG_TaskAppend,"Task append to GPRS_TYPE_QUEUE fail!\n");

		}else if(!WhetherCMDExist(CC_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
			#if DebugPrint
				err_Print(1,"NO.2.2\n");
			#endif
			if( TaskGenerateAndAppend(CC_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_CC) )
				debug(DEBUG_TaskAppend,"Task append to CC_TYPE_QUEUE fail!\n");

		}else if(!WhetherCMDExist(DEVICE485_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
		#if DebugPrint
			err_Print(1,"NO.2.3\n");
		#endif
			if( TaskGenerateAndAppend(DEVICE485_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_485) )
				debug(DEBUG_TaskAppend,"Task append to DEVICE485_TYPE_QUEUE fail!\n");

		}else if(!WhetherCMDExist(ETH_NET_TYPE_QUEUE,((faalpkt_t *)recvbuf)->ctrl)){
		#if DebugPrint
			err_Print(1,"NO.2.4\n");
		#endif
			if( TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,&recvbuf[8],NET_TASK,TASK_LEVEL_NET) )
				debug(DEBUG_TaskAppend,"Task append to ETH_NET_TYPE_QUEUE fail!\n");
		}
	 }
	return SUCCESS;
}



s32 SelctExcuteOrAppend(u8 queuetype,  u32 mode,  u8 tasktype,  u8 tasklevel,  u8 itf)
{

	volatile u8 ExcullSucces = 1;
	PureCmdArry *prcv;
	struct task_node node;
	u32 res = 1,i = 0;
	struct task_node *NodeAppend = NULL;
	//int WaitAlarm = 0;
	if(queuetype<0 || queuetype>5){
		err_Print(DEBUG_list, "Queue Type error!\n");
		return FAIL;
	}
	#if DebugPrint
		err_Print(1,"NO.3.1\n");
	#endif
	//pthread_mutex_lock(&mutex_task);	//获取task锁
	res=SelectTask(&taskQueue[queuetype],mode,&node,queuetype);
	//pthread_mutex_unlock(&mutex_task);	//解task锁
	if( res == TASK_QUEUE_EMPTY){ return SUCCESS;}

	#if DebugPrint
		err_Print(1,"NO.3.2\n");
	#endif
	if(!res)  {
		#if DebugPrint
			err_Print(1,"NO.3.3\n");
		#endif
		prcv = (PureCmdArry *)(node.pakect);
		ListCmdProcessFunc *plist = (ListCmdProcessFunc *)DetlCmdProcessFunc[queuetype];
		while(0xff != plist->ctrl) {
			if(prcv->ctrl == plist->ctrl) {
				debug(DEBUG_LOCAL_TASK,"%s#####Get Ctrl:0x%02x.\n",__func__,plist->ctrl);
				if(NULL != plist->pf) {
					debug(DEBUG_LOCAL_TASK,"%s#####Do Function is not null!\n",__func__);
					/* 调用执行任务 */
					TaskBusy = !0;	//任务忙标志，定时任务到来时会被挂起
					if( !(plist->pf)(prcv->ctrl,itf,&node) ){
						debug(DEBUG_LOCAL_TASK,"%s#####Exec Task Success!\n",__func__);
						ExcullSucces = 0;
						if (TOTAL_TASK_IN_EACH_QUEUE[queuetype] < 0){
							TOTAL_TASK_IN_EACH_QUEUE[queuetype] = 0;
						}
						TOPDeviceOprateRetryTMS[queuetype] = 0;
						TaskBusy = 0;	//任务执行完，复位任务忙标志
						/* 给自己发送一个自定义信号，查看再执行的时候有没有定时任务到了 */
						kill(getpid(),SIGUSR1);
						return SUCCESS;
					}
					TaskBusy = 0;
					kill(getpid(),SIGUSR1);
					break;
				} //if(NULL != plist->pf)
			} //if(prcv->ctrl == plist->ctrl)
			++plist;
		} //while(0xff != plist->ctrl)
		#if DebugPrint
			err_Print(1,"NO.3.4\n");
		#endif
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

s32 TopUserQueProc(u8 itf)
{
	#if DebugPrint
		err_Print(1,"NO.3\n");
	#endif
	SelctExcuteOrAppend(Coordinate_Queue,3,Coordinate_Task,Task_Level_Coor,itf);
	SelctExcuteOrAppend(GPRS_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_GPRS,itf);
	SelctExcuteOrAppend(CC_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_CC,itf);
	SelctExcuteOrAppend(DEVICE485_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_485,itf);
	SelctExcuteOrAppend(ETH_NET_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_NET,itf);

	return SUCCESS;
}

