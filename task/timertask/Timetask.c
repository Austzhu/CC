/********************************************************************
	> File Name:	Timetask.c
	> Author:	Austzhu
	> Mail:		153462902@qq.com
	> Created Time:	2016年06月20日 星期一 09时43分41秒
 *******************************************************************/
#include "Timetask.h"

static s32 ReadyTaskID = -1;	//保存即将执行的任务ID
int  TaskBusy;
int  Timetask_Pending;
int  AppendTimeTask2Queue(PureCmdArry *task)
{
	int Queue = -1;
	if(!WhetherCMDExist(GPRS_TYPE_QUEUE,task->ctrl)){
		if( TaskGenerateAndAppend(GPRS_TYPE_QUEUE,(u8*)task,NET_TASK,TASK_LEVEL_GPRS)){
			Queue = GPRS_TYPE_QUEUE;
			goto err;
		}
	}else if(!WhetherCMDExist(CC_TYPE_QUEUE,task->ctrl)){
		if( TaskGenerateAndAppend(CC_TYPE_QUEUE,(u8*)task,NET_TASK,TASK_LEVEL_CC)	){
			Queue = CC_TYPE_QUEUE;
			goto err;
		}
	}else if(!WhetherCMDExist(DEVICE485_TYPE_QUEUE,task->ctrl)){
		if( TaskGenerateAndAppend(DEVICE485_TYPE_QUEUE,(u8*)task,NET_TASK,TASK_LEVEL_485)){
			Queue = DEVICE485_TYPE_QUEUE;
			goto err;
		}
	}else if(!WhetherCMDExist(ETH_NET_TYPE_QUEUE,task->ctrl)){
		if( TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,(u8*)task,NET_TASK,TASK_LEVEL_NET)){
			Queue = ETH_NET_TYPE_QUEUE;
			goto err;
		}
	}

	return SUCCESS;
 err:
	debug(DEBUG_Timetask,"Time task Append to %d fail!\n",Queue);
	return FAIL;

}


int ExecTimetask(TableTasklist_t *Ptask ,int nLen)
{

	int j=0;
	char Taskcmd[200];
	//sleep(3);
	/* 遍历每个tastlist表 */
	while(nLen--){
		memset(Taskcmd,0,sizeof(Taskcmd));
		debug(DEBUG_Timetask,"^^^^^^EXEC cmd:%s\n",Ptask->Cmd);
		StrToHex((u8*)Taskcmd,  Ptask->Cmd,  strlen((char*)Ptask->Cmd)/2);

		debug(DEBUG_Timetask,"Cmd Data:");
		j=0; while(j < strlen((char*)Ptask->Cmd)/2){
			debug(DEBUG_Timetask,"%02x ",Taskcmd[j++]);
		}debug(DEBUG_Timetask,"\n");

		AppendTimeTask2Queue((PureCmdArry*)Taskcmd);
		++Ptask;
	}
	return SUCCESS;
}

void Signal_Alarm(int signo)
{
	TableTasklist_t 	Task_list[32];
	char sql[128];
	int i = 0;
	if(TaskBusy){
		Timetask_Pending = !0;//挂起定时任务标记
		return;
	}
	if(ReadyTaskID != -1){
		memset(Task_list,0,sizeof(Task_list));
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select * from db_tasklist where Tk_id=%d ;",ReadyTaskID);
		Select_Table_V2( sql,  (char*)&Task_list,   sizeof(Task_list[0]),  sizeof(Task_list)/sizeof(Task_list[0]),  1, sizeof(Task_list[0].Cmd) );
		// /* 计算有多少条任务 */
		i=0;while(Task_list[i].Cmd[0] != 0 && i < sizeof(Task_list)/sizeof(Task_list[0]) ){++i;};
		 debug(DEBUG_Timetask,"The num of tasklist cmd %d\n",i);
		/* 执行任务指令 */
		if( SUCCESS == ExecTimetask(Task_list,i)){
			/* 更新任务表中的状态 */
			memset(sql,0,sizeof(sql));
			sprintf(sql,"set State = 0 where id=%d ",ReadyTaskID);
			debug(DEBUG_Timetask,"sql:%s\n",sql);
			Update_Table(Cmd_task,sql);
		}else{//执行失败
			err_Print(DEBUG_Timetask,"Do ExecTimetask Fail!\n");
		}
		if(SUCCESS != SearchAndStartTimetask()){ alarm(5); }
	}else{//没有找到任务id
		debug(DEBUG_Timetask,"Have no TaskID was Ready!\n");
	}
}

void Signal_User1(int signo)
{
	if(Timetask_Pending){
		Timetask_Pending = 0;
		debug(DEBUG_Timetask,"while do time task after 1s\n");
		alarm(1);
	}
}

