/******************************************************************
** 文件名:	update.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.11
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __UPDATE_H__
#define __UPDATE_H__
#include "include.h"

#define SOH      0x01
#define STX       0x02
#define EOT      0x04
#define ACK      0x06
#define NAK     0x15
#define CAN     0x18
#define CTRLZ 0x1A
#define UPACKSIZE   128

typedef struct{
	int update_size;
	int update_frame;        //一共有多少帧数据
	int update_cframe;      //当前第几帧数据
	int update_fd;
	int update_isImageA;
	int update_isCoordi;
	int update_repeat;
	int update_checkType;
	int update_pend;
	int update_timeout;
	int update_ok;
} update_Info_t;

typedef struct{
	u8 header;
	u8 Coor_addr;
	u8 single_addr[2];
	u8 frame[2];          //第一个字节存第几帧数据，第2个字节存第一个字节的反码
	u8 data[UPACKSIZE];
	u8 CRC16[2];        //低在前，高位在后
} package_t;

struct Single_t;
typedef struct update_t {
	update_Info_t info;
	package_t package;
	struct Single_t *subclass;      //继承update的子类
	int (*update_start)(struct update_t*,int addr,char Image);
	int (*update_slave)(struct update_t *);
	int (*update_send_package)(struct update_t*);
	u8 (*update_recv_onebyte)(struct update_t*);
	void (*update_release)(struct update_t**);
} update_t;

extern update_t *update_init(struct Single_t *subclass);
#endif

