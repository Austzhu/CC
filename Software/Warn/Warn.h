/******************************************************************
** 文件名:	Warn.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __WARN_H__
#define __WARN_H__
#include "database.h"

#define Warn_Single_ok 		0x00
#define Warn_Coordi_ok 		0x00

#define Warn_Single_open  	0x01
#define Warn_Single_close  	0x02
#define Warn_Single_status 	0x04
#define Warn_Single_electric 	0x08
#define Warn_Single_voltage  0x10
#define Warn_Single_light 	0x20

#define Warn_Coordi_open  	0x01
#define Warn_Coordi_close  	0x02
#define Warn_Coordi_status 	0x04
#define Warn_Coordi_electric 0x08
#define Warn_Coordi_voltage 0x10
#define Warn_Coordi_light 	0x20

enum { judge_single,judge_coordi };
typedef enum {
	fault_none,			//没有故障
	fault_single,			//单灯设备故障
	fault_coordi,			//协调器设备故障
	fault_communication,	//通讯故障
	fault_light,				//灯具故障
	fault_valtage			//供电电压故障
}  fault_t;

typedef struct Warn_t{
	char warn_mark[48];
	void *topuser;
	int (*warn_setflags)(struct Warn_t*,int,int,const char*);
	int (*warn_cleanflags)(struct Warn_t*,int,int,const char*);
	int (*warn_Insert)(struct Warn_t*,int addr,int type);
	int (*warn_delete)(struct Warn_t*,int addr);
	int (*warn_verdict)(struct Warn_t*);
	void(*warn_relese)(struct Warn_t*);
} Warn_t;

extern Warn_t *warn_init(void*);

#endif	//end of  __WARN_H__
