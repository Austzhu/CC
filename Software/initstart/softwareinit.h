#ifndef __SOFTWAREINIT_H__
#define __SOFTWAREINIT_H__
#include "include.h"
#include "burglar_alarm.h"
#include "link_method.h"
#include "timertask.h"
//#include "task.h"
#include "tasklist.h"
#include "cmd_process.h"
#include "meter_cmdrdwr.h"

#define FILE_PARAM_PASS   		"./config/fileparam.ini"
#define DIDI_START_ADDR		6
#define DIDO_DEVICE_QUANTITY	1		//3*8=24circul
#define METER_START_ADDR		1
#define METER_DEVICE_QUANTITY	1
#define GROUP_LED_QUANTITY		10

typedef struct GlobalCCparam{

	UCHAR CCUID[6];
	UCHAR DebugLevel;		
	UCHAR ControlMethod;			//手自
	UCHAR ItfWay;
	CHAR ServerIpaddr[32];
	USHORT ServerPort;
	UCHAR HeartBCycle;
	UCHAR SignalDBM;

 #if 0
	UCHAR HeartBCycle;
	UCHAR TimeOut;
	UCHAR Retry;				//重发
	UCHAR ConfirmFlag;			//需要主站确认的通信服务(CON=1)的标志		
	//gprs
	UCHAR Proto;				//TCP -0;UDP-1
	UCHAR Dcd;				//自动拨号
	UCHAR Art;				//密码算法编号
	UCHAR Apn[10];			//接入服务器
	UCHAR NetSwitch;			//介入开关
	UCHAR SMSAlarmSupport;		//短信报警启用
 #endif

 #if 0
	SINT HeartBCycle;
	SINT TimeOut;
	SINT Retry;
	SINT ConfirmFlag;
	//gprs
	SINT Proto;
	SINT Dcd;
	SINT Art;//密码算法编号
	CHAR Apn[16];
	SINT NetSwitch;
 #endif

 #if  Not_Use
	UCHAR AlarmTelNumberAdmin[20];			//suppur admin
	UCHAR AlarmTelNumberUser1[20];			//namal user1
	UCHAR AlarmTelNumberUser2[20];
	UCHAR AlarmTelNumberUser3[20];
	UCHAR AlarmTelNumberUser4[20];

	UCHAR StartOFDay[2];					//[0]:hour,[1]:min
	UCHAR EndOfDay[2];					//[0]:hour,[1]:min
	//meter
	UCHAR MeterInfRecDays;				//recoard days
	UCHAR MeterRDInterval;				//read cicle min

	UCHAR Reserved[4];

	UCHAR SignalDBMInterval;
 #endif	
}__attribute__((packed))GlobalCCparam;

UINT ArgcParamAnlys();
//UINT HardWareInit();
UINT DefineParamLoad();
UINT DownLoadTaskLoad();
UINT DefineParamCheck();
UINT SetDebugShowLvl(UCHAR i);

SINT ParamFileFormtLoad();
SINT ParamSqliteFormtLoad();

SINT FileParamLoad(void);
SINT FileParamLoadedResultPrint(void);
SINT ProfileStringRead(const CHAR *section,const CHAR *key,char *value,int size,CHAR *DefaultValue,const CHAR *file);
SINT FileLoad2buf(const CHAR *file,CHAR *filebuf,SINT *filesize);
SINT ParamFindInFileBuf();

SINT IsRightBrach(CHAR c);
SINT EndofFileBufstr(CHAR c);
SINT IsNewLine(CHAR c);
SINT IsLeftBarch(CHAR c);
//SINT BCDToHex(CHAR *src, CHAR *dst, SINT lens);
SINT BCDToHex(UCHAR *src, UCHAR *dst, SINT lens);

SINT ProfileIntRead(const CHAR *section,const CHAR *key,SINT value,const CHAR *file);
SINT TOPTaskSaveAndFileParamRelatOprat(UCHAR flag);
SINT FileParamSave(UCHAR Flag);
SINT TopFileParamLoad(UCHAR Flag);
SINT ProfileStringWrite(const CHAR *section,const CHAR *key,const char *value,const CHAR *file);
SINT ProfileIntWrite(const CHAR *section,const CHAR *key,char *value,CHAR *DefaultValue,const CHAR *file);
SINT TOPCCPARAMSaveToFile(void);
UINT TOPGlobalInforPrintAndSave(void);
UINT TOPDeviceInforShowInit();
UINT TOPDetailTaskInforShow();


#endif
