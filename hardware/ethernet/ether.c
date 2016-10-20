/******************************************************************
** 文件名:	ether.c
** Copyright (c) 2012-2014 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/
#include "ether.h"
#include "Interface.h"
#define ether_RecvBufSize  2048
#define ether_SocketTimeout  30

static int ether_Init(ethernet_t *this,void *parent)
{
	assert_param(this,NULL,FAIL);
	assert_param(parent,NULL,FAIL);

	this->ether_sock = -1;
	this->ether_recvlen = 0,
	this->ether_recvhead = 0,
	this->ether_recvbuf = calloc(ether_RecvBufSize,sizeof(char));
	if(!this->ether_recvbuf) goto out;
	this->parent =  parent;
	pthread_mutex_init(&(this->ether_lock),NULL);
	return SUCCESS;
 out:
	debug(DEBUG_Ethnet,"ethernet init error!\n");
	return FAIL;
}

static int ether_Connect(ethernet_t *this)
{
	assert_param(this,NULL,FAIL);

	struct sockaddr_in addr;
	struct timeval timeo;
	this->ether_close(this);
	appitf_t *parent = (appitf_t *)this->parent;
	if(parent->Is_TCP){
		debug(DEBUG_Ethnet,"Connect for TCP!\n");
		this->ether_sock = socket(AF_INET, SOCK_STREAM, 0);
	}else{
		debug(DEBUG_Ethnet,"Connect for UDP!\n");
		this->ether_sock = socket(AF_INET, SOCK_DGRAM, 0);
	}
	if(this->ether_sock <= 0) goto out;

	/*设置socket 相关属性*/
	memset(&addr, 0, sizeof(addr));
	memset(&timeo,0,sizeof(timeo));
	addr.sin_family = AF_INET;
	addr.sin_port 	 = htons(parent->ServerPort);
	addr.sin_addr.s_addr = inet_addr(parent->ServerIpaddr);

	timeo.tv_sec = ether_SocketTimeout; 	 // 30 seconds 超时
	setsockopt(this->ether_sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));

	if (connect(this->ether_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		err_Print(DEBUG_Ethnet, "Connect to Server failed!\n");
		this->ether_close(this);
		goto out;
	}
	return SUCCESS;
 out:
	return FAIL;
}

static void ether_Close(ethernet_t *this)
{
	assert_param(this,NULL,;);

	if(this->ether_sock >0){
		close(this->ether_sock);
		this->ether_sock = -1;
	}
}

static int get_packageCheck(void *package,int size)
{
	assert_param(package,NULL,FAIL);

	u8 chk = 0;
	u8 *pchar = package;
	for(int i=0;i<size;++i) chk += *pchar++;
	*pchar++ = chk;
	*pchar = 0x16;
	return SUCCESS;
}
static int ether_HeartBeat(ethernet_t *this)
{
	#define HeartSize 14
	assert_param(this,NULL,FAIL);

	u8 heartbuf[24] = {0};
	u8 *heart = heartbuf;

	*heart++ = 0x68;
	memcpy(heart,((appitf_t*)this->parent)->CCUID,6);
	heart += 6;
	*heart++ = 0x68;
	*heart++ = 0xA1;
	*heart++ = 0x02;
	*heart++ = 0x02;
	*heart++ = 0x00;
	this->ether_packagecheck(heartbuf,HeartSize-2);
	return this->ether_send(this,heartbuf,HeartSize);
}
static int ether_logon(ethernet_t *this)
{
	#define LogonSize 13
	assert_param(this,NULL,FAIL);

	u8 logonBuf[24] = {0};
	u8 *logon = logonBuf;

	*logon++ = 0x68;
	memcpy(logon,((appitf_t*)this->parent)->CCUID,6);
	logon += 6;
	*logon++ = 0x68;
	*logon++ = 0xA1;
	*logon++ = 0x01;
	*logon++ = 0x01;
	this->ether_packagecheck(logonBuf,LogonSize-2);
	return this->ether_send(this,logonBuf,LogonSize);
}

