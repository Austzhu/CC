/********************************************************************
	> File Name:	database.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年03月30日 星期三 10时38分34秒
 *******************************************************************/
#ifndef __database_h__
#define __database_h__
#include "include.h"
#include "sqlite3.h"

#ifndef CFG_DB_NAME
#error "must define the path name of data base"
#endif

#define Get_CountByCondition(ptr,table,Column,Condition) ({ int count;\
ptr->sql_select(Asprintf("select count(%s) from %s  %s;",Column,table,\
Condition), (char*)&count,sizeof(int),1,0) ==SUCCESS ? (int)count:-1;})

#define Get_CountByColumn(ptr,table,Column)  Get_CountByCondition(ptr,table,Column," ")
#define Get_light_info_count(ptr,Column)  Get_CountByColumn(ptr,CFG_tb_light_info,Column)

#define sql_isexist(ptr,table,Condition)  ( {int Addr = 0;ptr->sql_select(\
Asprintf("select Base_Addr from %s where Base_Addr=%d;",table,\
Condition), (char*)&Addr,sizeof(int),1,0) == SUCCESS ? Addr : 0;})


/* 协调器记录表（db_coordinator） */
typedef struct{
	u32 id;	                         //主键ID
	u32 Wl_Addr;              //物理地址
	u32 Base_Addr;          //协调器地址
	u32 Coor_gid;             //协调器组ID
	s8 CC_id[16];               //集中器ID
	u32 Map_Addr;	         //被映射到Base_Addr的实际地址
	u32 Warn_flags;
} TableCoordi_t;

/* 单灯记录表（db_light） */
typedef struct{
	u32 id;                           //主键ID
	u32 Wl_Addr;              //物理地址
	u32 Base_Addr;          //单灯地址
	u32 lt_gid;                    //单灯组ID
	u32 Coor_id;                //对应的协调器的地址
	u32 Map_Addr;           //被映射到Base_Addr的实际地址
} TableSingle_t;

/* 单灯信息记录表（db_info_light） */
typedef struct {
	s32 id;				//主键
	s32 Base_Addr;		//单灯地址
	u32 operate_flags; //操作设备状态
	u32 Warn_flags;	//报警的标志
	u32 Rate_v;			//额定电压
	u32 Rate_p;		//额定功率
	u32 Rate_PF;		//功率因数
	s32 light_status;	//灯状态
	u32 light_val;		//灯调光值
	u32 light_E;			//灯电流值
	u32 light_P;			//灯功率值
	u32 light_V;			//灯电压值
	u32 light_D;		//灯电量值
	u32 rtime;			//更新时间
} Tableinfolight_t;


/* 报警日志记录表（db_warn） */
typedef struct{
	u32 	id;
	u32		Add_time;	//报警时间
	u32		Type;		//报警类型
	u32		Grade;		//报警等级
	u32		State;		//状态
	u32 	addr;		//单灯/协调器地址
	u8	Remark[48];	//备注
} Tablewarn_t;


typedef struct sql_t{
	int (*sql_insert)(const char*);
	int (*sql_delete)(const char*);
	int (*sql_update)(const char*,const char*);
	int (*sql_select)(const char*, char*,int,int,int,...);
	int (*sql_Isexist)(const char *table,const char *condition);
	void (*sql_release)(struct sql_t*);
} sql_t;

extern sql_t *sql_Init(sql_t*);

#endif
