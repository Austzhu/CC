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
#define CFG_COLUMN 	10

enum { left=0, right };

typedef enum {
	enter_0 = 0,	enter_1,				/* 入口段 */
	intensify_0, intensify_1, intensify_2, 	/* 加强段 */
	base_0, base_1 = 7,  					/* 基本段 */
	exit_0, exit_1,							/* 出口段 */
} column_t;


typedef struct light_info_t{
	/* 当前段的亮度指向calc_t 对应段的数据 */
	int32_t *light_current;
	/**
	 *	当前段的亮度对应的pwm等级
	 *	单向车道时只使用pwm_level[0]
	 *	双向车道时 pwm_level[0]为左边pwm, pwm_level[1]为右边pwm
	 */
	int32_t pwm_level[2];
} light_info_t;

typedef struct control_t{
	int32_t Point_flag;
	pthread_t ctrl_thread;
	int32_t pthread_start;
	int32_t group_cnt;
	light_info_t light_info[CFG_COLUMN];

	struct calc_args_t *args;
	struct calc_t *calc;
	struct Single_t 	*single;

	int (*ctrl_exec)(struct control_t*,sql_t *);
	int (*ctrl_getvalues)(struct control_t*);
	int (*ctrl_openloop)(struct control_t*);
	int (*ctrl_output)(struct control_t*,int group, int light);
	int (*ctrl_start)(struct control_t*);
	int (*ctrl_stop)(struct control_t*);
	int (*ctrl_PID)(struct control_t*);
	void (*ctrl_release)(struct control_t*);
} control_t;

extern control_t *control_init(struct control_t *,void *);


#endif
