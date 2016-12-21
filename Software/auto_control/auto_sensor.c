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
static int sensor_get_values(struct sensor_t *this, value_t value)
{
	assert_param(this,0);

	int max, min, sum = 0;
	int *buffer = (value == traffic) ? this->traffic : this->light;
	max=min = *buffer;
	for(int i=0; i++< COLLECT_CNT; ){
		sum +=  *buffer;
		if(*buffer > max) max = *buffer;
		else if(*buffer < min)  min = *buffer;
		++buffer;
	}
	return (sum-max-min)/(COLLECT_CNT-2);
}

static void sensor_release(struct sensor_t **this,int Is_ptr)
{
	assert_param(this,;);
	sensor_t *_this = Is_ptr ? *this : (sensor_t *)this;
	_this->sensor_start = 0;
	pthread_join(_this->sensor_thread, NULL);
	if(Is_ptr)  FREE(*this);

}

void *sensor_pthread(void *args)
{
	sensor_t *this = args;
	srand(time(NULL));
	static int index = 0;
	while(this->sensor_start){
		pthread_mutex_lock(&this->sensor_lock);
		this->traffic[index] = rand();
		this->light[index] = rand();
		pthread_mutex_unlock(&this->sensor_lock);
		++index;
		index %= COLLECT_CNT;
		msleep(1000);
	}
	pthread_exit(NULL);
}

sensor_t *sensor_init(struct sensor_t *this)
{
	sensor_t *temp = this;
	if(!temp){
		this = malloc(sizeof(sensor_t));
		if(!this)  return NULL;
	}
	bzero(this,sizeof(sensor_t));
	pthread_mutex_init(&this->sensor_lock,NULL);

	this->sensor_get_values = sensor_get_values;
	this->sensor_release = sensor_release;
	this->sensor_start = 1;
	if(0 != pthread_create(&this->sensor_thread,NULL,sensor_pthread,this))
		goto out;
	return this;
 out:
 	if(!temp) FREE(this);
 	return NULL;
}
