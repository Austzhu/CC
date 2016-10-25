/******************************************************************
** 文件名:	single.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __single_h__
#define __single_h__
#include "include.h"
#include "taskque.h"

#define GetSingleCunt 30
#define Query_res 0x80
#define Query_nores 0

typedef struct {
	u8	Header;
	u8 	Ctrl;
	u8 	Group_Addr;
	u8 	Coordi_Addr;
	u8 	Single_Addr[2];	//高地址在前，低地址在后
	u8 	Cmd[2];		//高字节在前，低字节在后
	u8 	Data[2];
	u8 	Crc16[2];
}light_t;

typedef enum {cfg_sinMap,  cfg_sinGroup,  cfg_coorMap } sin_cfg_t;
typedef enum{cmd_single,  cmd_group,  cmd_broadcast,  cmd_grouplight,  cmd_broadlight } cmd_t;
typedef enum {Query_elec,Query_stat } Query_t;


typedef struct Single_t{
	void *parent;
	int (*sin_open)(struct Single_t*,int,u32,u32);
	int (*sin_close)(struct Single_t*,int,u32);
	int (*sin_reply)(struct Single_t*,int,int,int);
	int (*sin_config)(struct Single_t*,sin_cfg_t,void*);
	int (*sin_Queryelectric)(struct Single_t*,int,u32);
	int (*sin_Querystatus)(struct Single_t*,int,u32);
	int (*sin_RecvPackage)(struct Single_t*,void*,int,int);
	int (*sin_init)(struct Single_t*,void*);
	void (*Display)(const char*,void*,int);
} Single_t;

extern Single_t g_single;
#endif
