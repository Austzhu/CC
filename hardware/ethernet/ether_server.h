/******************************************************************
 ** 文件名:	ether_server.h
 ** Copyright (c) 2012-2014 *********公司技术开发部
 ** 创建人:	Austzhu
 ** 日　期:	2016.11
 ** 修改人:
 ** 日　期:
 ** 描　述:
 ** ERROR_CODE:
 **
 ** 版　本:	V1.0
*******************************************************************/
#ifndef __ETHER_SERVER_H__
 #define __ETHER_SERVER_H__
#include "include.h"
#include "ether.h"
#include "list.h"
typedef struct {
	int keepalive_on;            //keepalive_on = 1,start keepalive.
	int keepalive_idle;          //间隔keepalive_idle秒没有数据通信，第一次发送keepalive数据包
	int keepalive_inteval;    //keepalive 数据包发送间隔
	int keepalive_count;      //探测尝试的次数。如果第1次探测包就收到响应了,则后2次的不再发
}keepalive_t;

typedef struct client_t {
	struct list_head  entries;
	int socket_fd;
	struct sockaddr_in client_addr;
} client_t;

typedef struct server_t {
	client_t *client_header;
	pthread_t thread_listen;      //监听客户端线程
	pthread_t thread_status;     //监测客户端是否断开线程
	pthread_mutex_t client_lock;
	int thread_on;
	int client_count;
	int server_fd;
	struct sockaddr_in server_addr;

	int (*ser_send)(struct server_t*,client_t*,void *,int);
	int (*ser_recv)(struct server_t*,client_t*,void*,int,int);
	int (*ser_start)(struct server_t*,u16 port);
	int (*ser_stop)(struct server_t*);
	void (*ser_flush)(int);
	void (*ser_release)(struct server_t**);
} server_t;

extern server_t *ser_init(server_t *);

 #endif


