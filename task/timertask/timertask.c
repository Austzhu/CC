/******************************************************************
** 文件名:
** Copyright (c) 1998-1999 *********公司技术开发部
** 创建人:
** 日　期:
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:
*******************************************************************/
#include "timertask.h"
#include "cmd_process.h"

#define NO_SUCH_TASKID 		0
#define ONE_DAY_SECS 		24*60*60
#define ONE_HOUR_MIN 		60
#define ONE_HOUR_SEC 		60*60
#define ONE_MINSECS 			60
#define TimerTaskFlagUsed 		1
#define TimerTaskFlagUNUsed 		0
#define TimerTaskFlagChanged		1
#define TimerTaskFlagUNChanged	0
extern volatile CHAR TOPSHOWPERTICK;

struct TOPTimerTaskListStruct TOPTimerTaskList;

SINT TimerTaskTimeUpFuntion(void *node,SINT In);


SINT TimerTaskIDSearch(UCHAR *taskID)
{
	SINT Task_i=1;

	TimerTaskToSaveFormatStruct *PtimerTaskSearch = TimerTaskList;
	debug(DEBUG_LOCAL_TIME_TASK,"***#####in TimerTaskIDSearch\n");
		
		
	if(!taskID){
		debug(DEBUG_LOCAL_TIME_TASK,"######@@@@@@@@@@!taskID\n");
	
		return NO_SUCH_TASKID;
	}

	for(Task_i = 1;Task_i<(TimerTaskQuatityMax+1);Task_i++){
		debug(DEBUG_LOCAL_TIME_TASK,"taskIDDDDD ####IS %x :%x\n",taskID[0],taskID[1]);
		debug(DEBUG_LOCAL_TIME_TASK,"PtimerTaskSearch->TaskID is $$$$ %x %x\n",PtimerTaskSearch->TaskID[0],PtimerTaskSearch->TaskID[1]);
		

		if(!bcmp(taskID,PtimerTaskSearch->TaskID,2)){
			printf("task_i is %d\n",Task_i);
			return Task_i;
		}
		PtimerTaskSearch++;

	}
	return NO_SUCH_TASKID;
}

