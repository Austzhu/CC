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

#define ether_RecvBufSize        2048
#define ether_SocketTimeout   30

static int ether_Connect(ethernet_t *this)
{
	assert_param(this,NULL,FAIL);

	struct sockaddr_in addr;
	struct timeval timeo;
	this->ether_close(this);
	appitf_t *parent = (appitf_t *)this->parent;
	if(parent->param.Is_TCP){
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
	addr.sin_port 	 = htons(parent->param.ServerPort);
	addr.sin_addr.s_addr = inet_addr(parent->param.ServerIpaddr);

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
	memcpy(heart,((appitf_t*)this->parent)->param.CCUID,6);
	heart += 6;
	*heart++ = 0x68;  *heart++ = 0xA1;
	*heart++ = 0x02;  *heart++ = 0x02;
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
	memcpy(logon,((appitf_t*)this->parent)->param.CCUID,6);
	logon += 6;
	*logon++ = 0x68;  *logon++ = 0xA1;
	*logon++ = 0x01;  *logon++ = 0x01;
	this->ether_packagecheck(logonBuf,LogonSize-2);
	return this->ether_send(this,logonBuf,LogonSize);
}

static int ether_Send(ethernet_t *this,u8 *buffer,int size)
{
	#define Wait_SendEmpty  30
	assert_param(this,NULL,FAIL);
	assert_param(buffer,NULL,FAIL);
	if(this->ether_sock < 0) goto out;

	int buflen = -1;
	pthread_mutex_lock(&this->ether_lock);                      //get the ether lock
	if( send(this->ether_sock,buffer,size,MSG_NOSIGNAL) < 0){
		pthread_mutex_unlock(&this->ether_lock);          //relese the ether lock
		err_Print(DEBUG_Ethnet, "ethernet send data error!\n");
		goto out;
	}
	pthread_mutex_unlock(&this->ether_lock);                 //relese the ether lock
	/* make sure send success! */
	for(int i=0; i < Wait_SendEmpty; ++i){
		if (ioctl(this->ether_sock, SIOCOUTQ, &buflen)) {
			err_Print(DEBUG_Ethnet, "The data is not sent out!\n");
			goto out;
		}
		if (0 == buflen) return SUCCESS;
		usleep(50000);               //50ms
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
		this->ether_recvlen = recv(this->ether_sock,\
			this->ether_recvbuf, ether_RecvBufSize, MSG_DONTWAIT);
		/* recv error or the peer has performed an orderly shutdown */
		if( this->ether_recvlen <= 0  ){
			/* have no data */
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

static int ether_Recv(ethernet_t *this,u8 *buf,int size)
{
	assert_param(this,NULL,FAIL);
	assert_param(buf,NULL,FAIL);
	int res = -1;
	u8 *pbuf = buf;
	while(size--){
		res = this->ether_getchar(this,pbuf++);
		if(SUCCESS != res) return res;
	}
	return pbuf-buf;
}

static void ether_Relese(ethernet_t **this)
{
	assert_param(this,NULL,;);
	assert_param(*this,NULL,;);
	free((*this)->ether_recvbuf);
	(*this)->ether_close(*this);
	memset(*this,0,sizeof(ethernet_t));
	free(*this);   *this = NULL;
}

ethernet_t *ether_Init(struct appitf_t *topuser)
{
	ethernet_t *this = malloc(sizeof(ethernet_t));
	if(!this) return NULL;

	memset(this,0,sizeof(ethernet_t));

	this->ether_sock = -1;
	this->ether_recvbuf = calloc(ether_RecvBufSize,sizeof(char));
	if(!this->ether_recvbuf) goto out;
	pthread_mutex_init(&(this->ether_lock),NULL);

	this->parent =  topuser;
	this->ether_connect  = ether_Connect;
	this->ether_logon  = ether_logon;
	this->ether_packagecheck  = get_packageCheck;
	this->ether_send  = ether_Send;
	this->ether_heartbeat  = ether_HeartBeat;
	this->ether_getchar  = ether_Getchar;
	this->ether_recv  = ether_Recv;
	this->ether_relese  = ether_Relese;
	this->ether_close  = ether_Close;

	if( !this->parent || !this->ether_connect || !this->ether_logon ||\
		!this->ether_packagecheck || !this->ether_send || !this->ether_heartbeat || \
		!this->ether_getchar || !this->ether_recv || !this->ether_relese || !this->ether_close){
		err_Print(DEBUG_Ethnet,"Here are some Api pointer is NULL!\n");
		goto out1;
	}

	return this;
 out1:
	free(this->ether_recvbuf);
 out:
	free(this);
	return NULL;
}
