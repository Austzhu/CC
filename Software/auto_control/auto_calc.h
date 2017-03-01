/******************************************************************
** 文件名:	auto_calc.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __AUTO_CALC_H__
#define __AUTO_CALC_H__
#include "include.h"
#include "database.h"
#include "auto_sensor.h"

enum { ONE_WAY = 0, TWO_WAY };

typedef struct calc_args_t{
	int dedign_speed;		//设计时速
	int bothway;			//单双向车道
	int extern_light;		//洞外亮度
	int extern_stream;  		//洞外车流量
	int light_enter[4];    		//入口段
	int light_transit[6];  		//过渡段
	int light_base[2];		//基本段
	int light_exit[2]; 		//出口段
} calc_args_t;

typedef struct calc_t{
	calc_args_t args;
	struct sql_t *sql;
	sensor_t *sensor;
	int32_t Point_flag;
	int32_t (*calc_K)(struct calc_t*);
	int32_t (*calc_light_enter)(struct calc_t*);
	int32_t (*calc_light_transit)(struct calc_t*);
	int32_t (*calc_light_base)(struct calc_t*);
	int32_t (*calc_light_exit)(struct calc_t*);
	void (*calc_release)(struct calc_t*);
} calc_t;

extern calc_t *calc_init(struct calc_t *this);

#endif