void InitTimeTASK(void)
{
	signal(SIGALRM,Signal_Alarm);
	signal(SIGUSR1,Signal_User1);
	SearchAndStartTimetask();
}
int SearchAndStartTimetask(void)
{
	TableTask_t Task;
	char sql[128];
	time_t Current_time;
	s32 AlamSec;
	ReadyTaskID = -1;
	memset(&Task,0,sizeof(Task));
	debug(DEBUG_Timetask,"\n*****Start of %s*****\n",__func__);
	if( SUCCESS != Select_Table_V2("select * from db_task order by Run_Time,Priority",(char*)&Task, sizeof(Task),1,1,sizeof(Task.Name)) ){
		err_Print(DEBUG_Timetask,"select task fail!\n");
		goto ERR;
	}
	//数据库中没有找到可以执行的任务,定时任务将会终止，直到有新的时间任务加入数据库
	if(Task.Start_Date == 0){
		debug(DEBUG_Timetask,"No can perform tasks in the database\n");
		return SUCCESS;
	 }
	debug(DEBUG_Timetask,"task id:%d\n",Task.id);
	debug(DEBUG_Timetask,"task name:%s\n",Task.Name);
	debug(DEBUG_Timetask,"task Priority:%d\n",Task.Priority);
	debug(DEBUG_Timetask,"task Start_Date:0x%x\n",(u32)Task.Start_Date);
	debug(DEBUG_Timetask,"task End_Date:0x%x\n",(u32)Task.End_Date);
	debug(DEBUG_Timetask,"task Run_Time:0x%x\n",(u32)Task.Run_Time);
	debug(DEBUG_Timetask,"task Inter_Time:%d\n",Task.Inter_Time);
	debug(DEBUG_Timetask,"task Type:%d\n",Task.Type);
	debug(DEBUG_Timetask,"task State:%d\n",Task.State);

	if(Task.Run_Time < Task.End_Date ){
		if(2 == Task.Type){//循环任务，执行
			time(&Current_time);
			AlamSec = Task.Run_Time - Current_time;
			debug(DEBUG_Timetask,"AlamSec=%d\n",AlamSec);
			alarm(AlamSec <=0?5:AlamSec);
			ReadyTaskID = Task.id;
			do{//更新下次运行的时间
				Task.Run_Time += Task.Inter_Time;
			}while(Task.Run_Time < Current_time);
			//更新数据库的运行时间
			memset(sql,0,sizeof(sql));
			sprintf(sql,"set Run_Time=%d where id=%d",(u32)Task.Run_Time,Task.id);
			if(SUCCESS != Update_Table(Cmd_task,sql) ){
				err_Print(DEBUG_Timetask,"Update sql err!\n");
				goto ERR;
			}

		}else{//单次任务
			if(Task.State == 1){//任务没有被执行过，执行
				time(&Current_time);
				AlamSec = Task.Run_Time - Current_time;
				alarm(AlamSec <=0?5:AlamSec);
				ReadyTaskID = Task.id;
			}else if(Task.State == 0){//执行过的单次任务，删除
				memset(sql,0,sizeof(sql));
				sprintf(sql,"where id=%d",Task.id);
				Delete_Table(Cmd_task,sql);
			}else{//正在执行的任务
				/* do nothing */
			}
		}
	}else{//任务过期了，删除
		memset(sql,0,sizeof(sql));
		sprintf(sql,"where id=%d",Task.id);
		Delete_Table(Cmd_task,sql);
	}

	return SUCCESS;
ERR:
	return FAIL;
}

