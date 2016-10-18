/******************************************************************
** 文件名:
** Copyright (c) 1998-1999 *********公司技术开发部
** 创建人:
** 日　期:
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:
*******************************************************************/
#include "communication.h"

const struct cmd_type recv_cmd_types[][255]={
	//0 载波接口相关   	协议定义
	{
		{0X42},
		{0X43},
		{0X45},
		{0X47},
		{0X48},
		//{CMD_CONDIF_OVER},
		{0xff},//结束标志
	},
	//1 ---gprs/sms 接口相关  inner  非协议定义
	{	//可拆解出诸如:     gprs\ether net interface
		{0x31},				//XXXX_START_ADD  0X30--0X7F
		{0x42},
		{0x4F},				//XXXX_END_ADD  0X30--0X6F
		{0xff},
	},
	//2 ----CC-08集中器本身相关	协议定义
	{
		//0x80
		{CC_SERVER_ACK_CMD},	//0x80 :pc return cmd for cc

		//{0xA0},			//tech no exist
		//{CC_LOGON_CMD},		//tech no exist
		{CC_OPRATE_CMD},
		{CC_PARA_SET_CMD},
		{CC_UPDATE_CMD},
		{CC_LOGON_CMD_test},	//NET_START_ADD  "0XA1--0XCEN  + 0XF0--0XFE"

		{0xC0},
		{CC_TIME_CTRL_CMD},
		{CC_GEOGRA_CMD},
		{CC_ARYS_CTRL_CMD},

		{0xF1},					//ALARM RELATED	0XF0--0XFF
		{0xF2},
		{0xF3},
		{0xFE},					//MAX__END 0XFE

		{0xFF},
	},
	//3 -----   485接口相关		协议定义
	{
		{0xD0},
		{CC_METER_READ_CMD},	//485_device_start		0XD0--0XEF
		{CC_METER_SET_CMD},
		{CC_METER_RD_EXTR_CMD},
		{CC_METER_SET_EXTR_CMD},

		{0xE0},
		{CC_DIDO_SET_CMD},
		{CC_DIDO_READ_CMD},
		{0X03}, 				//Austzhu 2016.3.29 广播命令
		{0x02},				//Austzhu 2016.4.13 单播命令
		{0x01},

		{0xEE},				//NET__MAX__END	0XEE
		{0xff},//结束标志
	},
	//4      网络接口相关		协议未定义
	{
		{0x51},
		{0x7F},
		{0xff},			//结束标志
	}
};


UINT WhetherCMDExist(UCHAR type,UCHAR cmd)
{
	UCHAR i=0;
	if(type>sizeof(recv_cmd_types)/sizeof(recv_cmd_types[0])) {
		debug(DEBUG_LOCAL_TASK,"no cmd %d in type :%d",cmd,type);
		return TASK_NOZONE_EXIST_ERR;	 //CMD 不存在
	}

	while(recv_cmd_types[type][i].cmdname  !=  0xff){

		if(cmd  ==  recv_cmd_types[type][i].cmdname) {
			debug(DEBUG_LOCAL_TASK,"recv_cmd_types[type][%d].cmdname is %#x , cmd is %#x\n",
										i,recv_cmd_types[type][i].cmdname,cmd);
			return SUCCESS;//成功
		}
		++i;
	}
	return TASK_NOCMD_EXIST_ERR;
}


