/********************************************************************
	> File Name:	config.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月11日 星期三 09时14分49秒
 *******************************************************************/
#ifndef __config_h__
#define __config_h__

#define Not_Use 		0
#define DebugPrint 		0
#define Config_Log
#define Config_NewProtocol
#define Config_EC_6018

#define Uart1_ttyO1_485 	1		//和协调器通讯的串口
#define Uart3_tty03_DIDO	3		//和扩展DIDO通讯的串口

#define ResponseSubCmd 			//广播和组播操作时候第一次回复增加子命令字段
#define SingleNumber 	500 		//一个协调器下的最大单灯个数
#define PWMmax 		0x1D		//PWM的峰值
//#define Config_PWM_ 				//pwm的正负，
//#define MC_ARM335X			//新开发板串口支持

//#define SingleCheckThread 			//单灯状态查询是否启用一个线程

#define UsePthread 				//使用多线程
#define HeartBeatErrCnt  	3 		//连续发送心跳包失败次数后重新连接

#endif