SINT TimerTaskFormatChang2SaveFormat(struct TimerTaskToSaveFormatStruct *PtimetaskInsert,struct ServerCommunTimerTaskStruct *Taskstruget)
{
	time_t TimeStart,TimeEnd;		//task 有效期
	SINT TaskTimeOffset;
	TIME_SHORT_FORMAT TimeExch;
	struct tm t={0};
	if(!PtimetaskInsert|| !Taskstruget) return FAIL;

	//time format change for start 
	t.tm_sec = 0;
	t.tm_min = 0;
	t.tm_hour = 0;
	t.tm_mday = Taskstruget->TaskStartDay;
	if(Taskstruget->TaskStartMonth > 12) Taskstruget->TaskStartMonth = 12;
	if(Taskstruget->TaskStartMonth < 1) Taskstruget->TaskStartMonth = 1;
	
	/*LINUX TIME MONTH 0---11*/
	/*WONDOW TIME MONTH 1---12*/
	t.tm_mon = (Taskstruget->TaskStartMonth -1);
	t.tm_year = 100 + Taskstruget->TaskStartYear;
	debug(DEBUG_LOCAL_TIME_TASK,"*********start date nianyueri is %d:%d:%d: shifenmia0:%d%d%d Taskstruget->TaskStartYearis %d\n",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec,Taskstruget->TaskStartYear);

	TimeStart = mktime(&t);

	//time format change for end
	t.tm_sec = 59;
	t.tm_min = 59;
	t.tm_hour = 23;

	if((0 == (Taskstruget->TaskEndDay))&&(0 == (Taskstruget->TaskEndMonth))&&(0 == (Taskstruget->TaskEndYear))){
		TimeEnd = 0;
	}else{
			t.tm_mday = Taskstruget->TaskEndDay;
			if(Taskstruget->TaskEndMonth > 12){Taskstruget->TaskEndMonth = 12;}
			if(Taskstruget->TaskEndMonth < 1){Taskstruget->TaskEndMonth = 1;}
			/*LINUX TIME MONTH 0---11*/
			/*WONDOW TIME MONTH 1---12*/
			t.tm_mon = (Taskstruget->TaskEndMonth - 1);
			t.tm_year = 100+ Taskstruget->TaskEndYear;

			TimeEnd = mktime(&t);

			if(TimeEnd < TimeStart){
				TimeEnd = (TimeStart + ONE_DAY_SECS -1);
			}
	}

	if(TimerTaskOprate_Once == Taskstruget->CycleType){
		TimeEnd = (TimeStart + ONE_DAY_SECS -1);
	}
	debug(DEBUG_LOCAL_TIME_TASK,"*********end date is %d:%d:%d\n",t.tm_year,t.tm_mon,t.tm_mday);

	

	memcpy((UCHAR *)&PtimetaskInsert->TaskID[0],(UCHAR *)&Taskstruget->TaskID[0],2);
	debug(DEBUG_LOCAL_TIME_TASK,"*********taskID is %d:%d\n",PtimetaskInsert->TaskID[0],PtimetaskInsert->TaskID[1]);
	

	

	PtimetaskInsert->TaskType = Taskstruget->TaskType;		//线控  单灯
	PtimetaskInsert->ControlMode = Taskstruget->ControlMode;
	PtimetaskInsert->CycleType = Taskstruget->CycleType;
	PtimetaskInsert->OpraType= Taskstruget->OpraType;
	
	PtimetaskInsert->TaskBegin = TimeStart;
	PtimetaskInsert->TaskEnd = TimeEnd;
	PtimetaskInsert->TaskStartEvryDay= Taskstruget->RelatEffecStartDay;
	PtimetaskInsert->TaskStartEvryMonth= Taskstruget->RelatEffecStartMonth;

	PtimetaskInsert->TaskEndEvryDay = Taskstruget->RelatEffecEndDay;
	PtimetaskInsert->TaskEndEvryMonth = Taskstruget->RelatEffecStartMonth;

	PtimetaskInsert->WeekDay= Taskstruget->WeekDay;
		//大小端检测  :视情况而定
		//TimeExch.TimeS[0] = Taskstruget->Interval_HI;
		//TimeExch.TimeS[1] = Taskstruget->Interval_LOW;

		TimeExch.TimeS[0] = Taskstruget->Interval_LOW;
		TimeExch.TimeS[1] = Taskstruget->Interval_HI;
	PtimetaskInsert->TaskOpratInteral = TimeExch.TimeL;

	memcpy((UCHAR *)&PtimetaskInsert->Reserve,(UCHAR *)&Taskstruget->Reserve,4);


	TaskTimeOffset = (SINT)(Taskstruget->TaskExecuteHour) * ONE_HOUR_MIN + Taskstruget->TaskExecuteMinute;
	PtimetaskInsert->OpratTimeOffset = TaskTimeOffset;
	
	PtimetaskInsert->TaskCMDLen = Taskstruget->TrasfCMDPakectLen;
	memcpy((UCHAR *)&PtimetaskInsert->TaskCMDPakt[0],(UCHAR *)&Taskstruget->TrasfCMDPakect[0],Taskstruget->TrasfCMDPakectLen);

	debug(DEBUG_LOCAL_TIME_TASK,"*********TaskCMDPakt is %d:%d:%d:%d:%d:%d:%d\n",PtimetaskInsert->TaskCMDPakt[0],PtimetaskInsert->TaskCMDPakt[1],
					PtimetaskInsert->TaskCMDPakt[2],PtimetaskInsert->TaskCMDPakt[3],
					PtimetaskInsert->TaskCMDPakt[4],PtimetaskInsert->TaskCMDPakt[5],
					PtimetaskInsert->TaskCMDPakt[6]);

	

	#if DEBUG_LOCAL_TIME_TASK
		printf("PtimetaskInsert->TaskType %d,Taskstruget->TaskType %d\n\n",PtimetaskInsert->TaskType,Taskstruget->TaskType);
		printf("TIME TASK START FROM %ld\n ,END AT %ld\n",TimeStart,TimeEnd);
		printf("PtimetaskInsert->TaskCMDLen = %d Taskstruget->TrasfCMDPakectLen =%d\n",PtimetaskInsert->TaskCMDLen,Taskstruget->TrasfCMDPakectLen);
		printf("PtimetaskInsert->OpratTimeOffset = %d TaskTimeOffset =%d\n",PtimetaskInsert->OpratTimeOffset,TaskTimeOffset);
		printf("PtimetaskInsert->WeekDay= %d Taskstruget->WeekDay =%d\n",PtimetaskInsert->WeekDay,Taskstruget->WeekDay);
		printf("PtimetaskInsert->TaskOpratInteral = %d,Taskstruget->Interval_HI Taskstruget->Interval_LOW %d %d\n",PtimetaskInsert->TaskOpratInteral,Taskstruget->Interval_HI,Taskstruget->Interval_LOW);

		printf("PtimetaskInsert->TaskEndEvryDay = %d Taskstruget->RelatEffecEndDay =%d\n",PtimetaskInsert->TaskEndEvryDay,Taskstruget->RelatEffecEndDay);
		printf("PtimetaskInsert->TaskEndEvryMonth = %d Taskstruget->TaskEndEvryMonth =%d\n",PtimetaskInsert->TaskEndEvryMonth,Taskstruget->RelatEffecEndMonth);
		printf("PtimetaskInsert->TaskStartEvryDay = %d Taskstruget->TaskStartEvryDay =%d\n",PtimetaskInsert->TaskStartEvryDay,Taskstruget->RelatEffecStartDay);
		printf("PtimetaskInsert->TaskStartEvryMonth = %d Taskstruget->TaskStartEvryMonth =%d\n",PtimetaskInsert->TaskStartEvryMonth,Taskstruget->RelatEffecStartMonth);

		printf("PtimetaskInsert->TaskBegin %ld,PtimetaskInsert->TaskEnd %ld\n",PtimetaskInsert->TaskBegin,PtimetaskInsert->TaskEnd);
		printf("PtimetaskInsert->ControlMode %d Taskstruget->ControlMode %d\n",PtimetaskInsert->ControlMode,Taskstruget->ControlMode);
		printf("PtimetaskInsert->CycleType %d Taskstruget->CycleType is %d\n\n",PtimetaskInsert->CycleType,Taskstruget->CycleType);
		printf("PtimetaskInsert->OpraType %d Taskstruget->OpraType is %d\n\n",PtimetaskInsert->OpraType,Taskstruget->OpraType);

	#endif	//end of if(DEBUG_LOCAL_TIME_TASK){

	return SUCCESS;

}