static int ether_Send(ethernet_t *this,u8 *buffer,int size)
{
	#define Wait_SendEmpty  20
	assert_param(this,NULL,FAIL);
	assert_param(buffer,NULL,FAIL);

	int buflen = -1;
	if(this->ether_sock < 0) goto out;

	pthread_mutex_lock(&this->ether_lock); 	//get the ether lock
	if( send(this->ether_sock,buffer,size,MSG_NOSIGNAL) < 0){
		pthread_mutex_unlock(&this->ether_lock);	//relese the ether lock
		err_Print(DEBUG_Ethnet, "ethernet send data error!\n");
		goto out;
	}
	pthread_mutex_unlock(&this->ether_lock);	//relese the ether lock

	for(int i=0;i < Wait_SendEmpty; ++i){
		if (ioctl(this->ether_sock, SIOCOUTQ, &buflen)) {
			err_Print(DEBUG_Ethnet, "The data is not sent out!\n");
			goto out;
		}
		if (0 == buflen) return SUCCESS;
		usleep(100000);			//100ms
	}
 out:
 	return FAIL;
}

static int ether_Getchar(ethernet_t *this,u8 *buf)
{
	assert_param(this,NULL,FAIL);
	assert_param(buf,NULL,FAIL);

	if(this->ether_sock < 0) goto out;

	if(this->ether_recvlen <= 0){
		this->ether_recvlen = recv(this->ether_sock, this->ether_recvbuf, ether_RecvBufSize, MSG_DONTWAIT);

		if( this->ether_recvlen <= 0  ){
			if(errno == EWOULDBLOCK ) return RECV_NULL;
			err_Print(DEBUG_Ethnet, "ethernet recv error! \n");
			goto out;
		}else{
			this->ether_recvhead = 0;
		}
	}

	*buf = this->ether_recvbuf[this->ether_recvhead];
	++this->ether_recvhead;
	--this->ether_recvlen;
	return SUCCESS;

 out:
 	return FAIL;
}
static void ether_Relese(ethernet_t *this)
{
	assert_param(this,NULL,;);
	free(this->ether_recvbuf);
}
static int ether_Recv(ethernet_t *this,u8 *buf,int size)
{
	assert_param(this,NULL,FAIL);
	assert_param(buf,NULL,FAIL);
	int res = -1;
	u8 *pbuf = buf;
	while(size--){
		res = this->ether_getchar(this,pbuf);
		pbuf++;
		if(SUCCESS != res) return res;
	}
	return pbuf-buf;
}
ethernet_t ethernet ={
	.parent = NULL,
	.ether_init = ether_Init,
	.ether_close = ether_Close,
	.ether_logon =ether_logon,
	.ether_connect = ether_Connect,
	.ether_send = ether_Send,
	.ether_heartbeat = ether_HeartBeat,
	.ether_getchar = ether_Getchar,
	.ether_packagecheck = get_packageCheck,
	.ether_relese = ether_Relese,
	.ether_recv = ether_Recv,
};


























#if 0
#include "main.h"

#define ReLogonFrequece 500
#define CC "123456"

int sock_ether;
/*local function */
static SINT ether_recvlen = 0;			/*in func : int Ether_Getchar(UCHAR *buf)*/
static SINT ether_recvhead = 0;			/*in func : int Ether_Getchar(UCHAR *buf)*/
static UCHAR ether_recvbuf[2048];		/*in func :  int Ether_Getchar(UCHAR *buf)*/

extern UCHAR ipmain[];
extern UINT  port;
extern volatile UCHAR socket_comm_line_stat;
extern volatile UCHAR cc_register_stat;
extern volatile SINT TOPSocketConnectState;
extern volatile SINT TOPCCRegistr2ServerState;
extern GlobalCCparam CCparamGlobalInfor;
extern UCHAR error_code_txt[];
extern UCHAR UID[];
cc_global_para cc_para_term = {"192.192.191.192", "ABCDEf", "new", "baidu", "www.baidu.com", "cc"};

