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

static s32 UID_Check(appitf_t *this,void *r_uid)
{
	assert_param(this,NULL,FAIL);
	assert_param(r_uid,NULL,FAIL);
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

static s32 TopUser_ProcQue(appitf_t *this)
{
	assert_param(this,NULL,FAIL);
	if(SUCCESS != this->Queue->Task_Exec(this->Queue)) {
		this->msleep(5);		//执行失败睡眠5ms,否则cpu的占用率会很高
		return FAIL;
	}
	return SUCCESS;
}

static s32 TopUser_InsertQue(appitf_t *this)
{
	assert_param(this,NULL,FAIL);
	/* if the connect_status is't ok return */
	if(this->Connect_status != Connect_ok){
		this->msleep(10);
		return FAIL;
	}

	u8 recvbuf[300];
	if(SUCCESS != this->TopUserRecvPackage(this,recvbuf,300)) return FAIL;

	if(SUCCESS != this->UID_Check(this,&recvbuf[1]) ){		// 增加对集中器UID的判断
		debug(DEBUG_TaskAppend,"CC UID Unmatch!\n");
		return FAIL;
	}

	/* get Queue type and level
	 *  return_value&0xff is Queue type
	 *  (return_value>>8)&0xff is Queue level
	 *  return_value == -1,get Queue type error
	 */
	int Que_type = this->Queue->get_Quetype(this->Queue,recvbuf[8]);
	if(-1 == Que_type) return FAIL;

	if(SUCCESS != this->Queue->Task_Append(this->Queue,Que_type&0xff,(Que_type>>8) &0xff,&recvbuf[8],recvbuf[9]+2)){
		debug(1,"new task append error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int TopUser_Keepalive(appitf_t *this)
{
	assert_param(this,NULL,FAIL);

	if(this->HeartBeat_status == HeartBeat_ok){
		this->HeartBeat_status = HeartBeat_error;
		/* HeartBeat */
		if(this->ethernet && this->ethernet->ether_heartbeat && \
			SUCCESS != this->ethernet->ether_heartbeat(this->ethernet))
			return FAIL;
		this->msleep(1000*this->HeartBCycle);
	}else{
		this->Connect_status = Connect_error;
		/* 判断是否使用网络方式 */
		if(this->ItfWay == ether_net){
			/* connect to server */
			if(this->ethernet && this->ethernet->ether_connect)
				this->Connect_status = this->ethernet->ether_connect(this->ethernet) == \
				SUCCESS ? Connect_ok : Connect_error;
			/* logon and heartbeat */
			if(   this->Connect_status == Connect_ok /* connect ok */ &&\
				this->ethernet->ether_logon /* function logon is not null */ && \
				this->ethernet->ether_heartbeat /* function HeartBeat is not null */ && \
				SUCCESS == this->ethernet->ether_logon(this->ethernet) /* log on success? */&&\
				SUCCESS == this->ethernet->ether_heartbeat(this->ethernet) ){
				this->msleep(1000*this->HeartBCycle);
				return SUCCESS;
			}else{
				err_Print(1,"ethernet logon or heartbeat error!\n");
				this->Connect_status = Connect_error;
				this->msleep(3000);
				return FAIL;
			}
		}	//end of if(this->ItfWay == ether_net){
	}
	return SUCCESS;
}

static int package_check(void *package)
{
	assert_param(package,NULL,FAIL);

	int package_len = ((faalpkt_t*)package)->len + 10;
	u8 *pchar = (u8*)package;
	u8 chk = 0;

	for(int i=0;i<package_len;++i) chk += *pchar++;

	if(chk != *pchar++) return FAIL;
	if(*pchar != 0x16){
		debug(1,"package end error!\n");
		return FAIL;
	}
	return SUCCESS;
}
static int TopUser_RecvPackage(appitf_t *this,u8 *buffer,int bufsize)
{
	assert_param(this,NULL,FAIL);
	assert_param(buffer,NULL,FAIL);
	memset(buffer,0,bufsize);
	int res = -1;
	u8 *pchar = buffer;
	int len = 0;
	/* recv header */
	while(*buffer != 0x68){
		res = this->ethernet->ether_getchar(this->ethernet,pchar);
		if(res != SUCCESS) this->msleep(5);		//接收失败睡眠5ms,否则cpu的占用率会很高
	}
	pchar++;
	res = this->ethernet->ether_recv(this->ethernet,pchar,9);
	/* recv the ccuid ctr and len */
	if(res == 9){
		pchar += 9;
		len = *(pchar-1)+2;
	}else{
		debug(1,"^^^^^errno:%d\n",res);
		if( FAIL == res){
			this->Connect_status = Connect_error;
			this->HeartBeat_status = HeartBeat_error;
		}
		return FAIL;
	}
	/* check header 2 */
	if(buffer[7] != 0x68){
		debug(1,"[%02x]package header 2 error!\n",buffer[7]);
		return FAIL;
	}
	/* recv  remain data */
	res = this->ethernet->ether_recv(this->ethernet,pchar,len);
	if(res == len){
		/* check the package */
		if(SUCCESS != this->packagecheck(buffer) ){
			err_Print(1,"Recv package check error!\n");
			return FAIL;
		}
		this->Connect_status = Connect_ok;
		this->HeartBeat_status = HeartBeat_ok;
		return SUCCESS;
	}else if( FAIL == res){
		this->Connect_status = Connect_error;
		this->HeartBeat_status = HeartBeat_error;
	}
	return FAIL;
}

static void TopUser_usleep(u32 us)
{
	struct timeval tv;
	tv.tv_sec   = us/1000000;
	tv.tv_usec = us%1000000;
	select(1, NULL,NULL,NULL,&tv);
}

void TopUser_msleep(u32 ms)
{
	struct timeval tv;
	ms *= 1000;
	tv.tv_sec   = ms/1000000;
	tv.tv_usec = ms%1000000;
	select(0, NULL,NULL,NULL,&tv);
}

static int appitf_init(appitf_t *this)
{
	assert_param(this,NULL,FAIL);
	loadParam(&g_appity);
	this->Connect_status = Connect_error;
	this->HeartBeat_status = HeartBeat_error;

	/* init for Queue */
	if(this->Queue && this->Queue->Que_init && \
		SUCCESS == this->Queue->Que_init(this->Queue,this) ){
	}else{
		debug(DEBUG_app,"Queue init error! \n");
		return FAIL;
	}

	switch(this->ItfWay){
		case ether_net: 	/* init for ethernet */
			if( this->ethernet && this->ethernet->ether_init &&\
				SUCCESS == this->ethernet->ether_init(this->ethernet,this) ){
			}else{
				debug(DEBUG_app,"ethernet init error! \n");
				return FAIL;
			}break;
		case gprs:break;
		case zigbee:break;
		default:
			debug(DEBUG_app,"Unknow communication %d.\n",this->ItfWay);
			return FAIL;
	}
	/* init for serial */
	if(this->Serial && this->Serial->serial_init &&\
		SUCCESS == this->Serial->serial_init( this->Serial,(0x01<<COM_485) | (0x01<<COM_DIDO) ,9600,9600) ){
	}else{
		debug(DEBUG_app,"Serial init error! \n");
		return FAIL;
	}

	if( access("cc_corl.db",F_OK))
		system("./config/Create_Database.sh &");
	this->msleep(1000);
	return SUCCESS;
}

appitf_t g_appity = {
	.Queue = &Que,
	.ethernet = &ethernet,
	.Serial = &g_serial,

	.app_Init = appitf_init,
	.UID_Check = UID_Check,
	.TopUserProcQue = TopUser_ProcQue,
	.TopUserInsertQue = TopUser_InsertQue,
	.TopUserKeepalive = TopUser_Keepalive,
	.TopUserRecvPackage = TopUser_RecvPackage,
	.packagecheck = package_check,
	.usleep = TopUser_usleep,
	.msleep = TopUser_msleep,
};