SINT TimerTaskMarkSet(SINT taskIDOffs)
{
	struct TimerRelevTaskIDUnionStruct *P_TimerRelevTaskIDUnion;

	P_TimerRelevTaskIDUnion =(struct TimerRelevTaskIDUnionStruct *)&(TOPTimerTaskList.TimerTaskNodes);
	
	pthread_mutex_lock(&(TOPTimerTaskList.lock));

	if((taskIDOffs < TimerTaskQuatityMax)&&(taskIDOffs >= 0)){
		//(P_TimerRelevTaskIDUnion+taskIDOffs)->
		(P_TimerRelevTaskIDUnion+taskIDOffs)->UsedFlag = TimerTaskFlagUsed;
		(P_TimerRelevTaskIDUnion+taskIDOffs)->ChangeFlag = TimerTaskFlagChanged;
	}
	pthread_mutex_unlock(&(TOPTimerTaskList.lock));

	debug(DEBUG_LOCAL_TIME_TASK,"in fuction TimerTaskMarkSet\n");
	debug(DEBUG_LOCAL_TIME_TASK,"(P_TimerRelevTaskIDUnion+taskIDOffs)->UsedFlag is %d\n",((P_TimerRelevTaskIDUnion+taskIDOffs)->UsedFlag));
	debug(DEBUG_LOCAL_TIME_TASK,"(P_TimerRelevTaskIDUnion+taskIDOffs)->ChangeFlag is %d\n",((P_TimerRelevTaskIDUnion+taskIDOffs)->ChangeFlag));
	debug(DEBUG_LOCAL_TIME_TASK,"P_TimerRelevTaskIDUnion.TaskIDOffs is %d\n",P_TimerRelevTaskIDUnion->TaskIDOffs);

	
	return SUCCESS;
}

SINT TaskElseControlMdMarkSet(SINT taskID)
{
	debug(DEBUG_LOCAL_TIME_TASK,"in fuction TaskElseControlMdMarkSet\n");
	return SUCCESS;
}


