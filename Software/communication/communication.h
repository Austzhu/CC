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
#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__
#include "include.h"

struct cmd_type{ 
	UCHAR cmdname;
};

extern u32 WhetherCMDExist(UCHAR type,UCHAR cmd);

#define PLC_UNI_CAST_CMD 		0x01
#define PLC_GRP_CAST_CMD 		0x02
#define PLC_BROADCAST_CMD 	0x03
#define PLC_REPORT_CMD		0x04
#define PLC_NODE_CFG_CMD 		0x06
#define PLC_NODE_SET_CMD 		0x07
#define PLC_NODE_GET_CMD 		0x08
#define PLC_NODE_DEL_CMD 		0x09
#define PLC_ADDR_CMD_CMD 		0x10
#define PLC_QUERY_CC_CMD 		0x11
#define PLC_GRP_DEL_CMD 		0x12
#define PLC_RESET_CMD 		0x13
#define PLC_CONDIF_OVER_CMD 	0x14

#define GPRS_TEST1_CMD 		0x31
#define GPRS_TEST2_CMD 		0x32
#define GPRS_TEST3_CMD 		0x33
#define GPRS_TEST4_CMD 		0x34
#define GPRS_TEST5_CMD 		0x35

#define CC_ETH_ShortACK_CMD 	0x51
#define CC_ETH_LongContex_CMD 	0x52
#define CC_ETH_TEST3_CMD 		0x53
#define CC_ETH_TEST4_CMD 		0x54

#define CC_SERVER_ACK_CMD		0x80
#define CC_LOGON_CMD 		0xA1
#define CC_OPRATE_CMD 		0xA2
#define CC_PARA_SET_CMD 		0xA3
#define CC_UPDATE_CMD 		0xA4
#define CC_XXXX_CMD  	           	0xA5		
#define CC_XXX_CMD			0xA6		
#define CC_LOGON_CMD_test		0xA7		//0xA0

#define CC_BACK_SET_1_CMD 		0xB1
#define CC_BACK_SET_2_CMD 		0xB2
#define CC_BACK_SET_3_CMD 		0xB3		
#define CC_BACK_SET_4_CMD 		0xB4		
#define CC_BACK_SET_5_CMD 		0xB5		
#define CC_BACK_SET_6_CMD 		0xB6		

#define CC_TIME_CTRL_CMD 		0xC1
#define CC_GEOGRA_CMD 		0xC2
#define CC_ARYS_CTRL_CMD 		0xC3
#define CC_xxxxx_CMD 		0xC4		
#define CC_xxx_x_CMD 	      	0xC5		
#define CC_x_xxx_CMD 		0xC6		

#define CC_METER_READ_CMD 	0xD1
#define CC_METER_SET_CMD 		0xD2
#define CC_METER_RD_EXTR_CMD 	0xD3
#define CC_METER_SET_EXTR_CMD 	0xD4		
#define CC_XX_XXXX_CMD 		0xD5		
#define CC_XXX_XXX_CMD 		0xD6		

#define CC_DIDO_SET_CMD 		0xE1
#define CC_DIDO_READ_CMD 		0xE2
#define CC_DIDO_XX_X_CMD 		0xE3		
#define CC_DIDO_XXX_X_CMD 		0xE4		
#define CC_DIDO_XXXX_X_CMD 	0xE5		
#define CC_DIDO_XXXXX_X_CMD 	0xE6		

#define CC_ALARM_SET_1_CMD 	0xF1
#define CC_ALARM_SET_2_CMD 	0xF2
#define CC_ALARM_SET_3_CMD 	0xF3		
#define CC_ALARM_SET_4_CMD 	0xF4		
#define CC_ALARM_SET_5_CMD 	0xF5		
#define CC_ALARM_SET_6_CMD 	0xF6		
#define ALL_SUPPT_CMD_END    	0xFF

/*********************************************************************/
#define CC_METER_Broadcast 		0x03 		//广播命令，test



#endif
