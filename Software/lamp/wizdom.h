/******************************************************************
** 文件名:	wizdom.h
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
#ifndef __WIZDOM_H__
#define __WIZDOM_H__
#include "include.h"
#include "ether_server.h"

enum {
	Register_brigt 		= 0x0100, 		//光照
	Register_temperature 	= 0x0200, 	//温度
	Register_rain 		= 0x0400, 		//雨量
	Register_wind 		= 0x0600, 		//风速
	Register_soil 		= 0x0700, 		//土壤
	Register_stive 		= 0x0700, 		//粉尘
};

typedef struct { u8 addr;  int Register;  void *result;  int result_size; } result_t;

typedef struct wizdom_t{
	server_t *ether_server;
	//CRC_t *crc;
	int (*wiz_Query)(struct wizdom_t*,client_t*,result_t*);
	int (*wiz_getaddr)(struct wizdom_t*);
	int (*wiz_setaddr)(struct wizdom_t*,char addr);

	void (*wiz_display)(const char*,const char*,int);
	void (*wiz_release)(struct wizdom_t**);
} wizdom_t;

extern wizdom_t *wiz_init(wizdom_t*);
#endif
