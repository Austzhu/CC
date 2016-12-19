/******************************************************************
** 文件名:	porccess.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "include.h"
#include "taskque.h"
#include "Interface.h"

extern int Reset2DoFunctions(void);
extern int time_tick(u8*);
extern int Query_time(u8 *buf,int bufsize);
extern int CC_Inquire_Version(u8 *buf,int size);
extern int SingleConfig(u8 *package,appitf_t *app);
extern int CoordiConfig(u8 *package,appitf_t *app);
extern int delete_sql(u8 *package,appitf_t *app);
extern int tunnel_config(u8 *package,appitf_t *app);

#endif
