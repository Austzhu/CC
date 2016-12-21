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
#include "single.h"
#include "loadfile.h"

static void *KeepaliveThread(void *args)
{
	if(!args){
		printf("create KeepaliveThread error!\n");
		pthread_exit(NULL);
	}
	appitf_t *this = (appitf_t*)args;
	while(this->pthread_start)
		this->TopUser_Keepalive(this);
	#ifdef  CFG_exitMessage
		printf("Top KeepaliveThread exit!\n");
	#endif
	pthread_exit(NULL);
}

static void *RecvInsertQueueThread(void *args)
{
	if(!args){
		printf("create RecvInsertQueueThread error!\n");
		pthread_exit(NULL);
	}
	appitf_t *this = (appitf_t*)args;
	while(this->pthread_start)
		this->TopUser_InsertQue(this);
	#ifdef  CFG_exitMessage
		printf("Top RecvInsertQueueThread exit!\n");
	#endif
	pthread_exit(NULL);
}

static void *UserQueProcThread(void *args)
{
	if(!args){
		printf("create UserQueProcThread error!\n");
		pthread_exit(NULL);
	}
	appitf_t *this = (appitf_t*)args;
	while(this->pthread_start)
		this->TopUser_ProcQue(this);
	#ifdef  CFG_exitMessage
		printf("Top UserQueProcThread exit!\n");
	#endif
	pthread_exit(NULL);
}


static s32 inline UID_Check(appitf_t *this,void *r_uid)
{
	if(!this || !r_uid) return FAIL;
	return memcmp(r_uid,this->param.CCUID,6)?FAIL:SUCCESS;
}

static s32 TopUser_ProcQue(appitf_t *this)
{
	assert_param(this,FAIL);

	if(SUCCESS != this->Queue->Task_Exec(this->Queue)) {
		msleep(5);     //执行失败睡眠5ms,否则cpu的占用率会很高
		return FAIL;
	}
	return SUCCESS;
}

