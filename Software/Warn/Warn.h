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

#define Warn_Single_open  	0x01
#define Warn_Single_close  	0x02
#define Warn_Single_status 	0x04
#define Warn_Single_electric 	0x08
#define Warn_Single_ok 		0x00
#define Warn_Single_All  		(Warn_Single_open|Warn_Single_close|Warn_Single_status|Warn_Single_electric)

#define Warn_Coordi_open  	0x01
#define Warn_Coordi_close  	0x02
#define Warn_Coordi_status 	0x04
#define Warn_Coordi_electric 0x08
#define Warn_Coordi_ok 		0x00
#define Warn_Coordi_All 		(Warn_Coordi_open|Warn_Coordi_close|Warn_Coordi_status|Warn_Coordi_electric)

typedef enum {sw_single,sw_group,sw_brocast,sw_inCoordi,sw_coordi,so_single,so_group,so_brocast } flags_enum;

typedef enum {
	fault_none		= 0x00,			//没有故障
	fault_single	= 0x01,			//单灯设备故障
	fault_coordi 	= 0x02,			//协调器设备故障
	fault_communication = 0x04,	//通讯故障
	fault_light 		= 0x08,			//灯具故障
	fault_valtage	= 0x10,			//供电电压故障
	fault_module 	= 0x20,			//一盏灯具有多个模组，其中一个或多个模组坏了
}  fault_t;

typedef struct sin_warn_t{
	int addr;
	int Warn_flags;
	int opt_flags;
	int Rpow;  int Rvol;  int Rpf; 	//额定功率, 电压, 功率因数
	int light;    int elec;  int vol;		//实际 状态,调光值,电流,电压
	fault_t fault_flags;				//故障内容
} sin_warn_t;

struct Warn_info_t{
	int Coor_Addr;
	int Warn_flags;
	int fault_flags;
	int Single_count;		//单灯有错误标记的个数
	sin_warn_t *single_info;
};

struct appitf_t;
typedef struct Warn_t{
	struct appitf_t *topuser;
	int (*warn_setflags)(struct Warn_t*,int,int,int);
	int (*warn_cleanflags)(struct Warn_t*,int,int,int);
	int (*warn_Insert)(struct Warn_t*,int addr,int type);
	int (*warn_verdict)(struct Warn_t*);
	void(*warn_relese)(struct Warn_t**);
} Warn_t;

extern Warn_t *warn_init(void*);

#endif	//end of  __WARN_H__
