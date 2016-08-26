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

#define Sqlite3
#define StringSize 	32
#ifdef Sqlite3

/**
 *  增删改查的命令字段
 */
typedef enum{
	Cmd_light 	= 0x01,
	Cmd_coordi	= 0x02,
	Cmd_task 	= 0x04,
	Cmd_tasklist	= 0x08,
	Cmd_warn	= 0x10,
	Cmd_String	= 0x20 		//查询的整条语句用字符串传入
}CmdTable_t;


typedef struct{
	u32 id;			//主键ID
	u32 Wl_Addr;		//物理地址
	u32 Base_Addr;	//协调器地址，不是实际的地址，是映射的地址。
	u32 Coor_gid;		//协调器组ID
	s8 CC_id[16];		//集中器ID
	u32 Map_Addr;	//被映射到Base_Addr的实际地址
}TableCoordi_t;

typedef struct{
	u32 id;			//主键ID
	u32 Wl_Addr;		//物理地址
	u32 Base_Addr;	//单灯地址，不是实际的地址，是映射的地址。
	u32 lt_gid;		//单灯组ID
	u32 Coor_id;		//对应的协调器的地址
	u32 Map_Addr;	//被映射到Base_Addr的实际地址
}TableSingle_t;


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
}TableTask_t;
typedef struct{
	u32 	id;
	u32	Tk_id;		//任务表id
	u32	Rank;		//执行顺序
	u8	Cmd[300];	//任务命令
	u32	Wait_time;	//执行延迟时间
}TableTasklist_t;
typedef struct{
	u32 	id;
	u32	Add_time;	//报警时间
	u32	Type;		//报警类型
	u32	Grade;		//报警等级
	u32	State;		//状态
	u8	Remark[48];	//备注
}Tablewarn_t;

/**
 *  往数据库里插入一条数据
 *  cmd：向数据库的哪张表插入，后面跟的是对应的那张表的结构体数据
 *  返回值：
 *  	成功 返回SUCCESS  失败  FAIL
 *  eg:	Insert_Table(Cmd_coordi, &Coordi);
 *  注意：除了主键id不用赋值，其他成员必须要赋值
 */
extern s32 Insert_Table(u32 cmd, ...);
/**
 *  删除表中的一条
 *  cmd： 对应的表枚举
 *  Condition：删除的条件，字符串， 后面的后面的可变参数，可以根据cmd内容进行扩展
 *  返回值：
 *  	成功   SUCCESS   失败    FAIL
 *  eg： Delete_Table(Cmd_coordi,"where id=2");
 */
extern s32 Delete_Table(u32 cmd,const char *Condition);
/**
 *  更新表中的一个字段
 *  cmd    对应的表枚举
 *  Condition：更新的条件，字符串
 *  返回值：
 *  	成功   SUCCESS   失败    FAIL
 *  eg： Update_Table(Cmd_coordi,"set Wl_Addr=0x88 where id=6");
 */
extern s32 Update_Table(u32 cmd,const char *Condition);
/**
 *  查询表内容
 *  cmd    	对应的表枚举
 *  Column    	查询字段的内容
 *  Condition    	查询的条件
 *  后面跟的是查询内容的结构体，和结构体的大小
 *  eg：		struct {u32 Wl_Addr;u32 Map_Addr;} Res[10];
 *		Select_Table(Cmd_light ,"Wl_Addr,Map_Addr","where id=1",Res,sizeof(Res[0]));
 */
extern s32 Select_Table(u32 cmd,const char *Column,const char*Condition,...);
/**
 * 	查询表内容
 * 	sql 对应的sql语句
 * 	buf 查询出来的内容存储缓冲区
 * 	RowSize 一个数组元素占多少字节
 * 	ColSize 数组元素的个数
 * 	如果包含字符串的话：
 * 		后面的可变参数要输入查询语句包含多少个字符串，紧接着依次写入每个字符串的缓存大小，
 * 		如果没有字符串，可变参数传入0即可
 * 	成功返回SUCCESS ，失败返回FAIL
 * eg:	struct {int CoordiAddr;int SingleAddr;}buf[1000];
 * 	Select_Table_V2( sql,  (char*)buf,   sizeof(buf[0]),   sizeof(buf)/sizeof(buf[0]),  0);
 */
extern s32 Select_Table_V2(const char *sql, char *buf,int RowSize,int ColSize,...);

#else

#define DATABASE_PATH "./my.db"
typedef struct {
	sqlite3 *db;
	s32 col_num;	//查询结果的字段数目
	s32 col_row;	//查询出来的条目
	s8* err_msg;	//错误信息
	s8** result; 	//查询的结果
} Sql_Resource;

extern Sql_Resource sql;
extern s32 DatabaseClose(void);
extern s32 DatabaseInit(s8 *DB_PathName);
extern s32 DatabaseRead(s8* sql_string,  void* result);
extern s32 DatabaseWrite(s8* sql_string);
#endif


#endif