UINT make_logon_package(faalpkt_t *pkt)
{
	UCHAR *puc;
	UCHAR chk = 0;
	UINT len, i;
	pkt->ctrl = CC_LOGON_CTRL;
	pkt->len = CC_LOGON_LEN;
	pkt->head = pkt->dep = FAAL_HEAD;
	pkt->rtua[0] =  CCparamGlobalInfor.CCUID[0];
	pkt->rtua[1] =  CCparamGlobalInfor.CCUID[1];
	pkt->rtua[2] =  CCparamGlobalInfor.CCUID[2];
	pkt->rtua[3] =  CCparamGlobalInfor.CCUID[3];
	pkt->rtua[4] =  CCparamGlobalInfor.CCUID[4];
	pkt->rtua[5] =  CCparamGlobalInfor.CCUID[5];
	pkt->data[0] = 0x01;
	debug(DEBUG_LOCAL_ETHNET, "RTU is %x:%x:%x:%x:%x:%x ", pkt->rtua[0], pkt->rtua[1], pkt->rtua[2], pkt->rtua[3], pkt->rtua[4], pkt->rtua[5]);
	debug(DEBUG_LOCAL_ETHNET, "UID is %x:%x:%x:%x:%x:%x,\n", cc_para_term.uid[0], cc_para_term.uid[1], cc_para_term.uid[2],
	      cc_para_term.uid[3], cc_para_term.uid[4], cc_para_term.uid[5]);
	len = pkt->len + LEN_FAALHEAD;
	puc = &pkt->head;
	for (i = 0; i < len; i++) chk += *puc++;
	*puc++ = chk;
	*puc = FAAL_TAIL;
	len += LEN_FAALTAIL;
	return len;
}

u32 Make_HeartBeat_Package(faalpkt_t *pkt)
{
	u8 *puc;
	u8 chk = 0;
	u32 len = 0, i = 0;
	pkt->ctrl 	=  CC_LINE_HEATBEAT;
	pkt->len 	=  CC_LINE_HEATBEAT_LEN;
	pkt->head 	=  pkt->dep = FAAL_HEAD;
	pkt->rtua[0]  	=  CCparamGlobalInfor.CCUID[0];
	pkt->rtua[1]  	=  CCparamGlobalInfor.CCUID[1];
	pkt->rtua[2]  	=  CCparamGlobalInfor.CCUID[2];
	pkt->rtua[3]  	=  CCparamGlobalInfor.CCUID[3];
	pkt->rtua[4]  	=  CCparamGlobalInfor.CCUID[4];
	pkt->rtua[5]  	=  CCparamGlobalInfor.CCUID[5];
	pkt->data[0] 	=  0x02;
	pkt->data[1] 	=  CCparamGlobalInfor.SignalDBM;
	len = pkt->len + LEN_FAALHEAD;
	puc = &pkt->head;
	for (i = 0; i < len; i++) chk += *puc++;
	*puc++ = chk;
	*puc = FAAL_TAIL;
	len += LEN_FAALTAIL;
	return len;
}

/*****************************************************************
** 函数名: 以太方式发送
** 输　入: buf len
**　　 a---发送内容
**　　 b---发送长度
**　　 c---
** 输　出: x---0 1
**　　　x 为 1, 表示...失败
**　　　x 为 0, 表示...成功
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本      :	socket_comm_line_stat   OK
****************************************************************/
SINT Ether_RawSend(UCHAR *buf, int len)
{
	int i, buflen;
	if (sock_ether < 0) {
		TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
		TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;		//add 20120710
		err_Print(DEBUG_Ethnet, "sock_ether < 0\n");
		return FAIL;
	}
	/* 打印网络发送的数据 */
#if EthnetSend_Print == 1
	char getsocketbuf[1024];
	memset(getsocketbuf, 0, sizeof(getsocketbuf));
	Bytes2String(buf, getsocketbuf, len);
	debug(DEBUG_Ethnet, "Ethnet send:%s\n", getsocketbuf);
#endif

	pthread_mutex_lock(&mutex_ether);					//获取ether锁
	if (send(sock_ether, buf, len, MSG_NOSIGNAL) < 0) {
		pthread_mutex_unlock(&mutex_ether);			//获取ether锁
		TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
		TOPCCRegistr2ServerState	= CC_REGISTER_ERR;		//add 20120710
		return 1;
	}
	pthread_mutex_unlock(&mutex_ether);				//获取ether锁
	/* 检测数据有没有发送出去 */
	for (i = 0; i < SEND_BUF_EMPTY_TIMES; i++) {
		if (ioctl(sock_ether, SIOCOUTQ, &buflen)) {
			TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
			TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;		//add 20120710
			err_Print(DEBUG_Ethnet, "The data is not sent out!\n");
			return FAIL;
		}
		if (0 == buflen) {
			TOPSocketConnectState = SOCKET_LINESTAT_OK;
			return SUCCESS;		//ok surccess is empty now
		}
		usleep(100000);//100ms
	}

	TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
	TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;		//add 20120710

	debug(DEBUG_LOCAL_ETHNET, "%s error! TOPSocketConnectState is %d\n", __func__, TOPSocketConnectState);

	return FAIL;
}

