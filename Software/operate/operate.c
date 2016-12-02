/******************************************************************
** 文件名:	operate.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "operate.h"
#include "Interface.h"

static int opt_connect(struct operate_t *this)
{
	assert_param(this,FAIL);
	switch(this->opt_param.param->ItfWay){
		case ether_net:
			return ((ethernet_t*)this->opt_Itfway)->ether_connect(this->opt_Itfway);
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	return FAIL;
}

static int opt_logon(struct operate_t *this)
{
	assert_param(this,FAIL);
	switch(this->opt_param.param->ItfWay){
		case ether_net:
			return ((ethernet_t*)this->opt_Itfway)->ether_logon(this->opt_Itfway);
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	return SUCCESS;
}

static int opt_send(struct operate_t *this,u8 *buffer,int size)
{
	assert_param(this,FAIL);
	switch(this->opt_param.param->ItfWay){
		case ether_net:
			return ((ethernet_t*)this->opt_Itfway)->ether_send(this->opt_Itfway,buffer,size);
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	return SUCCESS;
}

static int opt_heartbeat(struct operate_t *this)
{
	assert_param(this,FAIL);
	switch(this->opt_param.param->ItfWay){
		case ether_net:
			return ((ethernet_t*)this->opt_Itfway)->ether_heartbeat(this->opt_Itfway);
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	return SUCCESS;
}

static int opt_getchar(struct operate_t *this,u8 *ch)
{
	assert_param(this,FAIL);
	assert_param(ch,FAIL);

	switch(this->opt_param.param->ItfWay){
		case ether_net:
			return ((ethernet_t*)this->opt_Itfway)->ether_getchar(this->opt_Itfway,ch);
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	return SUCCESS;
}

static int opt_recv(struct operate_t *this,u8 *buf, int size)
{
	assert_param(this,FAIL);
	assert_param(buf,FAIL);

	switch(this->opt_param.param->ItfWay){
		case ether_net:
			return ((ethernet_t*)this->opt_Itfway)->ether_recv(this->opt_Itfway,buf,size);
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	return SUCCESS;
}

static void opt_relese(struct operate_t **this)
{
	assert_param(this,;);
	assert_param(*this,;);

	switch((*this)->opt_param.param->ItfWay){
		case ether_net:
			((ethernet_t*)(*this)->opt_Itfway)->ether_relese( (ethernet_t**)&((*this)->opt_Itfway) );
			(*this)->opt_Itfway = NULL;
			break;
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:break;
	}
	/* release recv buffer */
	FREE((*this)->opt_param.r_buf);
	FREE(*this);
	*this = NULL;
}

operate_t *operate_init(struct operate_t *this, struct param_t *param)
{
	assert_param(param,NULL);

	operate_t *const temp = this;
	if(!temp){
		this = (operate_t*)malloc(sizeof(operate_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(operate_t));
	/* malloc for recv buffer */
	this->opt_param.r_buf = calloc(Buffer_Size,sizeof(char));
	if(!this->opt_param.r_buf)  goto out1;

	this->opt_param.param = param;
	switch(param->ItfWay){
		case ether_net:
			this->opt_Itfway = ether_Init(this->opt_Itfway,&(this->opt_param));
			if(!this->opt_Itfway) goto out;
			break;
		case gprs:break;
		case zigbee:break;
		case lora:break;
		default:
			debug(DEBUG_app,"Unknow communication %d.\n",param->ItfWay);
			goto out;
	}

	this->opt_connect 		= opt_connect;
	this->opt_logon 		= opt_logon;
	this->opt_send 		= opt_send;
	this->opt_heartbeat 	= opt_heartbeat;
	this->opt_getchar 		= opt_getchar;
	this->opt_recv 			= opt_recv;
	this->opt_relese 		= opt_relese;

	return this;
out:
	FREE(this->opt_param.r_buf);
out1:
	if(!temp) FREE(this);
	return NULL;
}
