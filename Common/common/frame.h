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

typedef struct {
	unsigned char head;		//=68H
	unsigned char rtua[6];
	unsigned char dep;			//=68H
	unsigned char ctrl;			//控制域C
	unsigned char len;			//data length
	unsigned char data[0];
} faalpkt_t;




#endif
