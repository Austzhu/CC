/******************************************************************
** 文件名:	UART.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __UART_H__
#define __UART_H__
#include "include.h"
#ifdef Config_EC_6018
#include "libs_emfuture_odm.h"
#endif

#define CFG_UART_CNT	10

typedef struct uart_t{
	int32_t 	uart_fd;
	uint32_t uart_speed;
	uint8_t uart_port;
	uint8_t uart_bits;
	uint8_t uart_stop;
	uint8_t uart_parity;
	int32_t Point_flag;
	pthread_mutex_t uart_lock;
	int32_t 	(*uart_open)(struct uart_t*);
	int32_t 	(*uart_close)(struct uart_t*);
	int32_t 	(*uart_config)(struct uart_t*);
	int32_t  (*uart_readall)(struct uart_t *this, char *buf);
	int32_t 	(*uart_recv)(struct uart_t*, char *buf,uint32_t length,int32_t block);
	int32_t 	(*uart_send)(struct uart_t*,const char * buf,uint32_t length,int32_t block);
	void (*uart_flush)(struct uart_t*);
	void (*uart_relese)(struct uart_t*,int32_t);
} uart_t;

extern uart_t *uart_init(uart_t*,uint8_t port,const char*);

#endif	//#ifndef __UART_H__
