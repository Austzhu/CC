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
#include "base_type.h"
#define moffsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define mcontainer_of(ptr, type, member) 	{  const typeof( ((type *)0)->member ) *__mptr = (ptr);\
						(type *)( (char *)__mptr - moffsetof(type,member) );   }

enum {
	itf_ether = 2,
};

typedef struct {
	int sock_ether;				//网络连接的socket
	int ether_recvlen;			//缓存数据的长度
	int ether_recvhead;			//缓存数据中的读指针偏移量
	u8 ether_recvbuf[2048];		//数据的缓存区
	int (*connect)(void);			//链接建立
	int (*disconnect)(void);			//断开链接
	int (*logon)(void);			//登陆
	int (*linestat)(void);			//链路状态
	int (*rawsend)(UCHAR *buf, int len);   	//发送函数
	int (*getchar)(UCHAR *buf);    		//接收函数
	int (*keepalive)(void);			//心跳
} ethernet_t;

typedef struct {

} Serial_t;

typedef struct {

} Queue_t;

typedef struct{
	u8 CCUID[6];			//集中控制器的UID
	u8 DebugLevel;			//调试等级
	u8 ControlMethod;		//控制模式
	s8 ServerIpaddr[32];		//服务器IP
	u16 ServerPort;			//服务器的端口
	u8 ItfWay;			//集中器的操作方法
	u8 HeartBCycle;		//心跳周期
	u8 KwhSaveDay;		//电路保存天数
	u8 KwhReadInter;		//电量的读取间隔
	u8 Is_TCP;			//与服务器的连接方式
	ethernet_t 	net;		//与服务器通信的操作方法
	Serial_t 	ttyS485;	//485通信操作的方法
	Queue_t 	netque;		//队列操作方法
} appitf_t;

extern appitf_t g_appity;
int appitf_init(appitf_t*);

#endif
