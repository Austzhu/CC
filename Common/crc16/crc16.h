/******************************************************************
** 文件名:	CRC.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2012.11
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef  __CRC_H__
#define __CRC_H__
#include "include.h"

extern int crc_hight(unsigned char *crc,unsigned char *puchMsg, int usDataLen);
extern int crc_low(unsigned char *crc,unsigned char *puchMsg, int usDataLen);
extern int crc_cmp_hight(uint8_t *crc,  uint8_t *puchMsg,  int usDataLen);
extern int crc_cmp_low(uint8_t *crc,  uint8_t *puchMsg,  int usDataLen);


#endif
