/********************************************************************
	> File Name:	coordinate.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年04月11日 星期一 09时57分23秒
 *******************************************************************/
#ifndef __coordinate_h__
#define __coordinate_h__

#include "include.h"
#include "link_method.h"

/**
 * 单灯数据包
 */
typedef struct{
	u8 	group_addr;
	u8 	slave_addr;
	u8 	single_addrH;
	u8 	single_addrL;
	u8 	ctrl;
	u8 	cmd_h;
	u8 	cmd_l;
	u8 	light_level;
	u8 	crc16_l;
	u8 	crc16_h;
}Frame_485;

typedef struct {
	u8	Header;
	u8 	Ctrl;
	u8 	Group_Addr;
	u8 	Coordi_Addr;
	u8 	Single_Addr[2];	//高地址在前，低地址在后
	u8 	Cmd[2];		//高字节在前，低字节在后
	u8 	Data[2];
	u8 	Crc16[2];
}Pag_Single;

typedef enum{
	signle_open 		= 0x0001,
	signle_close 		= 0x0002,
	signle_run_stat 	= 0x0004,
	signle_light_level	= 0x0008,
	signle_volts 		= 0x0010,
	signle_electric 		= 0x0020,
	signle_power 		= 0x0040,
	signle_other 		= 0x0080,
	single_Group 		= 0x0100,	//配置组号
	single_MapAddr 	= 0x0200,	//配置映射地址
	Single_Query 		= 0x0400,
} signle_ctl;


/* 获取接受或发送buf的地址 */
typedef enum {
	send_addr,
	recv_addr
}SR_t;

enum CtrlCode{
	CtrlCode_Single 	= 0x01,
	CtrlCode_Group 	= 0x02,
	CtrlCode_Broadcast 	= 0x04,
	CtrlCode_Coordi 	= 0x10,
	CtrlCode_Check 	= 0x20,
};

extern s8* Getbuf(SR_t sr);
extern s32 Device_recv_485(UartPort  port,  s8** buf,   u32 len,  s32  block);
extern s32 DeviceRecv485_v2(UartPort  port,  s8 *buf,   u32 len,  s32  block);

#endif		//#ifndef __coordinate_h__

