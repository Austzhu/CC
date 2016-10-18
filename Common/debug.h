#ifndef __DEBUG_H__
#define __DEBUG_H__

#define ERR_PRINT
#define MY_DEBUG 	2
#define LEVEL 		5 	//设置调试打印等级

typedef enum{
	CRIT,
	ERROR,
	WARNING,
	NOTICE,
	INFO,
	DEBUG
} debug_level;

#define EthnetSend_Print				0		//开启显示网口发送数据的宏


#define DEBUG_Logon 				1
#define DEBUG_Timetask 				1		//定时任务
#define DEBUG_CC_Config 				1		//集中器配置
#define DEBUG_ERR 					1		//出现错误
#define DEBUG_Log 					1		//日志文件
#define DEBUG_Serial 					1		//串口调试
#define DEBUG_TaskAppend 				1		//加入队列信息
#define DEBUG_RECV					0 		//显示接收数据信息
#define DEBUG_server2cc 				0 		//服务器应答cc信息
#define DEBUG_cc2server 				0 		//cc对服务器应答
#define DEBUG_reboot 				1 		//重启调试
#define DEBUG_chktime 				1 		//对时调试
#define DEBUG_reset 					1 		//复位调试
#define DEBUG_inqTime 				1 		//查询时间命令调试
#define DEBUG_inqVersion				1		//查询版本号
#define DEBUG_DIDO 					1 		//DIDO调试
#define DEBUG_Broadcast 				1 		//广播命令
#define DEBUG_sqlite3 				1 		//sqlite数据库调试
#define DEBUG_coordinate 				1		//协调器的调试宏
#define DEBUG_single 					1 		//单灯
#define DEBUG_loadfile 				1		//加载配置文件调试宏
#define DEBUG_list 					1
#define DEBUG_update 				1		//升级程序调试宏
#define DEBUG_Ethnet 				1 		//网络调试宏
#define DEBUG_DelSql					1
#define DEBUG_Queue 				1




#define DEBUG_LOCAL_ETHNET			0		//网络调试
#define DEBUG_LOCAL_LINK_METH 			0
#define DEBUG_LOCAL_FORMAT_CH 			0
#define DEBUG_LOCAL_TASK 				0
#define DEBUG_LOCAL_TASK_EXCUT 			0
#define DEBUG_LOCAL_DATABASE 			0
#define DEBUG_LOCAL_FILEPARAM_LOAD 		0
#define DEBUG_LOCAL_BURGLAR 			0
#define DEBUG_ERR_RECD_ETHNET 			0
#define DEBUG_ERR_RECD_LINK 			0
#define DEBUG_ERR_RECD_SERIAL 			1
#define DEBUG_LOCAL_METTER 			0
#define DEBUG_LOCAL_DIDO 				0
#define DEBUG_LOCAL_GLOBAL_INFOR 		1
#define DEBUG_LOCAL_TIME_TASK 			0
#define DEBUG_LOCAL_PARAFILE_SAVE 		0
#define DEBUG_LOCAL_PARA_RECIV 			0
#define DEBUG_LOCAL_HEATBEAT 			0
#define DEBUG_LOCAL_CC2SERVER_RIGISTER 		0
#define DEBUG_LOCAL_CC2SERVER_HEARTBEAT 	0
#define DEBUG_LOCAL_HEAPMEM_INCREASE 		0
#define DEBUG_LOCAL_CMD_RESET 			0
#define DEBUG_LOCAL_GPRS_ATSEND 		0

#if 	MY_DEBUG == 1
	#define debug(level,fmt,args...)  do{ if(level<=LEVEL) printf(fmt,##args); }while(0)
#elif 	MY_DEBUG == 2
	#define debug(level,fmt,args...)  do{ if(level) printf(fmt,##args); }while(0)
#else
	#define debug(...)
#endif		//#ifdef DEBUG

#ifdef ERR_PRINT
	#define err_Print(level,fmt,args...) do{if(level){ printf("<%s>%d: ",__func__,__LINE__); printf(fmt,##args);}}while(0)
#else
	#define err_Print(...)
#endif

#endif 		//#define __DEBUG_H__
