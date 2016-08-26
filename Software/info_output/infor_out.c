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
#include "infor_out.h"

/*
* @name 记录系统日志函数
* @func system_log
* @author K.Shu <kshu.zhu@ztnet.com.cn>

* @param $msg:  需要记录的信息
* @return none
* @global C('CD_LOG_FILE_PATH') log日志的位置
* @include none
* @others none
*/
void system_log(char *msg)
{
	// $file = fopen(C('CD_LOG_FILE_PATH'),"a");
	// $msg = date('y-m-d h:i:s',time())." > ".$msg."\n";
	// fwrite($file, $msg);
		// fclose($file);
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
void print_hexbuf(UCHAR *buf, UINT len)
{
	CHAR str[64];

	UINT i;
	CHAR *pc;

	pc = str;
	*pc = 0;

	for(i=1; i<=len; i++) {
		hex2toa(*buf++, pc);
		pc += 2;

		*pc++ = ' ';
		*pc = 0;

		if(0 == (i&0x0f)) {		
			debug(DEBUG_LOCAL_FORMAT_CH,"%s\r\n", str);		      	
			pc = str;
			*pc = 0;
		}
		else if(0 == (i&0x07)) {
			*pc++ = ':';
			*pc++ = ' ';
			*pc = 0;
		}
	}

	if(0 != strlen(str)) {
		debug(DEBUG_LOCAL_FORMAT_CH,"%s\r\n", str);
	}

}


UINT print_logo(UCHAR level, char *format, ...)
{
	va_list va;


#ifdef MULTIITF_LOGO
	if(0 == cur_logoitf) {
		va_start(va, format);
		vprintf(format, va);
		va_end(va);
	}
	else {
		faalpkt_t *pkt = (faalpkt_t *)logoprint_buf;

		va_start(va, format);
		vsprintf((char *)pkt->data, format, va);
		va_end(va);

		faal_sendpkt(cur_logoitf, pkt);
	}
#else
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
	return SUCCESS;
#endif
}

