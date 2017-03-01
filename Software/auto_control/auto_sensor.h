/******************************************************************
** 文件名:	auto_sensor.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __AUTO_SENSOR_H__
#define __AUTO_SENSOR_H__
#include "include.h"
#ifdef Config_UART
#include "UART.h"
#endif
#ifdef Config_KALMAN
#include "kalman.h"
#endif

typedef enum{ traffic = 0, light } value_t;
typedef struct sensor_t {
	int32_t Point_flag;
	int traffic, light;
	#ifdef Config_UART
	uart_t *uart2;
	#endif
	#ifdef Config_KALMAN
	kalman_t *kalman_light;
	kalman_t *kalman_stream;
	#endif
	int (*sensor_get_values)(struct sensor_t*,value_t);
	int (*sensor_get_stream)(struct sensor_t*);
	int (*sensor_get_light)(struct sensor_t*);
	void (*sensor_release)(struct sensor_t*);

} sensor_t;

extern sensor_t *sensor_init(struct sensor_t *);

#endif
