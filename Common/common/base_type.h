/******************************************************************
** 文件名:	infor_out.c
** Copyright (c) 2012-2014 *********公司技术开发部
** 创建人:	wguilin
** 日　期:	2012.03
** 修改人:
** 日　期:
** 描　述:	数据长度定义
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __BASE_TYPE_H__
#define __BASE_TYPE_H__

#define _un unsigned
#define _vol volatile
#define _con const

#define s8   char
#define s16 short
#define s32 int
#define s64 double

#define u8    _un char
#define u16  _un short
#define u32  _un int
#define ul32 _un long

#define v8    _vol char
#define v16  _vol short
#define v32  _vol int
#define v64  _vol double

#define vu8    _vol _un char
#define vu16  _vol _un short
#define vu32  _vol _un int

#endif
