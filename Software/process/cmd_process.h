#ifndef __CMD_PROCESS_H__
#define __CMD_PROCESS_H__


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
#include "include.h"
#include "tasklist.h"
//#include "timertask.h"
#include "serial.h"
//#include "softwareinit.h"
#include "loadfile.h"
#include "link_method.h"
#include "gprs_ppp.h"
#include "meter_crc.h"
#include "coordinate.h"
#include "meter_cmdrdwr.h"
#include "SingleOrCoordi.h"
#include "SingleOpt.h"
extern TimerTaskToSaveFormatStruct  TimerTaskList[TimerTaskQuatityMax];

//extern SINT TopElecInforPrintf(struct MetterElecInforStruc *);
extern UINT MetterRealTimeRDOprate(CHAR DeviceId,CHAR contex, /*struct MetterElecInforStruc * */ void*);
extern SINT CCLocateTimeUpLoad(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD);
extern SINT RTCHardWareTimeGet(CHAR *FilePath,struct tm * TimeStru);
extern SINT SMS_MODULE_INFOR_MakeAND_UpLoad(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD);
extern SINT SMS_MODULE_DAIL_COMPULSORY(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD,UCHAR *Telephone);
extern UINT Reset2DoFunctions(void);

extern u32 CallBack_Open(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern u32 CallBack_Close(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern u32 CallBack_Light(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern u32 CallBack_Demand(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern u32 CallBack_RtData(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern s32 CC_Inquire_Version(u8 *AckBuf,u8 Ctrl,u8 Cmd);
extern u32 CallBack_electric(u8 ctrl,u8 itf,struct task_node *node);

extern UINT CallBackGPRSTest1(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackServerFeedback(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackLogon(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackMetterInfoColletRD(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackMetterInfoColletSet(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackMetterInfoColletExtrSet(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackMetterInfoColletExtrRD(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern u32 CallBackMetterInfoBroadcast(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackMetterDIDO(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackETHShotAck(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackETHLongContexBack(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackApplicUpDate(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackTimerControlTaskRLT(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackCCGlobalParaSetGet(UCHAR ctrl,UCHAR itf,struct task_node *node);
extern UINT CallBackReset(UCHAR ctrl,UCHAR itf,struct task_node *node);



extern s32 TOPDIDOShortCallBackAck(UCHAR *buf,CHAR AppendType,CHAR AppendLen,CHAR AckCMD,CHAR AckLen,CHAR AckRequCMD,CHAR AckRequSubCMD,CHAR DeviceID,CHAR ChanleID,CHAR Stat,CHAR Resault);
#endif
