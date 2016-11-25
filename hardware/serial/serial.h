/******************************************************************
** 文件名:	serial.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "include.h"

#define  SerialMax 10

typedef struct serial_t{
	int 	serialfd[SerialMax];
	int 	(*serial_open)(struct serial_t*,u32);
	int 	(*serial_close)(struct serial_t*,u32);
	int 	(*serial_config)(struct serial_t*,u32,u32,s32,s32,s8);
	int 	(*serial_recv)(struct serial_t*,u32,  s8*,u32,s32);
	int 	(*serial_send)(struct serial_t*,u32,  s8*,u32,s32);
	void (*serial_flush)(struct serial_t*,int);
	void (*serial_relese)(struct serial_t**);
} serial_t;

extern serial_t *serial_Init(serial_t*,u32 port,...);

#endif