static s32 TopUser_InsertQue(appitf_t *this)
{
	assert_param(this,FAIL);
	u8 recvbuf[300] = {0};

	/* if the connect_status is't ok return */
	if(this->Connect_status != Connect_ok){
		msleep(10);
		return FAIL;
	}

	if(SUCCESS != this->TopUser_RecvPackage(this,recvbuf,300)) return FAIL;

	if(SUCCESS != this->TopUser_Uidchk(this,&recvbuf[1]) ){    // 增加对集中器UID的判断
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

	if(SUCCESS != this->Queue->Task_Append(this->Queue,\
		Que_type&0xff,(Que_type>>8) &0xff,&recvbuf[8],recvbuf[9]+2)){
		debug(1,"new task append error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int TopUser_Keepalive(appitf_t *this)
{
	assert_param(this,FAIL);

	if(this->HeartBeat_status == HeartBeat_ok){
		this->HeartBeat_status = HeartBeat_error;
		/* HeartBeat */
		if(this->opt_Itf && this->opt_Itf->opt_heartbeat &&\
			SUCCESS != this->opt_Itf->opt_heartbeat(this->opt_Itf) )
			return FAIL;

		for(int ii=0;ii<1000;++ii){
			msleep(this->param.HeartBCycle);
			if(!this->pthread_start) return FAIL;
		}
	}else{
		/* connect to server */
		this->Connect_status = \
			this->opt_Itf->opt_connect(this->opt_Itf) == SUCCESS ? Connect_ok : Connect_error;
		/* logon and heartbeat */
		if(   this->Connect_status == Connect_ok	/* connect ok */ &&\
			SUCCESS == this->opt_Itf->opt_logon(this->opt_Itf)	/* log on success? */&&\
			SUCCESS == this->opt_Itf->opt_heartbeat(this->opt_Itf) ){		/* heartbeat success? */
			for(int ii=0;ii<1000;++ii){
				msleep(this->param.HeartBCycle);
				if(!this->pthread_start) return FAIL;
			}
			return SUCCESS;
		}else{
			err_Print(1,"ethernet logon or heartbeat error!\n");
			this->Connect_status = Connect_error;
			this->HeartBeat_status = HeartBeat_error;
			return FAIL;
		}
	}
	return SUCCESS;
}

static int package_check(void *package)
{
	assert_param(package,FAIL);
	u8 chk = 0;
	int package_len = ((faalpkt_t*)package)->len + 10;
	u8 *pchar = (u8*)package;

	for(int i=0;i<package_len;++i)
		chk += *pchar++;

	if(chk != *pchar++) return FAIL;
	if(*pchar != 0x16){
		debug(1,"package end error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int TopUser_RecvPackage(appitf_t *this,u8 *buffer,int bufsize)
{
	assert_fail(this);
	assert_fail(buffer);

	bzero(buffer,bufsize);
	int res = -1, len = 0;
	u8 *pchar = buffer;
	while(*buffer != 0x68){    /* recv header */
		res = this->opt_Itf->opt_getchar(this->opt_Itf,pchar);
		if(res != SUCCESS) msleep(5);    //接收失败睡眠5ms,否则cpu的占用率会很高
		if(!this->pthread_start) return FAIL;
	}	pchar++;
	res = this->opt_Itf->opt_recv(this->opt_Itf,pchar,9);
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
	res = this->opt_Itf->opt_recv(this->opt_Itf,pchar,len);
	if(res == len){
		/* check the package */
		if(SUCCESS != this->TopUser_pakgchk(buffer) ){
			err_Print(1,"Recv package check error!\n");
			return FAIL;
		}
		/* 收到完整数据包，则标志心跳为ok */
		this->Connect_status = Connect_ok;
		this->HeartBeat_status = HeartBeat_ok;
		return SUCCESS;
	}else if( FAIL == res){
		this->Connect_status = Connect_error;
		this->HeartBeat_status = HeartBeat_error;
	}
	return FAIL;
}


static int TopUser_Relese(appitf_t *this)
{
	assert_param(this,FAIL);
	printf("  app exit!\n");

	this->pthread_start = 0;
	if(this->thread_Keepalive > 0 ){
		pthread_join(this->thread_Keepalive,NULL);
		printf("thread_Keepalive exit!\n");
	}
	if(this->thread_RecvInsert > 0 ){
		pthread_join(this->thread_RecvInsert,NULL);
		printf("thread_RecvInsert exit!\n");
	}
	if(this->thread_UserProc > 0 ){
		pthread_join(this->thread_UserProc,NULL);
		printf("thread_UserProc exit!\n");
	}

	DELETE(this->Queue,Que_release);
	DELETE(this->opt_Itf,opt_relese);
	DELETE(this->warn,warn_relese);
	DELETE(this->single,sin_release);
	#ifdef Config_Sqlite
	DELETE(this->sqlite,sql_release);
	#endif
	#ifdef Config_serial
	DELETE(this->Serial,serial_relese);
	#endif
	#ifdef Config_TCP_Server
	DELETE(this->tcp_server,ser_release);
	#endif
	#ifdef Config_Meter
	DELETE(this->meter,meter_release);
	#endif
	_exit(0);
}

static void app_exit(int signal)
{
	TopUser_Relese(&g_appity);
}

static int appitf_init(appitf_t *this)
{
	assert_param(this,FAIL);

	loadParam(&g_appity);
	this->Connect_status 		= Connect_error;
	this->HeartBeat_status 	= HeartBeat_error;
	this->thread_Keepalive 	= -1;
	this->thread_RecvInsert 	= -1;
	this->thread_UserProc 	= -1;
	this->pthread_start 		=   1;

	INIT_FAIL(this->Queue,Queue_Init,this);			/* init for Queue */
	INIT_FAIL(this->opt_Itf,operate_init,&(this->param));
	#ifdef Config_serial								/* init for serial */
	INIT_FAIL(this->Serial,serial_Init,((0x01<<CFG_COM485) | (0x01<<CFG_COMDIDO)),9600,9600);
	#endif
	#ifdef Config_Sqlite
	INIT_FAIL(this->sqlite,sql_Init);						/* init for sqlite */
	#endif
	INIT_FAIL(this->single,single_Init,this);				/* init for single */
	#ifdef Config_Meter
	INIT_FAIL(this->meter,meter_init,this);				/* init for dido */
	#endif
	INIT_FAIL(this->warn,warn_init,this);				/* init for warn */
	#ifdef  Config_TCP_Server
		INIT_FAIL(this->tcp_server,ser_init);			/* init for tcp server */
		this->tcp_server->ser_start(g_appity.tcp_server,8889);
	#endif
	if( access("cc_corl.db",F_OK))
		system("./config/Create_Database.sh &");

	/*  create thread */
	pthread_create(&this->thread_Keepalive,NULL,KeepaliveThread,this);
	pthread_create(&this->thread_RecvInsert,NULL,RecvInsertQueueThread,this);
	pthread_create(&this->thread_UserProc,NULL,UserQueProcThread,this);
	if(this->thread_Keepalive < 0 || this->thread_RecvInsert < 0 || this->thread_UserProc < 0){
		printf("create pthread error!\n");
		return FAIL;
	}
	/* catch signal "ctrl+c" to exit application */
	signal(SIGINT,app_exit);
	return SUCCESS;
}

appitf_t g_appity = {
	.TopUser_Init = appitf_init,
	.TopUser_relese = TopUser_Relese,
	.TopUser_Uidchk = UID_Check,
	.TopUser_ProcQue = TopUser_ProcQue,
	.TopUser_InsertQue = TopUser_InsertQue,
	.TopUser_Keepalive = TopUser_Keepalive,
	.TopUser_RecvPackage = TopUser_RecvPackage,
	.TopUser_pakgchk = package_check,
};
