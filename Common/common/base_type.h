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

#define SCHAR 		char
#define UCHAR 	unsigned char
#define CHAR 		char
#define VCHAR 	volatile unsigned char
#define SSHORT 	short
#define USHORT 	unsigned short
#define VSHORT 	volatile unsigned short
#define SINT 		int
#define UINT 		unsigned int
#define VINT 		volatile unsigned int
#define VSINT 		volatile signed int
#define SLONG 	long
#define ULONG 	unsigned long
#define VLONG 	volatile unsigned long
#define INT64 		long long
#define FLOAT 		float
#define DOUBALE 	double

typedef char				s8;
typedef short 				s16;
typedef int 				s32;
typedef double 			s64;
typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned int 			u32;
typedef unsigned long			ul32;
typedef volatile unsigned char 	vu8;
typedef volatile unsigned short 	vu16;
typedef volatile unsigned int 		vu32;
typedef volatile char 			vs8;
typedef volatile short 			vs16;
typedef volatile int 			vs32;


#endif
