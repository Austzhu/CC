
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



#include "format_exchg.h"
#include "err_code.h"

const char cons_strlib_digits[] = {"0123456789ABCDEF"};


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
void hex2toa(UCHAR uc, CHAR *str)
{
	*str++ = cons_strlib_digits[(uc>>4)&0x0f];
	*str++ = cons_strlib_digits[uc&0x0f];

	*str = 0;
}



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
ULONG make_long(UCHAR *str)
{
	VARLONG ul;

	ul.uc[0] = str[0];
	ul.uc[1] = str[1];
	ul.uc[2] = str[2];
	ul.uc[3] = str[3];

	return(ul.ul);
}

// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针
//       nSrcLength - 源字符串长度
// 输出: pDst - 目标数据指针
// 返回: 目标数据长度
//测试过
//int String2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)

//have bug guilin 20120426
int String2Bytes(const char* pSrc, char* pDst, int nSrcLength)

{
	int i;

    for (i = 0; i < nSrcLength; i += 2)
    {
        // 输出高4位
        if ((*pSrc >= '0') && (*pSrc <= '9'))
        {
            *pDst = (*pSrc - '0') << 4;
        }
		else
		{
            *pDst = (*pSrc - 'A' + 10) << 4;
        }

        pSrc++;

        // 输出低4位
        if ((*pSrc>='0') && (*pSrc<='9'))
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }

        pSrc++;
       	// TRACE("%s",pSrc);
        pDst++;
       	// TRACE("%s",pDst);
    }

    // 返回目标数据长度
    return (nSrcLength / 2);

}

// 字节数据转换为可打印字符串
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// 输入: pSrc - 源数据指针
//       nSrcLength - 源数据长度
// 输出: pDst - 目标字符串指针
// 返回: 目标字符串长度
//测试过
int Bytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";    // 0x0-0xf的字符查找表

	int i;

    for (i = 0; i < nSrcLength; i++)
    {
        *pDst++ = tab[*pSrc >> 4];        	// 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];    	// 输出低4位
        pSrc++;
    }

    // 输出字符串加个结束符
    *pDst = '\0'; 		//test

    // 返回目标字符串长度
    return (nSrcLength * 2);
}



