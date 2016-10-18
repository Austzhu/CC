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
#ifndef  __FRAME_H__
#define  __FRAME_H__

#include "base_type.h"


/*****************************************************************
**  struct name:
** 功能描述: 通信帧头信息

	0x68 + address + 0x68 + C + L + DATA + (CS + 0X16)

** 作　者:	wguilin
** 日　期:	201203
** 修　改:
** 日　期:
** 版本	:      V1.0
****************************************************************/
typedef struct {
	UCHAR head;		//=68H
	UCHAR rtua[6];
	UCHAR dep;		//=68H
	UCHAR ctrl;		//控制域C
	UCHAR len;		//data length
	UCHAR data[1];
}faalpkt_t;




SINT basic_makepkt(UCHAR itf,faalpkt_t *pkt);
SINT basic_checkpkt(UCHAR itf, faalpkt_t *pkt);
void basic_clrstat(UCHAR itf, UCHAR * recv);

#define CC_LOGON_CTRL				0xA1
#define CC_LOGON_LEN				1
#define CC_LINE_HEATBEAT			0xA1
#define CC_LINE_HEATBEAT_LEN		2
#define CC_LINE_HEATBEAT_DATA	0x02



//UINT make_logon_package();



#endif