/*****************************************************************
** 函数名:  Ether_Getchar()
** 输　入: buf
**　　 a---接收字节
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...fail
**　　　x 为 0, 表示...ok
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本      :	socket_comm_line_stat= ok
****************************************************************/

SINT Ether_Getchar(UCHAR *buf)
{
	if (sock_ether < 0) {
		TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
		TOPCCRegistr2ServerState= CC_REGISTER_ERR;		//add 20120710
		err_Print(DEBUG_LOCAL_ETHNET, "sock_ether < 0\n");
		return FAIL;
	}
	if (ether_recvlen <= 0) {
		ether_recvlen = recv(sock_ether, ether_recvbuf, 2048, MSG_DONTWAIT); /*  MSG_DONTWAIT :非阻塞 ;  !EWOULDBLOCK :链接异常;ether_recvlen未收到;需要关闭*/
		if (((ether_recvlen < 0) && (errno != EWOULDBLOCK))) { /* 连接正常;没有数据;需要继续接收*/
			err_Print(DEBUG_Ethnet, "Recv error! \n");
			CLOSE_SOCKET(sock_ether);
			TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
			TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;		//add 20120710
			return FAIL;
		} else if (ether_recvlen < 0) {
			return FAIL;
		} else {
			ether_recvhead = 0;
			TOPHEARTBEATACK = SUCCESS;	//有数据接收到，心跳标志位置成功
			if(ether_recvlen == 0){			//如果为0，则表示TCP的另一边关闭了。
				//debug(1,"At an other Shutdown the TCP!\n");
				TOPHEARTBEATACK		= FAIL;
				TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;
				TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
			}
		}
	}

	if ( ether_recvlen > 0 ) {
		*buf = ether_recvbuf[ether_recvhead++];
		ether_recvlen--;
		return SUCCESS;
	}
	TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
	TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;		//add 20120710
	return FAIL;
}



