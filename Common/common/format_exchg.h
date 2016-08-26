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


#ifndef __FORMAT_EXCHG_H__
#define __FORMAT_EXCHG_H__

#include "base_type.h"

void hex2toa(UCHAR uc, CHAR *str);
ULONG make_long(UCHAR *str);


//const char cons_strlib_digits[] = {"0123456789ABCDEF"};

/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
typedef union {
	ULONG ul;
	UCHAR uc[4];
} VARLONG;




//fuction

// 可打印字符串转换为字节数据    
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针    
//       nSrcLength - 源字符串长度  
// 输出: pDst - 目标数据指针    
// 返回: 目标数据长度
//测试过
//int String2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);   
int String2Bytes(const char* pSrc, char* pDst, int nSrcLength);   


// 字节数据转换为可打印字符串    
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// 输入: pSrc - 源数据指针    
//       nSrcLength - 源数据长度    
// 输出: pDst - 目标字符串指针    
// 返回: 目标字符串长度    
//测试过
int Bytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);

#endif
