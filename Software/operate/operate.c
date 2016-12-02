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


operate_t *operate_init(struct operate_t *this, struct param_t *param)
{
	assert_param(param,NULL);
	operate_t *const temp = this;
	if(!temp){
		this = (operate_t*)malloc(sizeof(operate_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(operate_t));

	this->opt_param.param = param;

	switch(param->ItfWay){
		case ether_net:
			INIT_FAIL(this->opt_Itfway,ether_Init,this);	/* init for ethernet */
			break;
		case gprs:break;
		case zigbee:break;
		default:debug(DEBUG_app,"Unknow communication %d.\n",this->param.ItfWay); return FAIL;
	}

	return this;
out:
	if(!temp) FREE(this);
	return NULL;
}
