/******************************************************************
** 文件名:	Interface.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "Interface.h"
#include "tasklist.h"

appitf_t g_appity = {
	.Queue = &Que,
	.app_Init = appitf_init,
};

static s32 UID_Check(appitf_t *this,void *r_uid)
{
	u8 *pr_uid = r_uid;
	for(int i=0;i<6;++i){
		if( pr_uid[i]   != this->CCUID[i] ){
			debug(DEBUG_TaskAppend,"Global UID:\t%02x %02x %02x %02x %02x %02x\n",
				this->CCUID[0],this->CCUID[1],this->CCUID[2],this->CCUID[3],this->CCUID[4],this->CCUID[5]);
			debug(DEBUG_TaskAppend,"Recv UID:\t%02x %02x %02x %02x %02x %02x\n",
											pr_uid[0],pr_uid[1],pr_uid[2],pr_uid[3],pr_uid[4],pr_uid[5]);
			return FAIL;
		}
	}
	return SUCCESS;
}

static s32 TopUserProcQue(appitf_t *this)
{
	SelctExcuteOrAppend(Coordinate_Queue,3,Coordinate_Task,Task_Level_Coor,this->ItfWay);
	SelctExcuteOrAppend(GPRS_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_GPRS,this->ItfWay);
	SelctExcuteOrAppend(CC_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_CC,this->ItfWay);
	SelctExcuteOrAppend(DEVICE485_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_485,this->ItfWay);
	SelctExcuteOrAppend(ETH_NET_TYPE_QUEUE,3,NET_TASK,TASK_LEVEL_NET,this->ItfWay);
	return this->Queue->Task_Exec(this->Queue);
}
static s32 TopUserInsertQue(appitf_t *this)
{
	u8 recvbuf[300];
	memset(recvbuf,0,sizeof(recvbuf));
	if( SUCCESS != faal_rcvpkt(2, recvbuf) ) return FAIL;

	if(SUCCESS != this->UID_Check(this,&recvbuf[1]) ){		// 增加对集中器UID的判断
			debug(DEBUG_TaskAppend,"CC UID Unmatch!\n");
			return FAIL;
	}
	int Que_type = this->Queue->get_Quetype(this->Queue,recvbuf);
	//debug(1,"Queue type = %d,level=%d\n",Que_type&0xff,(Que_type>>8) &0xff);
	if(-1 == Que_type) return FAIL;
	if(SUCCESS != this->Queue->Task_Append(this->Queue,Que_type&0xff,(Que_type>>8) &0xff,&recvbuf[8],recvbuf[9]+2)){
		debug(1,"new task append error!\n");
	}
	if( TaskGenerateAndAppend(Que_type&0xff,&recvbuf[8],NET_TASK,(Que_type>>8) &0xff)){
		debug(DEBUG_TaskAppend,"Task append to Queue %d fail!\n",Que_type);
		return FAIL;
	}
	return SUCCESS;
}

int appitf_init(appitf_t *this)
{
	loadParam(&g_appity);
	this->Queue->Que_init(this->Queue);
	this->TopUserInsertQue = TopUserInsertQue;
	this->TopUserProcQue = TopUserProcQue;
	this->UID_Check = UID_Check;


	 if( access("cc_corl.db",F_OK)){
		system("./config/Create_Database.sh &");
		sleep(1);
	}
	InitTaskQueue();	//初始化队列
	UartForCoordi();	//串口的配置
	InitTimeTASK();		//定时任务的初始化
	pthread_mutex_init(&mutex_ether,NULL);
	pthread_mutex_init(&mutex_task,NULL);

	return 0;
}


