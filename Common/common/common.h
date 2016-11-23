/******************************************************************
** 文件名:	common.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__
#include "base_type.h"
#define MessageBuffersize 512
/* 检查参数指针是否为NULL,为NULL则退出函数 */
#if 0
#define assert_param(expr,Message,rt)	do{ \
	if(!expr){\
		if(Message)\
			printf("%s,%d:%s\n",__FILE__,__LINE__,Message?Message:"");\
		return rt;\
	}\
}while(0)
#endif

#define assert_param(expr,rtn)	do{ \
	if(!expr){\
		printf("**%s,%d**: param \"%s\" error!\n",__FILE__,__LINE__, #expr);\
		return rtn;\
	}\
}while(0)
#define assert_null(expr) assert_param(expr,NULL)
#define assert_fail(expr) assert_param(expr,FAIL)

/* 快速适应sprintf返回拼接好的字符串 */
extern char MessageBuffer[];
#define Asprintf_Strlen   (strlen(MessageBuffer))
#define Asprintf(fmt,args...)  ({bzero(MessageBuffer,MessageBuffersize);\
							snprintf(MessageBuffer,MessageBuffersize,fmt,##args);\
							MessageBuffer;})
/* 大小端转化 */
#define bigend2littlend_2(dd)  (  ((0xff&(dd))<<8)  |  (((dd)>>8)&0xff)  )

#define NEW(type,func,args...)  ({\
	type *ptr = NULL;\
	ptr = func ? func(args) : calloc(1,sizeof(type));\
	ptr;\
})

#define DELETE(ptr,func) do{\
	if(ptr){\
		if((ptr)->func){\
			(ptr)->func(&ptr);\
			if(ptr){ free(ptr); ptr = NULL;  }\
		}else{  free(ptr);  ptr = NULL;  }\
	}\
}while(0)

#define FREE(ptr) do{\
	free(ptr);  ptr = NULL;\
}while(0)

#define INIT(ptr,func,rtn,args...) do{\
	(ptr) = func(args);\
	if(NULL == (ptr)) return rtn;\
}while(0)

#define INIT_FAIL(ptr,func,args...)  INIT(ptr,func,FAIL,##args)


extern char* Hex2Str(char*dest,const u8 *src,int size);
extern u8 *Str2Hex(u8 *dest,const char *src);
extern void msleep(u32 ms);

#endif		//#ifndef __COMMON_H__
