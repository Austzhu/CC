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

#define ether_SocketTimeout   5

static int ether_Connect(ethernet_t *this)
{
	assert_param(this,FAIL);

	struct sockaddr_in addr;
	struct timeval timeo;
	this->ether_close(this);

	if(this->opt_param->param->Is_TCP){
		debug(DEBUG_Ethnet,"Connect for TCP!\n");
		this->opt_param->fd = socket(AF_INET, SOCK_STREAM, 0);
	}else{
		debug(DEBUG_Ethnet,"Connect for UDP!\n");
		this->opt_param->fd = socket(AF_INET, SOCK_DGRAM, 0);
	}
	if(this->opt_param->fd <= 0) goto out;

	/*设置socket 相关属性*/
	bzero(&addr,sizeof(addr));
	bzero(&timeo,sizeof(timeo));
	addr.sin_family = AF_INET;
	addr.sin_port 	 = htons(this->opt_param->param->ServerPort);
	addr.sin_addr.s_addr = inet_addr(this->opt_param->param->ServerIpaddr);

	timeo.tv_sec = ether_SocketTimeout; 	 // 5 seconds 超时
	setsockopt(this->opt_param->fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));

	if (connect(this->opt_param->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		err_Print(DEBUG_Ethnet, "Connect to Server failed!\n");
		this->ether_close(this);
		goto out;
	}
	debug(DEBUG_Ethnet,"Connect to server success!\n");
	return SUCCESS;
 out:
	return FAIL;
}

static void ether_Close(ethernet_t *this)
{
	assert_param(this,;);
	if(this->opt_param && this->opt_param->fd > 0){
		close(this->opt_param->fd);
		this->opt_param->fd = -1;
	}
}

static int get_packageCheck(void *package,int size)
{
	assert_param(package,FAIL);

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
	assert_param(this,FAIL);
	u8 heartbuf[24] = {0x68,0,0,0,0,0,0,0x68,0xA1,0x02,0x02,0x02,0};
	memcpy(heartbuf+1,this->opt_param->param->CCUID,6);
	get_check_sum(heartbuf,HeartSize-2);
	return this->ether_send(this,heartbuf,HeartSize);
}

static int ether_logon(ethernet_t *this)
{
	#define LogonSize 13
	assert_param(this,FAIL);

	u8 logonBuf[24] = {0x68,0,0,0,0,0,0,0x68,0xA1,0x01,0x01,0};
	memcpy(logonBuf+1,this->opt_param->param->CCUID,6);
	get_check_sum(logonBuf,LogonSize-2);
	return this->ether_send(this,logonBuf,LogonSize);
}

static int ether_Send(ethernet_t *this,u8 *buffer,int size)
{
	assert_param(this,FAIL);
	assert_param(buffer,FAIL);

	int fd = this->opt_param->fd;
	int buflen = -1;
	if(fd < 0) goto out;

	pthread_mutex_lock(&this->ether_lock);                      //get the ether lock
	if( send(fd,buffer,size,MSG_NOSIGNAL) < 0){
		pthread_mutex_unlock(&this->ether_lock);          //relese the ether lock
		err_Print(DEBUG_Ethnet, "ethernet send data error!\n");
		goto out;
	}
	pthread_mutex_unlock(&this->ether_lock);                 //relese the ether lock
	/* make sure send success! */
	for(int i=0; i < 30; ++i){
		if (ioctl(fd, SIOCOUTQ, &buflen)) {
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
	assert_param(this,FAIL);
	assert_param(buf,FAIL);

	int socket_fd = this->opt_param->fd;
	if(socket_fd < 0) goto out;

	if(this->opt_param->r_len <= 0){
		this->opt_param->r_len = recv(socket_fd,this->opt_param->r_buf,Buffer_Size, MSG_DONTWAIT);
		/* recv error or the peer has performed an orderly shutdown */
		if( this->opt_param->r_len <= 0  ){
			/* have no data */
			if(errno == EWOULDBLOCK ) return RECV_NULL;
			err_Print(DEBUG_Ethnet, "ethernet recv error! \n");
			goto out;
		}else  this->opt_param->r_ptr = 0;
	}
	*buf = this->opt_param->r_buf[this->opt_param->r_ptr];
	++this->opt_param->r_ptr;
	--this->opt_param->r_len;
	return SUCCESS;
 out:
 	return FAIL;
}

static int ether_Recv(ethernet_t *this,u8 *buf,int size)
{
	assert_param(this,FAIL);
	assert_param(buf,FAIL);

	int res = -1;
	u8 *pbuf = buf;
	while(size--){
		if(SUCCESS != (res = this->ether_getchar(this,pbuf++)) )
			return res;
	}
	return pbuf-buf;
}

static void ether_Relese(ethernet_t **this)
{
	assert_param(this,;);
	assert_param(*this,;);

	(*this)->ether_close(*this);
	FREE(*this);
}

ethernet_t *ether_Init(ethernet_t *this,struct opt_param_t *opt_param)
{
	assert_param(opt_param,NULL);

	ethernet_t *pth = this;
	if(!pth){
		this = malloc(sizeof(ethernet_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(ethernet_t));

	this->opt_param = opt_param;
	this->opt_param->fd 		= -1;
	this->opt_param->r_ptr	= 0;
	this->opt_param->r_len	= 0;
	pthread_mutex_init(&(this->ether_lock),NULL);

	this->ether_connect  = ether_Connect;
	this->ether_logon  = ether_logon;
	this->ether_packagecheck  = get_packageCheck;
	this->ether_send  = ether_Send;
	this->ether_heartbeat  = ether_HeartBeat;
	this->ether_getchar  = ether_Getchar;
	this->ether_recv  = ether_Recv;
	this->ether_relese  = ether_Relese;
	this->ether_close  = ether_Close;
	if( !this->opt_param || !this->ether_connect || !this->ether_logon ||\
		!this->ether_packagecheck || !this->ether_send || !this->ether_heartbeat || \
		!this->ether_getchar || !this->ether_recv || !this->ether_relese || !this->ether_close){
		err_Print(DEBUG_Ethnet,"Here are some Api pointer is NULL!\n");
		goto out;
	}
	return this;
 out:
	if(!pth) FREE(this);
	return NULL;
}
