/********************************************************************
	> File Name:	config.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月11日 星期三 09时14分49秒
 *******************************************************************/
#ifndef __config_h__
#define __config_h__

#define Config_Log
#define Config_NewProtocol
#define Config_EC_6018
#define Config_Meter

#define COM_485 		1		//和协调器通讯的串口
#define COM_DIDO	3		//和扩展DIDO通讯的串口

#define PeakPwm 		0x1D	//PWM的峰值
//#define Config_PWM_ 		//pwm的正负，
//#define MC_ARM335X		//新开发板串口支持

#define HeartBeatErrCnt  	3 	//连续发送心跳包失败次数后重新连接
//#define DisplayResPackage	//是否打印回复上位机的数据包

#endif