SINT TimerTaskSetProc(UCHAR *buf,UCHAR len)
{

	
	
	SINT TaskID_Exist = 0;
	//ok	struct ServerCommunTimerTaskStruct *TimerTaskGet = (struct ServerCommunTimerTaskStruct *)(buf+1);
	struct ServerCommunTimerTaskStruct *TimerTaskGet = (struct ServerCommunTimerTaskStruct *)buf;
	TimerTaskToSaveFormatStruct *currentTimetaskInsert = TimerTaskList;	

	#if DEBUG_LOCAL_TIME_TASK
		printf("taskID get is %x%x,TaskType :%x  ControlMode:%x CycleType: %x  OpraType:%x\n",TimerTaskGet->TaskID[0],TimerTaskGet->TaskID[1],TimerTaskGet->TaskType,
			TimerTaskGet->ControlMode,TimerTaskGet->CycleType,TimerTaskGet->OpraType);
		printf("TimerTaskGet.TaskStartDay: %d,TimerTaskGet.TaskStartMonth : %x,TimerTaskGet.TaskStartYear is %x\n",TimerTaskGet->TaskStartDay,TimerTaskGet->TaskStartMonth,TimerTaskGet->TaskStartYear);

		printf("TimerTaskGet.TaskEndDay %x,TimerTaskGet.TaskEndMonth %x,TimerTaskGet.TaskEndYear %x\n",TimerTaskGet->TaskEndDay,TimerTaskGet->TaskEndMonth,TimerTaskGet->TaskEndYear);
		printf("TimerTaskGet.RelatEffecStartDay is %x,TimerTaskGet.RelatEffecStartMonth is %x,TimerTaskGet.RelatEffecEndDay is %x,TimerTaskGet.RelatEffecEndMonth is %x\n",TimerTaskGet->RelatEffecStartDay,TimerTaskGet->RelatEffecStartMonth,TimerTaskGet->RelatEffecEndDay,TimerTaskGet->RelatEffecEndMonth);
		printf("TimerTaskGet.WeekDay is %x,TimerTaskGet.Interval_LOW is %x,TimerTaskGet.Interval_HI is %x,TimerTaskGet.TaskExecuteHour is %x,TimerTaskGet.TaskExecuteMinute is %x\n",TimerTaskGet->WeekDay,TimerTaskGet->Interval_LOW,TimerTaskGet->Interval_HI,TimerTaskGet->TaskExecuteHour,TimerTaskGet->TaskExecuteMinute);
		printf("TimerTaskGet.TrasfCMDPakectLen is %x,TimerTaskGet.TrasfCMDPakect[0] is %x,TimerTaskGet.TrasfCMDPakect[1] is %x\n",TimerTaskGet->TrasfCMDPakectLen,TimerTaskGet->TrasfCMDPakect[0],TimerTaskGet->TrasfCMDPakect[1]);
	#endif
	

	if(NULL == buf){
		
		debug(DEBUG_LOCAL_TIME_TASK,"NULL == buf 2012-05-23\n");
		return FAIL;
	}
	
	
	debug(DEBUG_LOCAL_TIME_TASK,"in function case TimerTaskSetProc: 2012-05-23\n");
	
			
	//TimeTaskIDSearch return ID, 0:no such task;!0--task ID
	TaskID_Exist = TimerTaskIDSearch(TimerTaskGet->TaskID);
	debug(DEBUG_LOCAL_TIME_TASK,"#############TaskID_Exist is %d\n",TaskID_Exist);

	if(TaskID_Exist){	//this task has exist to update
		if(TimerTaskFormatChang2SaveFormat((currentTimetaskInsert+TaskID_Exist-1/*offset bcas 0 is noval*/),TimerTaskGet)){
			return TIMER_TASK_INSERT_ERR;
		}
		if(TimerTaskMarkSet(TaskID_Exist-1)){
			return TIMER_TASK_TIMERSET_ERR;
		}
	}else{		//no such taskid to insert
		for(TaskID_Exist = 0;TaskID_Exist<TimerTaskQuatityMax;TaskID_Exist++){
			if(currentTimetaskInsert->TaskBegin == 0){		//task deleted or invalid
				if(TimerTaskFormatChang2SaveFormat(currentTimetaskInsert,TimerTaskGet)){
					return TIMER_TASK_INSERT_ERR;
				}

				if(currentTimetaskInsert->ControlMode == TASK_CONTROL_TYPE_TIME){
					//SET timerSetMark
					debug(DEBUG_LOCAL_TIME_TASK,"the task ControlMode is :TASK_CONTROL_TYPE_TIME\n");
					if(TimerTaskMarkSet(TaskID_Exist)){
						return TIMER_TASK_TIMERSET_ERR;
					}
				}else{
					//SET SetMark
					debug(DEBUG_LOCAL_TIME_TASK,"the task ControlMode is not TASK_CONTROL_TYPE_TIME\n");
					if(TaskElseControlMdMarkSet(TaskID_Exist)){
						return TASK_ELSEMODESET_ERR;
					}
				}
				break;
			}
			++currentTimetaskInsert;
		}
	}	
	return SUCCESS;
}