int InsertSqlite3(Timetask_Format *Timetask)
{
	TableTask_t 	Task;
	TableTasklist_t	Tasklist;
	int 		i = 0;
	char 		sql[128];
	PureCmdArry 	*tasklist_pkg = NULL;
	if(!Timetask){
		err_Print(DEBUG_Timetask,"Timetask is null!\n");
		return Pointer_Err;
	}
	memset(&Task,0,sizeof(Task));
	memset(&Tasklist,0,sizeof(Tasklist));
	memcpy(Task.Name,Timetask->Task_Name,sizeof(Timetask->Task_Name));
	Task.Priority 	= Timetask->Task_Priority;
	Task.Start_Date = Timetask->Task_Start;
	Task.End_Date	= Timetask->Task_End;
	Task.Run_Time	= Timetask->Task_Run;
	Task.Inter_Time= Timetask->Task_Inter;
	Task.Type	= Timetask->Task_Loop;
	Task.State 	= 1;	//0:已经接受，1:即将执行，2:正在执行
	if(SUCCESS != Insert_Table(Cmd_task,&Task)){
		err_Print(DEBUG_Timetask,"Insert Task to sqlite3 failed!\n");
		return FAIL;
	}
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select id from db_task where Name='%s';",Task.Name);
	if(SUCCESS != Select_Table_V2(sql,(char*)&Tasklist.Tk_id, sizeof(Tasklist.Tk_id),1,0) ){
		err_Print(DEBUG_Timetask,"Get Task Id err!\n");
		memset(sql,0,sizeof(sql));
		sprintf(sql,"where Name='%s';",Task.Name);
		Delete_Table(Cmd_task,sql);
		return FAIL;
	}
	debug(DEBUG_Timetask,"TK_id = %d\n",Tasklist.Tk_id);

	tasklist_pkg = (PureCmdArry*)Timetask->Task_Cmd;
	i = 0;
	while( ((u8*)tasklist_pkg - Timetask->Task_Cmd)  < Timetask->Task_CMDLen){
		Tasklist.Rank 		= i++;
		Tasklist.Wait_time 	= 0;

		if(tasklist_pkg->len > sizeof(Tasklist.Cmd)/2){
			err_Print(DEBUG_Timetask,"tasklist cmd too long!\n");
			/* 删除之前插入的内容 */
			memset(sql,0,sizeof(sql));
			sprintf(sql,"where Name='%s';",Task.Name);
			Delete_Table(Cmd_task,sql);
			return FAIL;
		}
		/* 把命令的16进制数转化成字符 */
		HexToStr_v3(Tasklist.Cmd,&tasklist_pkg->ctrl, tasklist_pkg->len+2);
		debug(DEBUG_Timetask,"Tasklist.Cmd: %s\n",Tasklist.Cmd);
		/* 插入数据库中 */
		Insert_Table(Cmd_tasklist,&Tasklist);
		/* 指向下一条命令 */
		tasklist_pkg = (PureCmdArry*)((char*)tasklist_pkg+tasklist_pkg->len + 2);
	}
	return SUCCESS;
}

int TimetaskInsertSQL(u8 *Package, int nLen)
{
	Timetask_Format Timetask;
	struct tm tt;
	int ii = 0;
	if(!Package){
		err_Print(DEBUG_Timetask,"Package is null!\n");
		return Pointer_Err;
	}
	if(nLen != *(Package+56) + 58){
		debug(DEBUG_Timetask,"Packge length err!\n");
		debug(DEBUG_Timetask,"package len %d cmd len %d\n",nLen,*(Package+56));
		return FAIL;
	}

	memset(&Timetask,0,sizeof(Timetask));
	memcpy(Timetask.Task_Name,Package,sizeof(Timetask.Task_Name));
	debug(DEBUG_Timetask,"Task name:%s\n",Timetask.Task_Name);
	Package += sizeof(Timetask.Task_Name);
	Timetask.Task_Priority = *Package++;
	debug(DEBUG_Timetask,"Task Priority:%d\n",Timetask.Task_Priority );
	memset(&tt,0,sizeof(tt));
	tt.tm_year 	= 100 + *Package++;
	tt.tm_mon 	= *Package++ - 1;
	tt.tm_mday 	= *Package++;
	tt.tm_hour	= *Package++;
	tt.tm_min	= *Package++;
	tt.tm_sec	= *Package++;
	Timetask.Task_Start = mktime(&tt);
	debug(DEBUG_Timetask,"time start:%08x\n",(u32)Timetask.Task_Start);
	memset(&tt,0,sizeof(tt));
	tt.tm_year 	= 100 + *Package++;
	tt.tm_mon 	= *Package++ - 1;
	tt.tm_mday 	= *Package++;
	tt.tm_hour	= *Package++;
	tt.tm_min	= *Package++;
	tt.tm_sec	= *Package++;
	Timetask.Task_End = mktime(&tt);
	debug(DEBUG_Timetask,"time end:%08x\n",(u32)Timetask.Task_End);
	memset(&tt,0,sizeof(tt));
	tt.tm_year 	= 100 + *Package++;
	tt.tm_mon 	= *Package++ - 1;
	tt.tm_mday 	= *Package++;
	tt.tm_hour	= *Package++;
	tt.tm_min	= *Package++;
	tt.tm_sec	= *Package++;
	Timetask.Task_Run = mktime(&tt);
	debug(DEBUG_Timetask,"time Run:%08x\n",(u32)Timetask.Task_Run);

	ii = 0;	while(ii<4){ Timetask.Task_Inter |= *(Package+ii) << (8*(3-ii)); ++ii;} Package += 4;
	debug(DEBUG_Timetask,"Task Inter:%x\n",Timetask.Task_Inter);

	Timetask.Task_Loop = *Package++;
	Timetask.Task_CMDLen = *Package++;
	memcpy(Timetask.Task_Cmd,Package,Timetask.Task_CMDLen);
	ii = 0;
	while(ii < Timetask.Task_CMDLen){
		debug(DEBUG_Timetask,"%02x ",Timetask.Task_Cmd[ii++]);
	}debug(DEBUG_Timetask,"\n");
	/* 向数据库中写定时任务 */
	return InsertSqlite3(&Timetask) == SUCCESS ? SearchAndStartTimetask() : FAIL;
}
