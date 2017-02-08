/******************************************************************
 ** 文件名:	main.c
 ** Copyright (c) 2012-2014 *********公司技术开发部
 ** 创建人:	wguilin
 ** 日　期:	2012.03
 ** 修改人:
 ** 日　期:
 ** 描　述:	16  进制打印
 ** ERROR_CODE:
 **
 ** 版　本:	V1.0
*******************************************************************/
#include "Interface.h"

int main(int argc,char *argv[])
{
	appitf_t *this = &g_appity;
	if(this->TopUser_Init(this) != SUCCESS ){
		debug(DEBUG_app,"system init error!\n");
		this->TopUser_relese(this);
	}
	while(true){
		if(this->Connect_status == Connect_ok){
			this->TopUser_setMode(this,this->param.ControlMethod);
		}else{
			this->TopUser_stopMode(this,this->param.ControlMethod);
		}
		msleep(5000);
	}
}

