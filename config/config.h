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
	#define  Config_Meter
	#define Config_EC_6018
	#define Config_TCP_Server
#elif defined(BORD_E3100)
	#define Config_wizdom
#endif

#ifdef Config_TCP_Server
#define Config_client_max 	10
#define Config_ReuseAddr 	1 	//设置网络bind地址可以重用
#define Config_keepidle 		30	//如该连接在30秒内没有任何数据往来,则进行探测
#define Config_keepinterval 	5	//keepalive探测间隔
#define Config_keepcount 	3 	//keepalive 探测次数
#define Config_ReuseAddr 	1 	//设置网络bind地址可以重用
#endif

#define Config_Log
#define Config_COM485    1                 //和协调器通讯的串口
#define Config_COMDIDO  3                //和扩展DIDO通讯的串口
//#define Config_PWM_N                      //负极性的pwm
#define Config_PWMAX      0x1D         //PWM的峰值
//#define Config_MC_ARM335X		 //支持Ti-AM335x开发板
//#define Config_showPackage           //是否打印回复上位机的数据包

//#define Config_exitMessage

#endif

