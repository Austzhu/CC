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
#include "update.h"

#define Set_db_group_info(p_sql,gid,val) do{\
	int cnt = 0;\
	if( (gid) < 0){\
		(p_sql)->sql_update("db_group_info",Asprintf("set pwm_value=%d ",(val)));\
		break;\
	}\
	if(SUCCESS != (p_sql)->sql_select(Asprintf("select count(*) from "\
		"db_group_info where gid=%d;",(gid) ),(char*)&cnt,sizeof(int),1,0)) break;\
	if(cnt > 0){	/* 表中存在 */\
		(p_sql)->sql_update("db_group_info",Asprintf("set pwm_value=%d where gid=%d",(val),(gid)));\
	}else{	/* 表中不存在 */\
		(p_sql)->sql_insert(Asprintf("insert into db_group_info(gid,pwm_value) values(%d,%d); ",(gid),(val)));\
	}\
}while(0)

#define GetSingleCunt    30
#define Query_res             0x80
#define Query_nores        0

typedef enum {cfg_sinMap,  cfg_sinGroup,  cfg_coorMap } sin_cfg_t;
typedef enum{cmd_single,  cmd_group,  cmd_broadcast,  cmd_grouplight,  cmd_broadlight } cmd_t;
typedef enum {Query_elec,Query_stat } Query_t;

typedef struct {
	u8	Header;
	u8 	Ctrl;
	u8 	Group_Addr;
	u8 	Coordi_Addr;
	u8 	Single_Addr[2];   //高地址在前，低地址在后
	u8 	Cmd[2];                  //高字节在前，低字节在后
	u8 	Data[2];
	u8 	Crc16[2];
}light_t;

struct appitf_t;
typedef struct Single_t{
	struct appitf_t *topuser;
	struct update_t *update;
	//struct CRC_t *crc;

	int (*sin_open)(struct Single_t*,int,u32,u32);
	int (*sin_close)(struct Single_t*,int,u32);
	int (*sin_reply)(struct Single_t*,int,int,int);
	int (*sin_config)(struct Single_t*,sin_cfg_t,void*);
	int (*sin_Queryelectric)(struct Single_t*,int,u32);
	int (*sin_Querystatus)(struct Single_t*,int,u32);
	int (*sin_RecvPackage)(struct Single_t*,void*,int,int);
	int (*sin_update)(struct Single_t*,int);
	void (*sin_release)(struct Single_t**);
	void (*Display)(const char*,void*,int);
} Single_t;

extern Single_t *single_Init(Single_t *,struct appitf_t *topuser);

#endif
