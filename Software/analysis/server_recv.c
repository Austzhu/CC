/******************************************* 
 ** Copyright (c) 1998-1999 **公司技术开发部
 ** 文件名: 	Pars_cmd.c
 ** 创建人: 	Austzhu
 ** 日 期: 	2016.3.16
 ** 修改人:
 ** 日　期:
 ** 描　述:
 ** 版　本:
********************************************/
#include "server_recv.h"

//#define rcv_size 50

/**
 * 时间校准处理函数
 * @param  pkt [description]
 * @return     [description]
 */
u32 Timer_Correct(PureCmdArry* pkt)
{
	s8 	set_time[32]={0};
	/* 开发板设置时间格式 */
	// date 051114542016.59  月 日 时 分 年.秒
	/* 把下发的时间格式化成字符串 */
	#ifdef Config_EC_6018
		sprintf(set_time,"date %04u%02u%02u%02u%02u.%02u",pkt->data[1]+2000,pkt->data[2],pkt->data[3],pkt->data[5],pkt->data[6],pkt->data[7]);
	#else
		sprintf(set_time,"date %02u%02u%02u%02u%04u.%02u",pkt->data[2],pkt->data[3],pkt->data[5],pkt->data[6],pkt->data[1]+2000,pkt->data[7]);
	#endif
	debug(DEBUG_chktime,"*******set time command:%s\n",set_time);
	system(set_time);
	return 0;
}
