/******************************************************************
** 文件名:	auto_sensor.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "auto_sensor.h"

static int sensor_get_values(struct sensor_t *this, value_t flags)
{
	assert_param(this,false);

	static int ave = 0;
	int sum = 0, cnt = 0;
	int *buffer = NULL;
	switch(flags){
		case traffic:
			buffer = this->traffic;
			this->sensor_get_stream(this);
			break;
		case light:
			buffer = this->light;
			this->sensor_get_light(this);
			break;
		default:
			return false;
	}
	int max = *buffer, min = max;

	for(int i=0; i++ < COLLECT_CNT; ++buffer){
		if(ave > 0){
			if(*buffer > ave/2 /* 50% */ && *buffer < ave*3/2 /* 150% */){
				*buffer > max ? max = *buffer : (*buffer < min ? min = *buffer : 0);
				sum += *buffer;  ++cnt;
			}
		}else{
			if(*buffer > 0){
				*buffer > max ? max = *buffer : (*buffer < min ? min = *buffer : 0);
				sum +=  *buffer;  ++cnt;
			}
		}
	}
	ave = cnt > 5 ? (sum-max-min)/(cnt-2) : sum/cnt;
	return ave;
}

static int sensor_get_stream(struct sensor_t *this)
{
	assert_param(this,false);
	static int index = 0;
	this->traffic[index++] = rand()%1500;
	index %= COLLECT_CNT;
	return true;
}
static int sensor_get_light(struct sensor_t *this)
{
	assert_param(this,false);
	static int index = 0;
	this->light[index++] = rand()%4500;
	index %= COLLECT_CNT;
	return true;
}

static void sensor_release(struct sensor_t **this,int Is_ptr)
{
	assert_param(this,;);
	if(Is_ptr)  FREE(*this);
}

sensor_t *sensor_init(struct sensor_t *this)
{
	sensor_t *temp = this;
	if(!temp){
		this = malloc(sizeof(sensor_t));
		if(!this)  return NULL;
	}
	bzero(this,sizeof(sensor_t));
	srand(time(NULL));

	this->sensor_get_values = sensor_get_values;
	this->sensor_get_stream = sensor_get_stream;
	this->sensor_get_light = sensor_get_light;
	this->sensor_release = sensor_release;

	return this;
}
