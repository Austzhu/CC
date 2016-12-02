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

struct appitf_t;
typedef struct ethernet_t{
	int ether_sock;					//网络连接的socket
	int ether_recvlen;				//缓存数据的长度
	int ether_recvhead;			//缓存数据中的读指针偏移量
	u8 *ether_recvbuf;				//数据的缓存区
	struct appitf_t *parent;
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

extern ethernet_t *ether_Init(ethernet_t*,struct appitf_t *topuser);

#endif
