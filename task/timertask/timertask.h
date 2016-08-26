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
#ifndef __TIMERTASK_H__
#define __TIMERTASK_H__
#include "include.h"
//#include "task.h"
#include "tasklist.h"
#define PAKECT_LEN 40

#define TimerTaskQuatityMax		  5


typedef struct TimerTaskToSaveFormatStruct{

	u8 	TaskID[2];
	u8 	DelEnable;		//是否允许删除
	u8 	TaskType;		//0-nomal ;1-circult
	u8 	ControlMode;		//控制方式timecontrol/ 2\timeandarry
	u8 	CycleType;		//周期类型(weekyly dalay\once)
	u8 	OpraType;		//open :1 /close: 2 操作类型
	time_t 	TaskBegin;		//起始年月日,0为无效任务项(未设置或已删除),当日0时0分的time_t值
	time_t 	TaskEnd;		//结束年月日,当日23时59分的time_t值
	u16 	OpratTimeOffset;	//执行时间偏移
	u8 	TaskStartEvryDay;	//相对每年开始有效时间
	u8 	TaskStartEvryMonth;
	u8 	TaskEndEvryDay;	//相对每年结束有效果时间
	u8 	TaskEndEvryMonth;
	u8 	WeekDay;		//星期(0-6依次代表周日,一~六)或日期
	u16 	TaskOpratInteral;	//间隔定时任务间隔(Min)
	u8 	Reserve[4];
	u8 	TaskCMDLen;
	u8 	TaskCMDPakt[PAKECT_LEN];

}__attribute__((packed))TimerTaskToSaveFormatStruct ;

typedef struct ServerCommunTimerTaskStruct{
	UCHAR TaskID[2];
	UCHAR TaskType;		//0 :namal;1-cicult

	UCHAR ControlMode;		//1-timecontrol
	UCHAR CycleType;		//dalay,weekly,moun,interval,once
	UCHAR OpraType;		//1:open 2close,3:query ,4:

	UCHAR TaskStartDay;
	UCHAR TaskStartMonth;
	UCHAR TaskStartYear;

	UCHAR TaskEndDay;
	UCHAR TaskEndMonth;
	UCHAR TaskEndYear;

	UCHAR RelatEffecStartDay;
	UCHAR RelatEffecStartMonth;

	UCHAR RelatEffecEndDay;
	UCHAR RelatEffecEndMonth;

	//USHORT TimerInterval;
	UCHAR WeekDay;
	UCHAR Interval_LOW;
	UCHAR Interval_HI;

	UCHAR TaskExecuteHour;
	UCHAR TaskExecuteMinute;
	UCHAR Reserve[4];

	UCHAR TrasfCMDPakectLen;
	UCHAR TrasfCMDPakect[PAKECT_LEN];

}__attribute__((packed))ServerCommunTimerTaskStruct ;


enum{	//cycle
	TimerTaskOprate_Dalay =1,
	TimerTaskOprate_Weekly,
	TimerTaskOprate_Cycle,
	TimerTaskOprate_Once,
	TimerTaskOprate_HoliDay,
};


enum{	//control mode 
	TASK_CONTROL_TYPE_TIME =1,
	TASK_CONTROL_TYPE_ARRY,
	TASK_CONTROL_TYPE_TIMEANDARRY,
	TASK_CONTROL_TYPE_LATI_LONGITUDE,
	TASK_CONTROL_TYPE_ARRYANDLALOGI,

};

typedef union {
	USHORT TimeL;
	UCHAR TimeS[2];
} TIME_SHORT_FORMAT;

struct TimerRelevTaskIDUnionStruct{
	SINT TaskIDOffs;

	UCHAR UsedFlag;
	UCHAR ChangeFlag;

	SINT TimeInterval;
	SINT TimeElapse;

};

struct TOPTimerTaskListStruct{
	
	struct TimerRelevTaskIDUnionStruct TimerTaskNodes[TimerTaskQuatityMax];
	struct itimerval PreValue;		//先前时间值
	struct itimerval NowValue;		//当前时间值

	UINT TimerChangeFlag;			//
	UINT TimerTaskStat;

	void (*PreFuction)(SINT);
	void (*NowFuction)(SINT);
	SINT (*CallBackFunc)(void *,SINT);
	pthread_mutex_t lock;
};

SINT TimerTaskSetProc(UCHAR *buf,UCHAR len);
SINT TimerTaskIDSearch(UCHAR *taskID);
SINT TimerTaskFormatChang2SaveFormat(struct TimerTaskToSaveFormatStruct *PtimetaskInsert,struct ServerCommunTimerTaskStruct *Taskstruget);
SINT TimerTaskMarkSet(SINT taskIDOffs);


SINT TimerEngineInit(void);
void TimerTaskPerTickfuntion();
SINT TimeIntervalGeneralCal();
SINT TimerIntervalForTaskNodeCal();
SINT OtherControlMDTimeElapse(time_t timeElapse);

UINT WhetherCMDExist(UCHAR type,UCHAR cmd);

#endif