/*****************************************************************
** 函数名:  Ether_Keepalive()
** 输　入: void
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...fail
**　　　x 为 0, 表示...ok
** 功能描述:  保活
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
#ifndef UsePthread
SINT Ether_Keepalive(void)
{
	SINT rtn = 0;
	UINT back_info = 1;
	UCHAR TimeBeatUp = 0;
	static time_t PreTime = 0;
	//static time_t PreTimeForGetDBM;
	static SINT LogonFreq = 0;
	static time_t CurrentTime;
	time_t ElapesTimeInsecs = 0;
	//time_t ElapesTimeInsecsForDBM;

	debug(DEBUG_LOCAL_ETHNET, "in Ether_Keepalive function,socket_comm_line is %d\n", TOPSocketConnectState);
	time(&CurrentTime);

	if (PreTime == 0) {
		PreTime = CurrentTime;
		ElapesTimeInsecs = 1;
	} else {
		ElapesTimeInsecs = CurrentTime - PreTime;
		debug(DEBUG_LOCAL_HEATBEAT, "ElapesTimeInsecs is %ld\n", ElapesTimeInsecs);
		debug(DEBUG_LOCAL_HEATBEAT, "cc_connect_para.cycka*60 is %d\n", CCparamGlobalInfor.HeartBCycle * 60);
		//heatbeat is time up
		if (ElapesTimeInsecs > CCparamGlobalInfor.HeartBCycle * 60) {
			TimeBeatUp = 1;
			ElapesTimeInsecs = 0;
			PreTime = CurrentTime;
		}
	}//end of else
	CCparamGlobalInfor.SignalDBM = 0xFF;
 #ifdef GPRS_ITFWAY
	if (CCparamGlobalInfor.ItfWay == ITF_WAY_GPRS) {
		if (PreTimeForGetDBM == 0) {
			PreTimeForGetDBM = CurrentTime;
			ElapesTimeInsecsForDBM = 1;
			//TimeBeatUp = 1;
		}
		else {

			ElapesTimeInsecsForDBM = CurrentTime - PreTimeForGetDBM;


			debug(DEBUG_LOCAL_HEATBEAT, "ElapesTimeInsecsForDBM is %ld\n", ElapesTimeInsecsForDBM);


			//heatbeat is time up
			if (ElapesTimeInsecsForDBM > CCparamGlobalInfor.SignalDBMInterval) {
				ElapesTimeInsecsForDBM = 0;
				PreTimeForGetDBM = CurrentTime;
				CCparamGlobalInfor.SignalDBM = SingnalDBMGetFunc();
			}
		}//end of else
	}//end of if(CCparamGlobalInfor.ItfWay == ITF_WAY_GPRS){
 #endif
	/* 判断网络链接是否正常 */
	if ((SOCKET_LINESTAT_OK == TOPSocketConnectState)) {
		debug(DEBUG_LOCAL_HEATBEAT, "网络链接正常.\n");

		if ((TimeBeatUp) && (!TOPCCRegistr2ServerState)) {
			TimeBeatUp = 0;
			rtn = Ether_HeartBeat();
		}
		debug(DEBUG_LOCAL_ETHNET, "rtn is %d\n", rtn);

		if ((0 != rtn) ) {	/*  Ether_HeartBeat error*/
			TOPSocketConnectState = SOCKET_LINESTAT_ERR;
			TOPCCRegistr2ServerState = CC_REGISTER_ERR;
			debug(DEBUG_LOCAL_ETHNET, "TOPCCRegistr2ServerState = CC_REGISTER_ERR 1\n");
			debug(DEBUG_ERR_RECD_ETHNET, "ETHER_LINE_DISCNT_ERR\n");
		} else {				/*  Ether_HeartBeat ok*/
			TOPSocketConnectState = SOCKET_LINESTAT_OK;
			LogonFreq++;
			debug(DEBUG_LOCAL_CC2SERVER_RIGISTER, "*****TOPSocketConnectState is %d\n", TOPSocketConnectState);
			debug(DEBUG_LOCAL_CC2SERVER_RIGISTER, "*****LogonFreq is %d\n", LogonFreq);
			debug(DEBUG_LOCAL_CC2SERVER_RIGISTER, "*****TOPCCRegistr2ServerState is %d\n", TOPCCRegistr2ServerState);
			//(wei registed)(socket tong) and (LogonFreq > 100)
			if ((TOPCCRegistr2ServerState) && (!TOPSocketConnectState) && (LogonFreq > ReLogonFrequece)) {
				LogonFreq = 0;
				back_info = Ether_Logon();
				debug(DEBUG_LOCAL_ETHNET, "the back_info 0: logon ok;\n");
				debug(DEBUG_LOCAL_ETHNET, "the back_info is %d;\n", back_info);
				if (back_info == 0) {
					// TOPCCRegistr2ServerState = CC_REGISTER_OK;
					// TOPSocketConnectState = SOCKET_LINESTAT_OK;
					return SUCCESS;
				} else {
					//TOPCCRegistr2ServerState = CC_REGISTER_ERR;
					//TOPCCRegistr2ServerState = CC_REGISTER_ERR;
					return FAIL;
				}
			}//end of if((TOPCCRegistr2ServerState)&&(!TOPSocketConnectState)&&
		}//end of else {
	}//end of 	if((SOCKET_LINESTAT_OK == TOPSocketConnectState)&&(TimeBeatUp)) {

	LogonFreq++;
	debug(DEBUG_LOCAL_CC2SERVER_RIGISTER,
	      "**************LogonFreq is %d *****************\n", LogonFreq);
	if ((LogonFreq > ReLogonFrequece) && (TOPCCRegistr2ServerState)) {
		LogonFreq = 0;
		Ether_Disconnect();
		// TOPCCRegistr2ServerState = CC_REGISTER_ERR;
		// TOPSocketConnectState = SOCKET_LINESTAT_ERR;
		if (!Ether_Connect()) {
			sleep(CONNECT_WAIT_SECOND);
			return (Ether_Logon());
		} else {
			// TOPCCRegistr2ServerState = CC_REGISTER_ERR;
			return FAIL;
		}
	}
	debug(DEBUG_LOCAL_ETHNET, "Ether_Keepalive error\n");
	debug(DEBUG_ERR_RECD_ETHNET, "ETHER_LINE_DISCNT_ERR\n");
	// TOPCCRegistr2ServerState = CC_REGISTER_ERR;
	// TOPSocketConnectState = SOCKET_LINESTAT_ERR;
	return FAIL
}

#elif 0