SINT TimerEngineInit(void)
{
	TOPTimerTaskList.PreFuction = signal(SIGALRM,TimerTaskPerTickfuntion);

	TOPTimerTaskList.NowFuction = TimerTaskPerTickfuntion;

	//3 SETtimer

	TOPTimerTaskList.NowValue.it_value.tv_sec = 1;
	TOPTimerTaskList.NowValue.it_value.tv_usec = 0;

	TOPTimerTaskList.NowValue.it_interval.tv_sec = 1;
	TOPTimerTaskList.NowValue.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL,&TOPTimerTaskList.NowValue,&TOPTimerTaskList.PreValue);

	TOPTimerTaskList.CallBackFunc = TimerTaskTimeUpFuntion;

	return SUCCESS;
}

SINT TimerTaskTimeUpFuntion(void *node,SINT In)
{
	
	debug(DEBUG_LOCAL_TIME_TASK,"time up up ***********************\n");
	struct TimerTaskToSaveFormatStruct *P_timerTaskAppend =&TimerTaskList[In];

	printf("P_timerTaskAppend->TaskCMDPakt : %x:%x:%x:%x:%x:%x:%x\n",P_timerTaskAppend->TaskCMDPakt[0],
		P_timerTaskAppend->TaskCMDPakt[1],P_timerTaskAppend->TaskCMDPakt[2],P_timerTaskAppend->TaskCMDPakt[3],
			P_timerTaskAppend->TaskCMDPakt[4],P_timerTaskAppend->TaskCMDPakt[5],P_timerTaskAppend->TaskCMDPakt[6]);

	if(!WhetherCMDExist(GPRS_TYPE_QUEUE,P_timerTaskAppend->TaskCMDPakt[0])){
		TaskGenerateAndAppend(GPRS_TYPE_QUEUE,&P_timerTaskAppend->TaskCMDPakt[0],NET_TASK,TASK_LEVEL_GPRS);
	}else if(!WhetherCMDExist(CC_TYPE_QUEUE,P_timerTaskAppend->TaskCMDPakt[0])){
		TaskGenerateAndAppend(CC_TYPE_QUEUE,&P_timerTaskAppend->TaskCMDPakt[0],NET_TASK,TASK_LEVEL_CC);
	}else if(!WhetherCMDExist(DEVICE485_TYPE_QUEUE,P_timerTaskAppend->TaskCMDPakt[0])){
		TaskGenerateAndAppend(DEVICE485_TYPE_QUEUE,&P_timerTaskAppend->TaskCMDPakt[0],NET_TASK,TASK_LEVEL_485);
	}else if(!WhetherCMDExist(ETH_NET_TYPE_QUEUE,P_timerTaskAppend->TaskCMDPakt[0])){
		TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,&P_timerTaskAppend->TaskCMDPakt[0],NET_TASK,TASK_LEVEL_NET);
	}	//end of else if
	return SUCCESS;
}

