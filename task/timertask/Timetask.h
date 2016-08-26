/********************************************************************
	> File Name:	Timetask.h
	> Author:	Austzhu
	> Mail:		153462902@qq.com
	> Created Time:	2016年06月20日 星期一 09时43分50秒
 *******************************************************************/
#ifndef __Timetask_h__
#define __Timetask_h__
#include "include.h"
#include "database.h"
#include "tasklist.h"
#define Task_CmdLen 	128
typedef struct {
	char 	name[12];
	time_t 	RunTime;
	s32 	Task_State;
}Task_Table; 

typedef struct{
	s8 	Task_Name[StringSize];
	s32 	Task_Priority;			//定时任务的优先级
	time_t 	Task_Start;
	time_t 	Task_End;
	time_t 	Task_Run;			//任务的运行时间
	u32 	Task_Inter;			//执行任务间隔，单位秒
	u32 	Task_Loop;			//是否循环任务
	u32 	Task_CMDLen;			//执行的命令数目
	u8 	Task_Cmd[Task_CmdLen];
}Timetask_Format;
extern int TaskBusy;

extern int TimetaskInsertSQL(u8 *Package, int nLen);
extern int InsertSqlite3(Timetask_Format *Timetask);
extern int SearchAndStartTimetask(void);
extern void InitTimeTASK(void);
#endif
