/********************************************************************
	> File Name:	SingleOrCoordi.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com 
	> Created Time:	2016年06月02日 星期四 16时46分47秒
 *******************************************************************/
#ifndef __SingleOrCoordi_h__
#define __SingleOrCoordi_h__
#include "include.h"

#define SOH 	0x01
#define STX 	0x02
#define EOT 	0x04
#define ACK 	0x06
#define NAK 	0x15
#define CAN 	0x18
#define CTRLZ	0x1A

#define UPACKSIZE 	128

#define AssertProgram(Expr)	do{if(Expr)return 0; }while(0)


enum{
	Crc_16,
	Crc_8,
	addChk,
};
typedef struct{
	u32 	filesize;			//升级文件大小
	u32 	DataNum;		//需要发送多少帧数据
	u32 	CntDataNum;		//当前发送的帧
	u32 	Repeat;			//每帧错误重发次数
	u32 	SinOrCoordi;		//升级单灯/协调器标记
	u32 	CheckType;		//校验方式
	s32 	fd;			//升级文件描述符
	s32 	pend;			//升级挂起
	s32 	Timeout;		//超时
	s32 	Update_OK;		//升级完成
	s32 	Is_UpdateImageA;	//升级A区？
}Package_Info;

typedef struct{
	u8 	Header;
	u8 	Coor_Addr;
	u8 	Single_Addr_H;
	u8 	Single_Addr_L;
	u8 	F_Num;		//第几帧数据
	u8 	__Num;		
	u8 	Data[UPACKSIZE];
	u8 	CRC16[2];	//低在前，高位在后
}Package_Data;

extern s32 UpdateThread(void * arg);
extern void * Updatethread(void *arg);
extern void *Update_Single_Coordi(void);
extern void inline GetUpdateInfo(Package_Info *Info);
extern void inline SetUpdateInfo(Package_Info *Info);
extern void Close_Update(void);
#endif
