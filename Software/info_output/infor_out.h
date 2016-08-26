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
#ifndef __INFOR_OUT_H__
#define __INFOR_OUT_H__
#include "include.h"

void print_hexbuf(UCHAR *buf, UINT len);
UINT print_logo(UCHAR level, char *format, ...);
#define LOGOLVL_SHORT 		0
#define LOGOLVL_ALM			1
#endif