void TimerTaskPerTickfuntion(SINT signol)
{
	static time_t CurrentSigTime = 0;
	static time_t PreviousSigTime =0;
	time_t	TempSignIntervElaps;
	SINT CircleTimes;
	struct TimerRelevTaskIDUnionStruct *P_TaskIDNode;

	P_TaskIDNode = TOPTimerTaskList.TimerTaskNodes;
	time(&CurrentSigTime);
	if(0 == PreviousSigTime){
		TempSignIntervElaps = 1;
	}else{
		
		TempSignIntervElaps = CurrentSigTime - PreviousSigTime;
	}
	/* time change*/
	PreviousSigTime = CurrentSigTime;
	//perticket change once
 	TOPSHOWPERTICK = ~TOPSHOWPERTICK;
	
	/*if node is exist*/
	if(P_TaskIDNode){	//node exist
		/*add lock to occupation*/
		pthread_mutex_lock(&TOPTimerTaskList.lock);
		/*time is changed ?*/
		if(TOPTimerTaskList.TimerChangeFlag){
			printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);
		
		}else{
			
			P_TaskIDNode= TOPTimerTaskList.TimerTaskNodes;
			for(CircleTimes = 0;CircleTimes<TimerTaskQuatityMax;CircleTimes++){
				if(P_TaskIDNode->UsedFlag){
					printf("in if(P_TaskIDNode->UsedFlag){:CircleTimes is %d\n\n",CircleTimes);
					printf("P_TaskIDNode->TimeElapse is %d,P_TaskIDNode->TimeInterval is %d,P_TaskIDNode->ChangeFlag is %x\n",P_TaskIDNode->TimeElapse,P_TaskIDNode->TimeInterval,P_TaskIDNode->ChangeFlag);
					printf("P_TaskIDNode->TaskIDOffs is %x,P_TaskIDNode->UsedFlag is %x \n",P_TaskIDNode->TaskIDOffs,P_TaskIDNode->UsedFlag);

					/*update the time that elaps*/
					P_TaskIDNode->TimeElapse += TempSignIntervElaps;
					
					//test P_TaskIDNode->TimeElapse += 3600;
					/*if first init or new task isert*/
					if(P_TaskIDNode->ChangeFlag){
						printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);
						TimerIntervalForTaskNodeCal(P_TaskIDNode,CircleTimes);
					}
					/*if time is up*/
					if(P_TaskIDNode->TimeElapse > P_TaskIDNode->TimeInterval){
						TOPTimerTaskList.CallBackFunc(P_TaskIDNode,CircleTimes);
						TimerIntervalForTaskNodeCal(P_TaskIDNode,CircleTimes);
					}
					
				}//end ofif(P_TaskIDNode->UsedFlag){
				/*P_TaskIDNode point to next*/
				P_TaskIDNode++;
			}	//end of for(CircleTimes = 0;
		}//end of else (time is not changed)
		/*unlock the timer list*/
		 pthread_mutex_unlock(&(TOPTimerTaskList.lock));
	
	}else{	/*has no such node*/
		OtherControlMDTimeElapse(TempSignIntervElaps);
	}
}

