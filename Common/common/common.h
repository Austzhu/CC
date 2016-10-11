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

/* 检查参数指针是否为NULL,为NULL则退出函数 */
#define assert_param(expr,Message,t)	do{ \
	if(!expr) {  if(Message){  printf("%s,%d:%s\n",__FILE__,__LINE__,Message? Message:"NULL" ); }  return t; } \
}while(0)
// #define assert_param(expr) ((expr) ? (void)0 : assert_failed((char *)__FILE__, __LINE__))
// void assert_failed(s8* file, u32 line);

#endif		//#ifndef __COMMON_H__
