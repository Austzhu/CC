/********************************************************************
	> File Name:	config.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月11日 星期三 09时14分49秒
 *******************************************************************/
#ifndef __config_h__
#define __config_h__

#define BARD_E6018
#define Config_ztcc

#if !defined(Config_ztcc) && !defined(Config_lamp)
#error "must define one of 'Config_ztcc, Config_lamp'"
#endif

#if defined(BARD_E6018)
	#define Config_Meter
	#define Config_EC_6018
	#define Config_Log
	#define Config_operate
	#define Config_autoControl
	#define Config_ether
	#define Config_serial
	#define Config_Sqlite
	#define Config_UART
	#define Config_KALMAN
	#define Config_TIMETASK
#elif defined(BORD_E3100)
	#define Config_TCP_Server
	#define Config_wizdom
	#define Config_Log
	#define Config_operate
#endif

#ifdef Config_Sqlite
#define CFG_DB_NAME 	"cc_corl.db"
#define CFG_tb_coordi 	"db_coordinator"
#define CFG_tb_single 		"db_light"
#define CFG_tb_task 		"db_task"
#define CFG_tb_tasklist	"db_tasklist"
#define CFG_tb_light_info	"db_info_light"
#define CFG_tb_warn 		"db_warn"
#define CFG_tb_tunnel 	"db_tunnel_info"
#define CFG_tb_pwm 		"db_index_pwm"
#define CFG_tb_gpinfo		"db_group_info"
#define CFG_tb_dido 		"db_dido_info"
#endif

#ifdef Config_TCP_Server
#define CFG_client_max 		10
#define CFG_ReuseAddr 		1 		//设置网络bind地址可以重用
#define CFG_keepidle 			30		//如该连接在30秒内没有任何数据往来,则进行探测
#define CFG_keepinterval 		5		//keepalive探测间隔
#define CFG_keepcount 		3 		//keepalive 探测次数
#endif

#define CFG_Recvbuf 			2048
#define CFG_FILE_PRAM 		"./config/fileparam.ini"		//配置文件
#define CFG_COM485 			1 		//和协调器通讯的串口
#define CFG_SENSOR			2		//光照车流传感器串口
#define CFG_COMDIDO		3 		//和扩展DIDO通讯的串口
#define CFG_PWMAX			0x1D 	//PWM的峰值
//#define CFG_PWM_N 				//负极性的pwm
//#define CFG_MC_ARM335X 			 //支持Ti-AM335x开发板
//#define CFG_showPackage 			//是否打印回复上位机的数据包
//#define CFG_exitMessage

#endif

