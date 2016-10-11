/********************************************************************
	> File Name:	log.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月09日 星期一 20时50分21秒
 *******************************************************************/
#ifndef __log_h__
#define __log_h__
#include "include.h"
#define LogCount 	5
typedef enum{
	CC		= 0X01,
	Coordinate	= 0X02,
	err,
	warring,
	Log_err 	= 0x100
}Logfile_t;

/**
 *  功能: 写日志
 *  参数: cmd = CC 后面跟的参数是faalpkt_t* 写的是服务器和集中控制器的日志
 *  	 cmd = Coordinate 后面跟的是Frame_485* 写的是集中器和协调器的日志
 *  返回值: 成功 SUCCESS 失败 FAIL
 */
extern s32 Write_log(int cmd, ...);
#endif
