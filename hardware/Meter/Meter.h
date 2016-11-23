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
#include "CRC.h"

typedef enum { sub_open  = 1,sub_close,sub_reado,sub_readi,sub_flash } subcmd_t;
struct appitf_t;
typedef struct Meter_t{
	struct appitf_t *topuser;
	struct CRC_t *crc;

	int (*meter_open)(struct Meter_t*,u8 slave_addr, u8 ndo);
	int (*meter_close)(struct Meter_t*,u8 slave_addr, u8 ndo);
	int (*meter_readi)(struct Meter_t*,u8 slave_addr,u8 ndo,subcmd_t);
	int (*meter_reado)(struct Meter_t*,u8 slave_addr,u8 ndo,subcmd_t);
	int (*meter_flashopen)(struct Meter_t*,u8 slave_addr,u8 ndo,int ms);
	void (*meter_release)(struct Meter_t**);
} Meter_t;

extern Meter_t *meter_init(Meter_t *this,struct appitf_t *);

#endif
