/******************************************************************
** 文件名:	Interface.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef   __Interface_h__
#define  __Interface_h__
#include "taskque.h"
#include "ether.h"
#include "serial.h"
#include "database.h"
#include "single.h"
#include "operate.h"
#ifdef Config_Meter
#include "Meter.h"
#endif
#include "Warn.h"
#ifdef Config_TCP_Server
#include "ether_server.h"
#endif
#ifdef Config_autoControl
#include "auto_control.h"
#include "auto_calc.h"
#include "auto_sensor.h"
#endif

#ifdef Config_UART
#include "UART.h"
#endif

#define Connect_ok 		0
#define Connect_error 		-1
#define HeartBeat_ok 	 	0
#define HeartBeat_error 	-1

typedef enum {ether_net = 1,  gprs,  zigbee ,lora }  ItfWay_t;

typedef struct {
	unsigned char head;		//=68H
	unsigned char rtua[6];
	unsigned char dep;			//=68H
	unsigned char ctrl;			//控制域C
	unsigned char len;			//data length
	unsigned char data[0];
} faalpkt_t;


typedef struct param_t{
	u8 CCUID[6]; 			//集中控制器的UID
	u8 DebugLevel; 		 //调试等级
	u8 ControlMethod;		//控制模式
	s8 ServerIpaddr[32]; 	//服务器IP
	u16 ServerPort;			//服务器的端口
	u8 ItfWay;				//集中器的操作方法
	u8 HeartBCycle;		//心跳周期
	u8 KwhSaveDay;		//电路保存天数
	u8 KwhReadInter;		//电量的读取间隔
	u8 Is_TCP;				//与服务器的连接方式
} param_t;

typedef struct appitf_t {
	int Connect_status;		//网络连接状态
	int HeartBeat_status;	//心跳状态
	int pthread_start;
	pthread_t thread_Keepalive ;
	pthread_t thread_RecvInsert;
	pthread_t thread_UserProc ;
	struct param_t param;

	#ifdef Config_serial
	struct serial_t 	*Serial;
	#endif
	#ifdef Config_Sqlite
	struct sql_t 	*sqlite;
	#endif
	struct Warn_t 	*warn;
	struct Single_t 	*single;
	struct Queue_t *Queue;
	struct operate_t *opt_Itf;
	#ifdef Config_TCP_Server
	struct server_t 	*tcp_server;
	#endif
	#ifdef Config_Meter
	struct Meter_t *meter;
	#endif
	#ifdef  Config_autoControl
		control_t *auto_mode;
	#endif

	int (*const TopUser_Uidchk)(struct appitf_t *this,void*r_uid);
	int (*const TopUser_InsertQue)(struct appitf_t *this);
	int (*const TopUser_ProcQue)(struct appitf_t *this);
	int (*const TopUser_Keepalive)(struct appitf_t *this);
	int (*const TopUser_RecvPackage)(struct appitf_t *this,u8*,int);
	int (*const TopUser_Init)(struct appitf_t *this);
	int (*const TopUser_relese)(struct appitf_t*);
	int (*const TopUser_pakgchk)(void*);
} appitf_t;

extern appitf_t g_appity;


#endif
