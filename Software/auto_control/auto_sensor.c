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

static int sensor_pack(struct sensor_t *this,uint8_t *buf)
{
	if(!this || !buf) return FAIL;

#ifdef Config_UART
	int block = 200;
	char *pf = (char*)buf;
	*pf = 0;
	do{
		this->uart2->uart_recv(this->uart2,pf,1,10000);
	}while(*buf != 0xff && block-- );

	if(block <= 0) {
		debug(DEBUG_sensor,"recv header error!\n");
		return FAIL;
	}
	this->uart2->uart_recv(this->uart2,++pf,2,100000);	++pf;
	this->uart2->uart_recv(this->uart2,pf+1,(uint32_t)*pf,100000);
	//display_pack("uart 2 recv",(char*)buf, (int)*pf+3);
	if(SUCCESS != crc_cmp_hight(buf+*pf+1,buf,*pf+1)){
		debug(DEBUG_sensor,"CRC check error!\n");
		return FAIL;
	}
#endif
	return SUCCESS;
}

static int sensor_get_values(struct sensor_t *this, value_t flags)
{
	assert_param(this,FAIL);
	int *buffer = NULL;
	int Z_last = 0;
	kalman_t *kal = NULL;
	switch(flags){
		case traffic:
			buffer = &this->traffic;
			Z_last = *buffer;
			kal = this->kalman_stream;
			this->sensor_get_stream(this);
			break;
		case light:
			buffer = &this->light;
			Z_last = *buffer;
			kal = this->kalman_light;
			this->sensor_get_light(this);
			break;
		default:
			return FAIL;
	}
	if(!buffer || !kal) return FAIL;

	float X_last = kal->kal_X;
	float X_now = kal->kal_filter(kal,(float)*buffer);
	debug(DEBUG_sensor,"last:%d,kalman:%f\t △ G=%d,"\
		"△ X=%f\n",Z_last,X_now, *buffer-Z_last,X_now-X_last);
	return (int)(X_now+0.5);
}


static int sensor_get_stream(struct sensor_t *this)
{
	assert_param(this,FAIL);
	#ifdef Config_UART
	uint8_t buf[16] = {0xff,0x01,0x04,0x01,0x01,0 };
	crc_hight(buf+5,buf,5);
	this->uart2->uart_send(this->uart2,(char*)buf,7,1000000);
	if(SUCCESS == sensor_pack(this,buf)){
		this->traffic = buf[5] <<24 | buf[6] << 16 | buf[7]<<8 | buf[8] << 0;
		return SUCCESS;
	}
	#endif
	return FAIL;
}
static int sensor_get_light(struct sensor_t *this)
{
	assert_param(this,FAIL);
#ifdef Config_UART
	uint8_t buf[16] = {0xff,0x01,0x04,0x01,0x02,0 };
	crc_hight(buf+5,buf,5);
	this->uart2->uart_send(this->uart2,(char*)buf,7,1000000);
	if(SUCCESS == sensor_pack(this,buf)){
		this->light = buf[5] <<24 | buf[6] << 16 | buf[7]<<8 | buf[8] << 0;
		return SUCCESS;
	}
#endif
	return FAIL;
}

static void sensor_release(struct sensor_t *this)
{
	if(!this ) return ;
	#ifdef Config_UART
	DELETE(this->uart2,uart_relese,CFG_SENSOR);
	#endif
	#ifdef Config_KALMAN
	DELETE(this->kalman_light,kal_release);
	DELETE(this->kalman_stream,kal_release);
	#endif
	if(this->Point_flag)
		FREE(this);
}

sensor_t *sensor_init(struct sensor_t *this)
{
	sensor_t *const temp = this;
	if(!temp){
		this = malloc(sizeof(sensor_t));
		if(!this)  return NULL;
	}
	bzero(this,sizeof(sensor_t));
	this->Point_flag = (!temp)?1:0;

	this->sensor_get_values = sensor_get_values;
	this->sensor_get_stream = sensor_get_stream;
	this->sensor_get_light = sensor_get_light;
	this->sensor_release = sensor_release;

	#ifdef Config_UART
	this->uart2 = uart_init(NULL,CFG_SENSOR,"9600,8,1,N");
	if(!this->uart2) goto out;
	#endif
	#ifdef Config_KALMAN
	this->kalman_light = kalman_init(NULL,0,0,0,0);
	this->kalman_stream = kalman_init(NULL,0,0,250,850);
	#endif
	return this;
out:
	if(!temp) FREE(this);
	return NULL;
}












#if 0
static int sensor_get_values(struct sensor_t *this, value_t flags)
{
	assert_param(this,FAIL);

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

#if 0
	int sum = 0, cnt = 0;
	int max = *buffer, min = max;
	for(int i=0; i++ < COLLECT_CNT && ave != NULL; ++buffer){
		#if 1
		if(*buffer > 0){
			(*buffer > max)  ?  (max = *buffer) : (*buffer < min ? (min = *buffer) : 0) ;
			sum +=  *buffer;  ++cnt;
		}
		#else
		if(*ave > 0){
			if(*buffer > *ave/4 /* 25% */ && *buffer < *ave*5/4 /* 125% */){
				(*buffer > max)  ?  (max = *buffer) : ( (*buffer < min) ? (min = *buffer) : 0);
				sum += *buffer;  ++cnt;
			}
		}else{
			if(*buffer > 0){
				(*buffer > max)  ?  (max = *buffer) : (*buffer < min ? (min = *buffer) : 0) ;
				sum +=  *buffer;  ++cnt;
			}
		}
		#endif
	}

	*ave =(int) cnt > 5 ? (sum-max-min)/(cnt-2) : sum/ ((cnt<=0) ? 1 : cnt);
	debug(DEBUG_autosensor,"cnt =%d,%s ave = %d\n",cnt,\
		(ave == ave_array) ? "stream":"light", *ave);
#endif

	static int Z_last = 0, kman_last = 0;
	int kman_now = sensor_Kman((float)*buffer);
	//int kman_now = sensor_Kman(1000.0);
	debug(DEBUG_sensor,"getvalue:%d,kalman:%d   △ z=%d,△ k=%d\n",\
				*buffer, kman_now,*buffer-Z_last,kman_now-kman_last);
	Z_last = *buffer;
	kman_last = kman_now;

	return kman_now;
}
#endif
