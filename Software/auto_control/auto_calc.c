/******************************************************************
** 文件名:	auto_calc.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "auto_calc.h"

/**
 * 						入口段亮度折减系数k
 *	设计小时交通量N[veh/(h·ln)]			 设计速度vt(km/h)
 *	单向交通	双向交通			120	  	100  	80    	60    	20~40
 *	  ≥1200		  ≥650				0.070 	0.045	0.035	0.022	0.012
 *	  ≤350		  ≤180				0.050 	0.035	0.025	0.015	0.010
 *	注：当交通量在其中间值时，按线性内插取值。
 */
static int calc_K(struct calc_t *this)
{
	assert_param(this,false);
	/*  入口段亮度折减系数K,扩大了100000倍 */
	struct {int speed; int max; int min;} table_K[] ={
		[0] = {.speed = 120,	.max = 7000,  .min = 5000 },
		[1] = {.speed = 100,	.max = 4500,  .min = 3500 },
		[2] = {.speed = 80,	.max = 3500,  .min = 2500 },
		[3] = {.speed = 60,	.max = 2200,  .min = 1500 },
		[4] = {.speed = 40,	.max = 1200,  .min = 1000 }
	};
	int Kj = 0, K = 0;		//计算K时，斜率。
	int Index = 0;
repeat:
	switch(this->args.dedign_speed){
		case 120:	Index = 0;  break;
		case 100:	Index = 1;  break;
		case 80:	Index = 2;  break;
		case 60:	Index = 3;  break;
		default:
			if(this->args.dedign_speed <= 0){
				this->sql->sql_select("select tun_speed,tun_bothway "\
					"from db_tunnel_info;",(void*)&this->args,sizeof(this->args),1,0);
				if(this->args.dedign_speed <= 0) return 0;
				goto repeat;
			}
			Index = 4;
			break;
	}

	Kj = (table_K[Index].max - table_K[Index].min)/\
		(this->args.bothway == 0 ? 850/* 单车道(1200-350) */ : 470/* 双向车道(650-180) */);
	this->args.extern_stream = this->sensor->sensor_get_values(this->sensor,traffic);
	return ((K = table_K[Index].min + Kj*this->args.extern_stream) >\
							table_K[Index].max)  ?  table_K[Index].max  :  K;
}

static int calc_light_enter(struct calc_t *this)
{
	assert_param(this,false);

	int K = this->calc_K(this);
	this->args.extern_light = this->sensor->sensor_get_values(this->sensor,light);
	this->args.light_enter[0] = (K * this->args.extern_light)/100000; 	//入口1段
	this->args.light_enter[1] = this->args.light_enter[0]/2;				//入口2段
	return true;
}

static int calc_light_transit(struct calc_t *this)
{
	assert_param(this,false);
	int Le = this->args.light_enter[0];
	this->args.light_transit[0] = 15*Le/100;
	this->args.light_transit[1] =    5*Le/100;
	this->args.light_transit[2] =    2*Le/100;
	return true;
}


/**
 *  							根据《公路隧道照明设计细则》，基本段亮度表Lin(cd/㎡)
 *	车流量		单向交通		N≥1200veh/(h·ln)		350veh/(h·ln)<N<1200veh/(h·ln）		N≤350veh/(h·ln)
 *				双向交通		N≥650veh/(h·ln)		180veh/(h·ln)<N<650veh/(h·ln）		N≤180veh/(h·ln)
 *				   120				10								6.0								4.5
 *				   100				6.5								4.5								3.0
 *	设计速度	    80				3.5								2.5								1.5
 *				    60				2.5								1.5								1.0
 *				 20-40				1.0								1.0								1.0
 *
 *	1.	当设计速度为100km/h时，中间段亮度可按80km/h对应亮度取值。
 *	2.	当设计速度为120km/h时，中间段亮度可按100km/h对应亮度取值。
 *	3.	对车流量在350～1200辆/h之间时采用了内插法，使得调光亮度变化的连续性更好
 *	4.	针对显色指数Ra≥65，色温介于3500～6500K的LED光源用于隧道基本照明时，
 *		亮度可按上表所列标准的50%取值，但不低于1cd/㎡。
 *	5.	行人与车辆混合通行的隧道，中间段亮度不应小于2.0cd/m2。
 *	6.	单向交通，以设计速度通过隧道的行车时间超过135s时，宜分为2个照明段，
 *		前30s为Lin,后面为80%（50%）且不低于1.0 cd/m2。
 *	7.	隧道内交通分流段、全流段的亮度不宜低于中间段亮度的3倍。
 *
 */
static int calc_light_base(struct calc_t *this)
{
	assert_param(this,false);

	/* 基本段亮度计算值，扩大1000倍。 */
	struct { int speed; int max; int min;} table_base[] = {
		[0] = { .speed = 120, 	.max = 10000, 	.min = 4500 },
		[1] = { .speed = 100, 	.max = 10000, 	.min = 4500 },
		[2] = { .speed = 80,   	.max = 10000, 	.min = 4500 },
		[3] = { .speed = 60,   	.max = 10000, 	.min = 4500 },
		[4] = { .speed = 40,   	.max = 10000, 	.min = 4500 }
	};
	int Index = 0;
repeat:
	switch(this->args.dedign_speed){
		case 120:	Index = 0;  break;
		case 100:	Index = 1;  break;
		case 80:	Index = 2;  break;
		case 60:	Index = 3;  break;
		default:
			if(this->args.dedign_speed <= 0){
				this->sql->sql_select("select tun_speed,tun_bothway "\
					"from db_tunnel_info;",(void*)&this->args,sizeof(this->args),1,0);
				if(this->args.dedign_speed <= 0) return 0;
				goto repeat;
			}
			Index = 4;
			break;
	}
	/*   系数扩大了1000倍。 */
	int K = (table_base[Index].max - table_base[Index].min)/\
		(this->args.bothway == 0 ? 850/* 单车道(1200-350) */ : 470/* 双向车道(650-180) */);
	this->args.light_base = (table_base[Index].min + K * this->args.extern_stream)/1000  ;
	if(this->args.light_base > table_base[Index].max)
		this->args.light_base = table_base[Index].max;
	return true;
}

static int calc_light_exit(struct calc_t *this)
{
	assert_param(this,false);

	int Lb = this->args.light_base;
	this->args.light_exit[0] = 3 * Lb;
	this->args.light_exit[1] = 5 * Lb;
	return true;
}

static void calc_release(struct calc_t **this,int Is_ptr)
{
	assert_param(this,;);
	calc_t *_this = Is_ptr ? *this : (calc_t *)this;
	DELETE(_this->sql,sql_release);
	DELETE(_this->sensor,sensor_release,true);
	if(Is_ptr)  FREE(*this);
}


calc_t *calc_init(struct calc_t *this)
{
	calc_t *temp = this;
	if(!temp){
		this = malloc(sizeof(calc_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(calc_t));
	this->sql = sql_Init(NULL);
	if(!this->sql)  goto out;
	this->sensor = sensor_init(NULL);
	if(!this->sensor) goto out1;

	this->sql->sql_select("select tun_speed,tun_bothway "\
		"from db_tunnel_info;",(void*)&this->args,sizeof(this->args),1,0);

	this->calc_K 			= calc_K;
	this->calc_light_enter	= calc_light_enter;
	this->calc_light_transit = calc_light_transit;
	this->calc_light_base	= calc_light_base;
	this->calc_light_exit	= calc_light_exit;
	this->calc_release		= calc_release;

	return this;
out1:
	if(this->sql)  DELETE(this->sql,sql_release);
out:
	if(!temp) FREE(this);
	return NULL;
}
