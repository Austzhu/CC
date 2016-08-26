/******************************************************************
** 文件名:	infor_out.c
** Copyright (c) 2012-2014 *********公司技术开发部
** 创建人:	wguilin
** 日　期:	2012.03
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:	
**
** 版　本:	V1.0
*******************************************************************/

#ifndef __CC_PARAM__
#define __CC_PARAM__

#include "include.h"
#include "server_recv.h"

typedef struct cc_task_global{
	UCHAR starttime[2];		//白天起始时间  2010.7.31    
	UCHAR endtime[2];		//白天结束时间  2010.7.31   
	UCHAR days;				//保存电能信息的时间天数
	UCHAR interval;    			//轮询电表的时间间隔(小时)

	UCHAR telephone[20];		//手机号码
	UCHAR telephone1[20];		//手机号码
	UCHAR telephone2[20];		//手机号码
	UCHAR telephone3[20];		//手机号码
	
}__attribute__((packed)) cc_task_global;


typedef struct cc_global_para{
	UCHAR ipmain[32];				//服务器域名或IP
	UCHAR uid[6];			       	//集中器uid
	UCHAR version[20];  		       	 //集中器的版本信息
	UCHAR company[15];			     //
	UCHAR website[20];			   //
	UCHAR equipment[20];
}cc_global_para;		//__attribute__((packed)) cc_global_para;


typedef struct cc_connect_para{
	UCHAR itf;			//接口模式 Ether/CDMA/GPRS/SMS
	USHORT portmain;		    //端口号
	UCHAR cycka;				//心跳周期, 分

	UCHAR flagcon;			//需要主站确认的通信服务(CON=1)的标志
	UCHAR proto;    				//0-TCP, 1-UDP
	UCHAR art;					//密码算法编号

	UCHAR netSwitch;				//网络开关
	UCHAR dcd;					//GPRS自动检测掉线.1-检测/0-不检测
	UCHAR apn[16];				  //GPRS APN	
	UCHAR csqThreshold;			//gprs 信号阈值小于此值不拨号
	UCHAR curCsq;				//当前信号强度，ether则为FF
	UCHAR masterSwitch[6];		//白天关灯标示码只需一个终端与此uid匹配即开启


	UCHAR timeout;			//终端等待从动站相应的超时时间
	UCHAR retry;				//重发次数
	UCHAR plc_retry;
	UCHAR plc_taskwin;			
}__attribute__((packed)) cc_connect_para;

extern cc_global_para  cc_para_term;
#endif
