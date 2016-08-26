/********************************************************************
	> File Name:	loadfile.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月20日 星期五 12时45分26秒
 *******************************************************************/
#ifndef __loadfile_h__
#define __loadfile_h__
#include "include.h"
#include "burglar_alarm.h"
#include "link_method.h"
#include "timertask.h"
//#include "task.h"
#include "tasklist.h"
#include "cmd_process.h"
#include "meter_cmdrdwr.h"

#define FileSize 		1024
#define FILE_PARAM   		"./config/fileparam.ini"
#define CCID 			"CCUID"
#define Duglev			"DebugLevel"
#define ConType 		"ConnectType"
#define ServerIp 		"ServerIpaddr"
#define SPort 			"ServerPort"
#define Beatcycle 		"HeartBeatcycle"

#define Default_ServerIp 	"192.168.0.119"
#define Default_ServerPort 	8888
#define Default_CCUID 	"888888888888"
#define Default_debuglevel 	2
#define Default_Method 	2
#define Default_ItfWay 	ITF_WAY_ETHER
#define Default_HeartBCycle	5
#define Default_TCP 		1

#define DIDI_START_ADDR			6
#define DIDO_DEVICE_QUANTITY	1		//3*8=24circul
#define METER_START_ADDR		1
#define METER_DEVICE_QUANTITY	1
#define GROUP_LED_QUANTITY		10

typedef struct {
	u8 CCUID[6];
	u8 DebugLevel;
	u8 ControlMethod;	//手自
	s8 ServerIpaddr[32];
	u16 ServerPort;
	u8 ItfWay;
	u8 HeartBCycle;
	u8 KwhSaveDay;	//电路保存天数
	u8 KwhReadInter;	//电量的读取间隔
	u8 SignalDBM;
	u8 Is_TCP;
}GlobalCCparam;

enum {
	CUID,
	DUGLEVEL,
	METHOD,
	ITWAY,
	CYCLE,
	IP,
	PORT
};
enum {
	_u8_1,
	_u8_2,
	_u8_6,
	_u8_s,
};
extern GlobalCCparam CCparamGlobalInfor;
extern s32 loadParam(void);
extern s32 SaveParam(void);
#endif
