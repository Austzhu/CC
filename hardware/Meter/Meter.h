/******************************************************************
** 文件名:	Meter.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __Meter_H__
#define __Meter_H__
#include "include.h"
#include "UART.h"
#include "database.h"

enum { UP_DI,UP_DO,UP_ALL };

typedef enum { sub_open  = 1,sub_close,sub_reado,sub_readi,sub_flash } subcmd_t;
struct appitf_t;
typedef struct Meter_t{
	struct appitf_t *topuser;
	struct uart_t *uart;
	struct sql_t *sql;

	int32_t Point_flag;
	int32_t (*meter_querydi)(struct Meter_t*,uint8_t addr,uint8_t ndi);
	int32_t (*meter_querydo)(struct Meter_t*,uint8_t addr, uint8_t ndo);
	int32_t (*meter_open)(struct Meter_t*,uint8_t addr, uint8_t num,uint32_t ndo);
	int32_t (*meter_close)(struct Meter_t*,uint8_t addr, uint8_t num,uint32_t ndo);
	int32_t (*meter_readi)(struct Meter_t*,uint8_t addr, uint8_t num);
	int32_t (*meter_reado)(struct Meter_t*,uint8_t addr, uint8_t num);
	int32_t (*meter_query_dido)(struct Meter_t*,uint8_t,uint8_t*);
	int32_t (*meter_flashopen)(struct Meter_t*,uint8_t addr,uint8_t num,uint32_t ndo,int32_t ms);
	void (*meter_recover_status)(struct Meter_t*);
	void (*meter_release)(struct Meter_t*);
} Meter_t;

extern Meter_t *meter_init(Meter_t *this,struct appitf_t *);

#endif
