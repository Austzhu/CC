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
#include "include.h"
appitf_t g_appity;

int appitf_init(appitf_t *app)
{
	memset(app,0,sizeof(appitf_t));
	loadParam(app);
	return 0;
}