SINT TimeIntervalGeneralCal(TimerTaskToSaveFormatStruct *P_task)
{
	SINT TimeIntervale = -1;
	time_t CurrentTimeInt;
	//struct tm CurrentTimeStr;
	//SINT DaysPassed;
	
	//struct tm TimeTaskExcu;

	
	//get time
	time(&CurrentTimeInt);
	localtime(&CurrentTimeInt);
	//TimeTaskExcu = *localtime(&P_task->TaskBegin);

	
	debug(DEBUG_LOCAL_TIME_TASK,"P_task->CycleType is %d\n",P_task->CycleType);
	debug(DEBUG_LOCAL_TIME_TASK,"P_task->TaskBegin is %ld CurrentTimeInt is %ld,P_task->TaskEnd is %ld\n",P_task->TaskBegin,CurrentTimeInt,P_task->TaskEnd);
	
	
	//任务在有效期内
	if((P_task->TaskEnd == 0)||(CurrentTimeInt < P_task->TaskEnd)){		//任务仍然有效
		switch(P_task->CycleType){
			case TimerTaskOprate_Dalay:
				//current time acced ,generate next time point 
				//任务开始后
				if(CurrentTimeInt > P_task->TaskBegin){
					// |(begin)---|(opratet)---|(current)---|(end)    @1
					if(CurrentTimeInt % (24*60*60) >= (P_task->OpratTimeOffset * 60)){
						TimeIntervale = (24*60*60 -((CurrentTimeInt%(24*60*60)) - P_task->OpratTimeOffset*60));

						debug(DEBUG_LOCAL_TIME_TASK,"here 1 and TimeIntervale is %d, CurrentTimeInt %% (24*60*60) is %ld , CurrentTimeInt %% 24*60*60 is %ld , P_task->OpratTimeOffset in sec is %d,\n",TimeIntervale,(CurrentTimeInt % (24*60*60)), (CurrentTimeInt % 24*60*60),P_task->OpratTimeOffset*60);
						

						break;
					}
					
					// |(begin)---|(current)---|(opratet)---|(end)    @2
					if(CurrentTimeInt % (24*60*60) < (P_task->OpratTimeOffset* 60)){
						TimeIntervale =(P_task->OpratTimeOffset*60 - CurrentTimeInt % (24*60*60));

						
						debug(DEBUG_LOCAL_TIME_TASK,"here 2 and TimeIntervale is %d\n",TimeIntervale);
						

						break;

					}

					
					debug(DEBUG_LOCAL_TIME_TASK,"任务有效期内\n");
					
					
				}// end of if(CurrentTimeInt > P_task->TaskBegin){
				
				//任务有效开始期前
				// |(current)---|(begin)---|(opratet)---|(end)    @3
				if(CurrentTimeInt < P_task->TaskBegin){
					debug(DEBUG_LOCAL_TIME_TASK,"(P_task->TaskBegin - CurrentTimeInt) is %ld\n",(P_task->TaskBegin - CurrentTimeInt));
					
					
					//相差1天以内
					if((P_task->TaskBegin - CurrentTimeInt)< 24*60*60){
						TimeIntervale = P_task->OpratTimeOffset*60+(P_task->TaskBegin - CurrentTimeInt);
						
						debug(DEBUG_LOCAL_TIME_TASK,"here 3 and TimeIntervale is %d\n",TimeIntervale);
						

						break;
					}
					//相差1天以外
					else{
					
						debug(DEBUG_LOCAL_TIME_TASK,"here 4 and TimeIntervale is %d\n",TimeIntervale);
						

						break;
					}
				}	//end of if(CurrentTimeInt < P_task->TaskBegin){
			

				break;
			case TimerTaskOprate_Weekly:

				break;
			case TimerTaskOprate_Cycle:
				//current time acced ,generate next time point 
				//get normal time 
				if((CurrentTimeInt >= P_task->TaskBegin)&&(CurrentTimeInt <= P_task->TaskEnd)){

					
					debug(DEBUG_LOCAL_TIME_TASK,"here TimerTaskOprate_Cycle 1 and TimeIntervale is \n");
					
					//
					//1  cycle < 24 hours
					printf("P_task->TaskOpratInteral is %d\n",P_task->TaskOpratInteral);
					if(P_task->TaskOpratInteral <= 24*60){
						printf("in case if(P_task->TaskOpratInteral <= 24*60){\n");
						//if((CurrentTimeInt % (24*60*60)) <= (P_task->OpratTimeOffset * 60)){

						if(CurrentTimeInt - P_task->TaskBegin < P_task->OpratTimeOffset*60){
							TimeIntervale = (P_task->OpratTimeOffset * 60)-(CurrentTimeInt % (24*60*60));

							
							debug(DEBUG_LOCAL_TIME_TASK,"CurrentTimeInt %% (24*60*60) is %ld,P_task->OpratTimeOffset * 60 is %d\n",(CurrentTimeInt % (24*60*60)),P_task->OpratTimeOffset * 60 );
							debug(DEBUG_LOCAL_TIME_TASK,"here TimerTaskOprate_Cycle 2 and TimeIntervale is %d P_task->TaskOpratInteral in min is %d\n",TimeIntervale,P_task->TaskOpratInteral);
							

							break;
						}

						printf("(CurrentTimeInt %% (24*60*60)) is %ld,(P_task->OpratTimeOffset * 60) is %d\n",(CurrentTimeInt % (24*60*60)),(P_task->OpratTimeOffset * 60));

						//if((CurrentTimeInt % (24*60*60)) > (P_task->OpratTimeOffset * 60)){

						if((CurrentTimeInt - P_task->TaskBegin) >(P_task->OpratTimeOffset * 60)){

							//TimeIntervale = (P_task->TaskOpratInteral * 60 )-((CurrentTimeInt % (24*60*60))-(P_task->OpratTimeOffset * 60));
							TimeIntervale = P_task->TaskOpratInteral * 60-((CurrentTimeInt-P_task->TaskBegin-(P_task->OpratTimeOffset * 60))%(P_task->TaskOpratInteral*60));//(P_task->TaskOpratInteral * 60 )-((CurrentTimeInt % (24*60*60))-(P_task->OpratTimeOffset * 60));

							
							debug(DEBUG_LOCAL_TIME_TASK,"here TimerTaskOprate_Cycle 3 and TimeIntervale is %d\n",TimeIntervale);
							

							break;
						}
					}
					
					//2 cycle >24 小时
					if(P_task->TaskOpratInteral > 24*60){	
				

						debug(DEBUG,"in case if(P_task->TaskOpratInteral > 24*60){\n");
						//if((CurrentTimeInt % (24*60*60)) <= (P_task->OpratTimeOffset * 60)){

						if(CurrentTimeInt - P_task->TaskBegin < P_task->OpratTimeOffset*60){
							TimeIntervale = (P_task->OpratTimeOffset * 60)-(CurrentTimeInt % (24*60*60));

				
							debug(DEBUG_LOCAL_TIME_TASK,"CurrentTimeInt %% (24*60*60) is %ld,P_task->OpratTimeOffset * 60 is %d\n",CurrentTimeInt % (24*60*60),P_task->OpratTimeOffset * 60);
							debug(DEBUG_LOCAL_TIME_TASK,"here TimerTaskOprate_Cycle 2 and TimeIntervale is %d P_task->TaskOpratInteral in min is %d\n",TimeIntervale,P_task->TaskOpratInteral);
							

							break;
						}

						printf("(CurrentTimeInt  yu (24*60*60)) is %ld,(P_task->OpratTimeOffset * 60) is %d\n",(CurrentTimeInt % (24*60*60)),(P_task->OpratTimeOffset * 60));

						//if((CurrentTimeInt % (24*60*60)) > (P_task->OpratTimeOffset * 60)){

						if((CurrentTimeInt - P_task->TaskBegin) >(P_task->OpratTimeOffset * 60)){

							//TimeIntervale = (P_task->TaskOpratInteral * 60 )-((CurrentTimeInt % (24*60*60))-(P_task->OpratTimeOffset * 60));
							TimeIntervale = P_task->TaskOpratInteral * 60-((CurrentTimeInt-P_task->TaskBegin-(P_task->OpratTimeOffset * 60))%(P_task->TaskOpratInteral*60));//(P_task->TaskOpratInteral * 60 )-((CurrentTimeInt % (24*60*60))-(P_task->OpratTimeOffset * 60));

						
							debug(DEBUG_LOCAL_TIME_TASK,"here TimerTaskOprate_Cycle 3 and TimeIntervale is %d\n",TimeIntervale);
							

							break;
						}//end of if((CurrentTimeInt - P_task->TaskBegin) >(P_task->OpratTimeOffset * 60))

					}

				}

				//get first oprate time
				//未到开始时间，获取第一次执行时间
				if((CurrentTimeInt < P_task->TaskBegin)&&(P_task->TaskBegin-CurrentTimeInt < 24*3600)){
					TimeIntervale = (P_task->TaskBegin - CurrentTimeInt + (P_task->OpratTimeOffset * 60));
					break;
				}

				break;
			case TimerTaskOprate_Once:
				//未到开始时间
				// |(current)---|(begin)---|(opratet)---|(end)    @4
				if((CurrentTimeInt < P_task->TaskBegin)&&((P_task->TaskBegin - CurrentTimeInt)< 24*60*60)){
					TimeIntervale = (P_task->TaskBegin - CurrentTimeInt + (P_task->OpratTimeOffset * 60));
					break;
				}
				
				//超过开始时间\未到结束时间
				// |(begin)---|(current)---|(opratet)---|(end)    @5
				if((CurrentTimeInt > P_task->TaskBegin)&&(CurrentTimeInt < P_task->TaskEnd)){
					if((CurrentTimeInt % (24*60*60)) < P_task->OpratTimeOffset *60){
						TimeIntervale = (P_task->OpratTimeOffset *60) - (CurrentTimeInt % (24*60*60));
						break;
					}
					else{
						break;
					}

				}
				
				break;
			case TimerTaskOprate_HoliDay:
				break;
			default :
				break;
		}	//end of switch

	}//end of 任务尚在有效期
	return TimeIntervale;
}

SINT TimerIntervalForTaskNodeCal(struct TimerRelevTaskIDUnionStruct *P_TaskIDNode,SINT CircleTimes)
{
	SINT Intervale;
	
	
	if(!P_TaskIDNode || CircleTimes<0 ||CircleTimes>TimerTaskQuatityMax){return FAIL;}
	Intervale = TimeIntervalGeneralCal(&TimerTaskList[CircleTimes]);
	//Intervale = 0;
	
	printf("file=%s,func=%s,line=%d\n",__FILE__,__FUNCTION__,__LINE__);
	printf("Intervale is %d\n",Intervale);
	
	if(Intervale >= 0){
		P_TaskIDNode->TimeInterval = Intervale;
		P_TaskIDNode->TimeElapse = 0;
		P_TaskIDNode->ChangeFlag = 0; 	//UNCHAGE;
		P_TaskIDNode->UsedFlag = 1;		//user	
	}
	else{
		P_TaskIDNode->UsedFlag = 0;		//UNCHAGE;
	}
	return SUCCESS;
}
SINT OtherControlMDTimeElapse(time_t timeElapse)
{

return SUCCESS;
}

