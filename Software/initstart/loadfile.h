/********************************************************************
	> File Name:	loadfile.h
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年05月20日 星期五 12时45分26秒
 *******************************************************************/
#ifndef __loadfile_h__
#define __loadfile_h__
#include "include.h"

#define FileSize 				1024
#define FILE_PARAM   			"./config/fileparam.ini"

#define Default_ServerIp 		"192.168.0.119"
#define Default_ServerPort 	8888
#define Default_CCUID 		"00000000c121"
#define Default_debuglevel 	2
#define Default_Method 		2
#define Default_ItfWay 		2
#define Default_HeartBCycle	5
#define Default_TCP 			1

extern s32 loadParam(void*);
extern s32 SaveParam(void);
#endif
