/******************************************************************
** 文件名:	auto_control.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __AUTO_CONTROL_H__
#define __AUTO_CONTROL_H__
#include "auto_calc.h"
#include "single.h"
#define gid_cnt_max  32		//最多有多少个组
typedef enum { enter = 1, transit, base, outway } output_t;
typedef struct {
	int gid_cnt;
	struct { int gid; int pwm; int limit[2];}g_val[gid_cnt_max];
} gid_info_t;
typedef struct control_t{
	struct calc_t *calc;
	struct Single_t 	*single;
	int pthread_start;
	gid_info_t gid_info;
	pthread_t control_thread;

	int (*control_getvalues)(struct control_t*);
	int (*control_limit)(struct control_t*);
	int (*control_openloop)(struct control_t*);
	int (*control_output)(struct control_t*,int group, int light);
	int (*control_start)(struct control_t*);
	int (*control_stop)(struct control_t*);
	int (*control_PID)(struct control_t*);
	void (*control_release)(struct control_t**,int);
} control_t;

extern control_t *control_init(struct control_t *,void *);


#endif
