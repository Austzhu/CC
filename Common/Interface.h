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
#ifdef Config_Meter
#include "Meter.h"
#endif
#include "Warn.h"


#define Connect_ok              0
#define Connect_error        -1
#define HeartBeat_ok          0
#define HeartBeat_error    -1

typedef enum {ether_net = 1,  gprs,  zigbee  }  ItfWay_t;
typedef enum { crc_get, crc_check }  crc_t;

typedef struct{
	u8 CCUID[6];                   //集中控制器的UID
	u8 DebugLevel;	             //调试等级
	u8 ControlMethod;	      //控制模式
	s8 ServerIpaddr[32];     //服务器IP
	u16 ServerPort;             //服务器的端口
	u8 ItfWay;                       //集中器的操作方法
	u8 HeartBCycle;            //心跳周期
	u8 KwhSaveDay;           //电路保存天数
	u8 KwhReadInter;       //电量的读取间隔
	u8 Is_TCP;                      //与服务器的连接方式
} param_t;

typedef struct appitf_t {
	Queue_t   *Queue;
	ethernet_t *ethernet;
	serial_t   *Serial;
	sql_t   *sqlite;
	Single_t   *single;
	Warn_t   *warn;
	#ifdef Config_Meter
	Meter_t   *meter;
	#endif
	int Connect_status;        //网络连接状态
	int HeartBeat_status;    //心跳状态
	param_t param;
	int (*const UID_Check)(struct appitf_t *this,void*r_uid);
	int (*const TopUserInsertQue)(struct appitf_t *this);
	int (*const TopUserProcQue)(struct appitf_t *this);
	int (*const TopUserKeepalive)(struct appitf_t *this);
	int (*const TopUserRecvPackage)(struct appitf_t *this,u8*,int);
	int (*const app_Init)(struct appitf_t *this);
	int (*const app_relese)(struct appitf_t*);
	int (*const packagecheck)(void*);
	int (*const Crc16)(int,u8*,u8*,int);
	int (*const Crc16_HL)(int,u8*,u8*,int);
	u8* (*const str2hex)(u8*,const char*);
	s8* (*const hex2str)(char*dest,const u8 *src,int size);
	void (*const msleep)(u32);
	void (*const usleep)(u32);
} appitf_t;

extern appitf_t g_appity;


#endif
