/******************************************************************
** 文件名:	infor_out.c
** Copyright (c) 2012-2014 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/

#ifndef __ETHER_H__
#define __ETHER_H__
#include "include.h"
struct opt_param_t;
typedef struct ethernet_t{
	struct opt_param_t *opt_param;
	pthread_mutex_t ether_lock;

	int (*ether_connect)(struct ethernet_t*);
	int (*ether_logon)(struct ethernet_t*);
	int (*ether_packagecheck)(void *package,int size);
	int (*ether_send)(struct ethernet_t*,u8 *buffer,int size);
	int (*ether_heartbeat)(struct ethernet_t*);
	int (*ether_getchar)(struct ethernet_t*,u8*);
	int (*ether_recv)(struct ethernet_t*,u8*,int);
	void (*ether_relese)(struct ethernet_t**);
	void (*ether_close)(struct ethernet_t*);
} ethernet_t;

extern ethernet_t *ether_Init(ethernet_t*,struct opt_param_t*opt_param);

#endif
