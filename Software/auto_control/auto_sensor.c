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
	assert_param(this,FAIL);

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
			return FAIL;
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
	debug(DEBUG_autosensor,"%s ave = %d\n",\
		buffer == this->traffic ? "stream":"light", ave);
	return ave;
}

static int sensor_pack(struct sensor_t *this,uint8_t *buf)
{
	if(!this || !buf) return FAIL;
	int block = 1000;
	char *pbuf = (char*)buf;
	while(! *buf != 0xff && block--)
		this->uart2->uart_recv(this->uart2,pbuf,1,1000);
	if(block <= 0) return FAIL;
	this->uart2->uart_recv(this->uart2,pbuf+1,2,100000);
	this->uart2->uart_recv(this->uart2,pbuf+3,*(pbuf+2),100000);
	if(SUCCESS != crc_cmp_hight(buf+*(buf+2)+1,buf,*(buf+2)+1)){
		debug(DEBUG_sensor,"CRC check error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int sensor_get_stream(struct sensor_t *this)
{
	assert_param(this,FAIL);
	static int index = 0;
	uint8_t buf[16] = {0xff,0x01,0x04,0x01,0x01,0 };
	crc_hight(buf+5,buf,7);
	this->uart2->uart_send(this->uart2,(char*)buf,7,1000000);
	if(SUCCESS == sensor_pack(this,buf)){
		this->traffic[index++] = buf[5] <<24 | buf[6] << 16 | buf[7]<<8 | buf[8] << 0;
		index %= COLLECT_CNT;
		return SUCCESS;
	}
	return FAIL;
}
static int sensor_get_light(struct sensor_t *this)
{
	assert_param(this,FAIL);
	static int index = 0;
	uint8_t buf[16] = {0xff,0x01,0x04,0x01,0x02,0 };
	crc_hight(buf+5,buf,7);
	this->uart2->uart_send(this->uart2,(char*)buf,7,1000000);
	if(SUCCESS == sensor_pack(this,buf)){
		this->light[index++] = buf[5] <<24 | buf[6] << 16 | buf[7]<<8 | buf[8] << 0;
		index %= COLLECT_CNT;
		return SUCCESS;
	}
	return FAIL;
}

static void sensor_release(struct sensor_t **this,int Is_ptr)
{
	assert_param(this,;);
	#ifdef Config_UART
	DELETE((*this)->uart2,uart_relese);
	#endif
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

	this->sensor_get_values = sensor_get_values;
	this->sensor_get_stream = sensor_get_stream;
	this->sensor_get_light = sensor_get_light;
	this->sensor_release = sensor_release;
	#ifdef Config_UART
	this->uart2 = uart_init(NULL,2,"9600,8,1,N");
	if(!this->uart2) goto out;
	#endif

	return this;
out:
	if(!temp) FREE(this);
	return NULL;
}
