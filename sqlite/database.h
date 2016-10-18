/********************************************************************
	> File Name:	database.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年03月30日 星期三 10时38分34秒
 *******************************************************************/

#ifndef __database_h__
#define __database_h__
#include "include.h"
#include "./include/sqlite3.h"

#define StringSize 	32

#define Table_Single		"db_light"
#define Table_Coordi 		"db_coordinator"
#define Table_Task 		"db_task"
#define Table_Tasklist		"db_tasklist"
#define Table_Warn		"db_warn"
#define Table_Info 		"db_info_light"


#define Column_Single 	"Wl_Addr,Base_Addr,lt_gid,Coor_id,Map_Addr"
#define Column_Coordi 	"Wl_Addr,Base_Addr,Coor_gid,CC_id,Map_Addr"
#define Column_Task	 	"Name,Priority,Start_Date,End_Date,Run_Time,Inter_Time,Type,State"
#define Column_Tasklist 	"Tk_id,Rank,Cmd,Wait_time"
#define Column_Warn 	"Add_time,Type,Grade,State,Remark"

#define Get_light_info_count(Column)  Get_CountByColumn(Table_Info,Column)
#define Get_CountByColumn(table,Column)  Get_CountByCondition(table,Column," ")
// #define Get_CountByColumn(table,Column)  ({int count;
// 	Select_Table_V2(Asprintf("select count(%s) from %s;",Column,table), (char*)&count,sizeof(int),1,0) ==SUCCESS ? (int)count:-1;})
#define Get_CountByCondition(table,Column,Condition) ({ int count;\
	Select_Table_V2(Asprintf("select count(%s) from %s  %s;",Column,table,Condition), (char*)&count,sizeof(int),1,0) ==SUCCESS ? (int)count:-1;})
/**
 *  增删改查的命令字段
 */
typedef enum{
	Cmd_light		= 0x01,
	Cmd_coordi	= 0x02,
	Cmd_task 		= 0x04,
	Cmd_tasklist	= 0x08,
	Cmd_warn		= 0x10,
	Cmd_String	= 0x20,                  //查询的整条语句用字符串传入
	Cmd_Info,
} CmdTable_t;

/* 协调器记录表（db_coordinator） */
typedef struct{
	u32 id;	                         //主键ID
	u32 Wl_Addr;              //物理地址
	u32 Base_Addr;          //协调器地址
	u32 Coor_gid;             //协调器组ID
	s8 CC_id[16];               //集中器ID
	u32 Map_Addr;	         //被映射到Base_Addr的实际地址
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
	s32 Base_Addr;	//单灯地址
	u32 Warn_flags;	//报警的标志
	u32 Rate_v;			//额定电压
	u32 Rate_p;		//额定功率
	u32 Rate_PF;		//功率因数
	s32 light_status;	//灯状态
	u32 light_val;		//灯调光值
	u32 light_E;		//灯电流值
	u32 light_P;		//灯功率值
	u32 light_V;		//灯电压值
	u32 light_D;		//灯电量值
	u32 rtime;			//更新时间
} Tableinfolight_t;

/* 任务表(db_task) */
typedef struct{
	u32 	id;
	u8	Name[StringSize];	//任务名称
	u32  	Priority; 		//优先级
	time_t 	Start_Date;		//开始日期
	time_t	End_Date;		//结束日期
	time_t	Run_Time;		//运行时间
	u32	Inter_Time; 		//运行时间间隔
	u32	Type;			//任务类型，单次/循环
	u32	State;			//任务执行状态
} TableTask_t;

/* 任务明细表（db_tasklist） */
typedef struct{
	u32 	id;
	u32	Tk_id;		//任务表id
	u32	Rank;		//执行顺序
	u8	Cmd[300];	//任务命令
	u32	Wait_time;	//执行延迟时间
} TableTasklist_t;

/* 报警日志记录表（db_warn） */
typedef struct{
	u32 	id;
	u32	Add_time;	//报警时间
	u32	Type;		//报警类型
	u32	Grade;		//报警等级
	u32	State;		//状态
	u8	Remark[48];	//备注
} Tablewarn_t;

extern s32 Insert_Table(u32 cmd, ...);
extern s32 Insert_Table_v2(const char *sql);
extern s32 Delete_Table(u32 cmd,const char *Condition);
extern s32 Update_Table(u32 cmd,const char *Condition);
extern s32 Update_Table_v2(const char *table,const char *Condition);
extern s32 Select_Table(u32 cmd,const char *Column,const char*Condition,...);
extern s32 Select_Table_V2(const char *sql, char *buf,int RowSize,int ColSize,int strcount,...);

#endif