s32 Ether_Keepalive(void)
{
	u8 LogonBuf[32];
	int res = -1;
	static int HeartBeatCount = 0;
	/* 未登入，或者未连接到服务器 */
	if (TOPCCRegistr2ServerState != CC_REGISTER_OK || TOPSocketConnectState != SOCKET_LINESTAT_OK) { //登入状态，未登入
		if (SUCCESS == Ether_Connect()) { //socket 连接到服务器
			TOPSocketConnectState = SOCKET_LINESTAT_OK;
			debug(1, "ether connect success!\n");
		} else {
			return FAIL;
		}
		/* 登入到服务器 */
		memset(LogonBuf, 0, sizeof(LogonBuf));
		res = make_logon_package((faalpkt_t*)LogonBuf);
		Display_package("Logon package", LogonBuf, res);
		Ether_RawSend(LogonBuf, res);
	}

	/* 心跳 */
	if (TOPHEARTBEATACK == SUCCESS) { //在心跳时间内有数据接收，TOPHEARTBEATACK会被置成SUCCESS
		TOPHEARTBEATACK = FAIL;
		HeartBeatCount = 0;
	} else { //长时间内没有数据发送就发个心跳包
		if (HeartBeatCount++ >= HeartBeatErrCnt) { //连续发送10次心跳包都没有回复，那么就需要重新连接网络
			TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;
			TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
			HeartBeatCount = 0;
		}
		/* 发送心跳包 */
		memset(LogonBuf, 0, sizeof(LogonBuf));
		res = Make_HeartBeat_Package((faalpkt_t*)LogonBuf);
		//debug(1,"HeartBeat Package len:%d\n",res);
		Display_package("HeartBeat package", LogonBuf, res);
		Ether_RawSend(LogonBuf, res);
	}

	//sleep(CCparamGlobalInfor.HeartBCycle);
	return SUCCESS;
}

#else

s32 Ether_Keepalive(void)
{
	static int HeartBeatCount = HeartBeatErrCnt;
	u8 LogonBuf[32];
	int res = -1;

	if (TOPHEARTBEATACK == SUCCESS){
		TOPHEARTBEATACK = FAIL;
		HeartBeatCount = 0;
		//debug(1,"Connect Success!\n");
	}else{
		if (HeartBeatCount++ >= HeartBeatErrCnt || CC_REGISTER_ERR == TOPCCRegistr2ServerState || SOCKET_LINESTAT_ERR == TOPSocketConnectState) {
			/* 未登入，或者未连接到服务器 */
			TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;
			TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
			if (SUCCESS == Ether_Connect()) { //socket 连接到服务器
				TOPSocketConnectState = SOCKET_LINESTAT_OK;
				debug(1, "ether connect success!\n");
			} else {
				HeartBeatCount = HeartBeatErrCnt;
				debug(1,"Connect err!\n");
				return FAIL;
			}
			HeartBeatCount = 0;
			/* 登入到服务器 */
			memset(LogonBuf, 0, sizeof(LogonBuf));
			res = make_logon_package((faalpkt_t*)LogonBuf);
			Display_package("Logon package", LogonBuf, res);
			Ether_RawSend(LogonBuf, res);
		}
		memset(LogonBuf, 0, sizeof(LogonBuf));
		res = Make_HeartBeat_Package((faalpkt_t*)LogonBuf);
		Display_package("HeartBeat package", LogonBuf, res);
		Ether_RawSend(LogonBuf, res);
	}

	return SUCCESS;
}

#endif


SINT Ether_Linestat(void)
{
	return (TOPSocketConnectState && TOPCCRegistr2ServerState);
}


/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
SINT Ether_Disconnect(void)
{
	CLOSE_SOCKET(sock_ether);
	TOPSocketConnectState = SOCKET_LINESTAT_ERR;
	TOPCCRegistr2ServerState = CC_REGISTER_ERR;
	debug(DEBUG_LOCAL_ETHNET, "TOPCCRegistr2ServerState = CC_REGISTER_ERR 2\n");
	return SUCCESS;
}


