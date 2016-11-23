/******************************************************************
** 文件名:	CRC.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2012.11
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef  __CRC_H__
#define __CRC_H__
#include "include.h"

typedef struct CRC_t{
	int (*CRCHL_get)(char*,const char*,int);
	int (*CRCHL_check)(const char*,const char*,int);
	int (*CRCLH_get)(char*,const char*,int);
	int (*CRCLH_check)(const char*,const char*,int);
	void (*CRC_release)(struct CRC_t**);
} CRC_t;

extern CRC_t *CRC_init(CRC_t *);
#endif
