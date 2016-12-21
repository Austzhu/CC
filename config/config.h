/********************************************************************
	> File Name:	config.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月11日 星期三 09时14分49秒
 *******************************************************************/
#ifndef __config_h__
#define __config_h__

#define BARD_E6018

#if defined(BARD_E6018)
	#define Config_Meter
	#define Config_EC_6018
	#define Config_Log
	#define Config_operate
	#define Config_autoControl
	#define Config_ether
	#define Config_serial
	#define Config_Sqlite
#elif defined(BORD_E3100)
	#define Config_TCP_Server
	#define Config_wizdom
	#define Config_Log
	#define Config_operate
#endif



#ifdef Config_TCP_Server
#define CFG_client_max 		10
#define CFG_ReuseAddr 		1 		//设置网络bind地址可以重用
#define CFG_keepidle 			30		//如该连接在30秒内没有任何数据往来,则进行探测
#define CFG_keepinterval 		5		//keepalive探测间隔
#define CFG_keepcount 		3 		//keepalive 探测次数
#endif

#define CFG_COM485 			1 		//和协调器通讯的串口
#define CFG_COMDIDO			3 		//和扩展DIDO通讯的串口
#define CFG_PWMAX			0x1D 	//PWM的峰值
//#define CFG_PWM_N 					//负极性的pwm
//#define CFG_MC_ARM335X 			 //支持Ti-AM335x开发板
//#define CFG_showPackage 			//是否打印回复上位机的数据包
//#define CFG_exitMessage

#endif