SINT Ether_HeartBeat(void)
{

	char sndbuf[50];
	faalpkt_t *pkt = (faalpkt_t *)sndbuf;
	int len;

	pkt->ctrl = CC_LINE_HEATBEAT;
	pkt->len = CC_LINE_HEATBEAT_LEN;
	pkt->data[0] = CC_LINE_HEATBEAT_DATA;	//sub cmd

	pkt->data[1] = CCparamGlobalInfor.SignalDBM;	//signal condition
	pkt->data[2] = 0x00;	//baoliu
	pkt->data[3] = 0x00;	//baoliu
	pkt->data[4] = 0x00;	//baoliu
	pkt->data[5] = 0x00;	//baoliu

	len = basic_makepkt(ITF_WAY_ETHER, pkt);
	//if((*g_faalitf[ITF_WAY_ETHER].rawsend)((UCHAR *)pkt, len)) {
	if ( Ether_RawSend((u8*)pkt, len) ) {
		debug(DEBUG_LOCAL_ETHNET, "Ether_HeartBeat error : ether_linetest()\n");

		TOPCCRegistr2ServerState = CC_LINE_STAT_ERR;
		debug(DEBUG_LOCAL_ETHNET, "TOPCCRegistr2ServerState = CC_REGISTER_ERR 3\n");
		return 1;
	}
	debug(DEBUG_LOCAL_ETHNET, "Ether_HeartBeat OK : Ether_HeartBeat()\n");

	return (SUCCESS);
}

/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本		//socket_comm_line_stat MODIFY OK
****************************************************************/
#if 0
SINT Ether_Connect(void)
{
	struct sockaddr_in addr;
	struct hostent host = {0}, *ptr;
	struct timeval timeo = {0};
	ULONG ip;

	/*初始化 sock_ether*/
	CLOSE_SOCKET(sock_ether);
	//socket_comm_line_stat = SOCKET_LINESTAT_ERR;
	TOPSocketConnectState = SOCKET_LINESTAT_ERR;
	TOPCCRegistr2ServerState = CC_REGISTER_ERR;		//add 20120710
	/*判断链接协议类型并建立 socket*/
	if (UDP) //UDP/*数据包格式，非面向链接*/
		sock_ether = socket(AF_INET, SOCK_DGRAM, 0);
	else  //TCP			/*流格式，面向链接*/
		sock_ether = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_ether < 0) {
		debug(DEBUG_LOCAL_ETHNET, "####%s create socket error!\n", __func__);
		return FAIL;
	}

	/*设置socket 相关属性*/
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(CCparamGlobalInfor.ServerPort);
	ptr = (struct hostent *)gethostbyname((CHAR *)CCparamGlobalInfor.ServerIpaddr);
	debug(DEBUG_LOCAL_ETHNET, "ServerIpaddr is%s\tServerPort is %d\tHeartBCycle is %d\n", CCparamGlobalInfor.ServerIpaddr,
	      CCparamGlobalInfor.ServerPort, CCparamGlobalInfor.HeartBCycle);
	if ( ptr ) { /*地址有效性判断*/
		host = *ptr;
	}
	debug(DEBUG_LOCAL_ETHNET, "the length of host.h_length in bit is %d\n", host.h_length);

	if ( host.h_length >= 4 ) {
		addr.sin_addr.s_addr = ip = make_long((UCHAR *)host.h_addr);
		debug(DEBUG_LOCAL_ETHNET, "connect to %d.%d.%d.%d:%d...\n", (UINT)(ip & 0xff), (UINT)((ip >> 8) & 0xff),
		      (UINT)((ip >> 16) & 0xff), (UINT)((ip >> 24) & 0xff), CCparamGlobalInfor.ServerPort);
		/*设置超时时间*/
		timeo.tv_sec = SOCKET_SET_OPT_TIMEOUT; 	 // 30 seconds 超时
		socklen_t len = sizeof(timeo);
		if (setsockopt(sock_ether, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) != 0) {
			debug(DEBUG_LOCAL_ETHNET, "socket set timeout: error code is %d\n", ETHER_SOCK_SETOPT_ERR);
		}
		if (connect(sock_ether, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			debug(DEBUG_LOCAL_ETHNET, "***SOCKET connect failed.error code is %d\n", ETHER_SOCK_CONNECT_ERR);
			CLOSE_SOCKET(sock_ether);
			return FAIL;
		}
		/*本地信息打印*/
		debug(DEBUG_LOCAL_ETHNET, "#####connect succeed#####\n");
		TOPSocketConnectState = SOCKET_LINESTAT_OK;
		return SUCCESS;
	}
	debug(DEBUG_LOCAL_ETHNET, "HOST NAME UNAVAILABLE ERR.error code is %d\n", HOST_NAME_UNAVAILABLE_ERR);
	TOPSocketConnectState = SOCKET_LINESTAT_ERR;
	TOPCCRegistr2ServerState = CC_REGISTER_ERR;		//add 20120710
	return FAIL;
}

#else

int Ether_Connect(void){
	struct sockaddr_in addr;
	struct timeval timeo;
	/*初始化 sock_ether*/
	CLOSE_SOCKET(sock_ether);
	TOPSocketConnectState = SOCKET_LINESTAT_ERR;
	TOPCCRegistr2ServerState = CC_REGISTER_ERR;
	if(CCparamGlobalInfor.Is_TCP){//TCP连接
		debug(DEBUG_Ethnet,"Connect for TCP!\n");
		sock_ether = socket(AF_INET, SOCK_STREAM, 0);
	}else{
		debug(DEBUG_Ethnet,"Connect for UDP!\n");
		sock_ether = socket(AF_INET, SOCK_DGRAM, 0);
	}
	if (sock_ether < 0) {
		err_Print(DEBUG_Ethnet, " Create socket error!\n");
		return FAIL;
	}
	/*设置socket 相关属性*/
	memset(&addr, 0, sizeof(addr));
	memset(&timeo,0,sizeof(timeo));
	addr.sin_family = AF_INET;
	addr.sin_port 	 = htons(CCparamGlobalInfor.ServerPort);
	addr.sin_addr.s_addr = inet_addr(CCparamGlobalInfor.ServerIpaddr);
	timeo.tv_sec = SOCKET_SET_OPT_TIMEOUT; 	 // 30 seconds 超时
	if (setsockopt(sock_ether, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) != 0) {
		debug(DEBUG_LOCAL_ETHNET, "socket set timeout: error code is %d\n", ETHER_SOCK_SETOPT_ERR);
	}
	if (connect(sock_ether, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		err_Print(DEBUG_Ethnet, "Connect to Server failed!\n");
		TOPSocketConnectState 	= SOCKET_LINESTAT_ERR;
		TOPCCRegistr2ServerState 	= CC_REGISTER_ERR;		//add 20120710
		CLOSE_SOCKET(sock_ether);
		return FAIL;
	}
	TOPSocketConnectState = SOCKET_LINESTAT_OK;
	return SUCCESS;
}

#endif


/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
SINT Ether_Logon(void)
{

	UCHAR sndbuf[50];
	faalpkt_t *pkt = (faalpkt_t *)sndbuf;
	UINT len_t;
	// 线路  OK 情况下
	if (TOPSocketConnectState == SOCKET_LINESTAT_OK) {
		/*登陆全部*/
		len_t = make_logon_package(pkt);
		if (Ether_RawSend((UCHAR *)pkt, len_t)) {
			//socket_comm_line_stat = SOCKET_LINESTAT_OK;
			TOPSocketConnectState = SOCKET_LINESTAT_ERR;
			TOPCCRegistr2ServerState = CC_REGISTER_ERR;
			debug(DEBUG_LOCAL_ETHNET, "ETHER rawsend failed.error code is %d\n", ETHER_RAWSEND_ERR);
			return FAIL;
		}
		if (TOPCCRegistr2ServerState == 0) {
			//cc_register_stat = CC_REGISTER_OK;
			//socket_comm_line_stat = SOCKET_LINESTAT_OK;
			TOPSocketConnectState = SOCKET_LINESTAT_OK;
			debug(DEBUG_LOCAL_ETHNET, "ETHER logon OK\n");
			return SUCCESS;
		} else { /*收到  注册失败包*/
			TOPCCRegistr2ServerState = CC_REGISTER_ERR;
			debug(DEBUG_LOCAL_ETHNET, "ETHER logon failed.error code is %d\n", ETHER_LOGON_ERR);
			debug(DEBUG_LOCAL_ETHNET, "TOPCCRegistr2ServerState = CC_REGISTER_ERR 4\n");
			//Ether_Disconnect();
			return FAIL;
		}
	} else {/*线路不通*/ //TOPSocketConnectState == SOCKET_LINESTAT_ERR
		debug(DEBUG_LOCAL_ETHNET, "ETHER logon failed.error code is %d\n", ETHER_CONNECT_CORRUPT_ERR);
		if (!Ether_Disconnect()) {
			return FAIL;
		}
		else {
			return ETHER_SOCKET_DISCNECT_ERR;
		}
	}
}


SINT debug_err_reccord(UINT value)
{
	debug(DEBUG_LOCAL_ETHNET, "need to write over\n");
	debug(DEBUG_LOCAL_ETHNET, "please make me full and the error code dd number is %d\n", value);
	debug(DEBUG_LOCAL_ETHNET, "please make me full and the error code uu number is %u\n", value);
	return SUCCESS;
}

#endif
