/******************************************************************
** 文件名:	auto_control.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.12
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "auto_control.h"

static int control_getvalues(control_t *this)
{
	assert_param(this,FAIL);
	int res = SUCCESS;

	res |= this->calc->calc_light_enter(this->calc);
	res |= this->calc->calc_light_transit(this->calc);
	res |= this->calc->calc_light_base(this->calc);
	res |= this->calc->calc_light_exit(this->calc);

	return (res==SUCCESS) ? SUCCESS : FAIL;
}


static void get_gid_info(control_t *this)
{
	assert_param(this,;);

	sql_t *sql = this->calc->sql;
	if(!sql) {
		debug(DEBUG_auto,"sql is NULL in class auto_calc!\n");
		return ;
	}
	/* get count of group */
	sql->sql_select("select count(distinct gid) from db_group_info;",\
						(char*)&this->gid_info.gid_cnt,sizeof(int),1,0);
	if(this->gid_info.gid_cnt <= 0){
		debug(DEBUG_auto,"select gid info from db error!\n");
		return ;
	}
	/* get gid and pwm values */
	sql->sql_select("select distinct gid,pwm_value from db_group_info order by gid;",\
		(char*)this->gid_info.g_val,sizeof(this->gid_info.g_val[0]),gid_cnt_max,0);

}

static int get_gid_pwm(control_t *this)
{
	assert_param(this,FAIL);
	sql_t *sql = this->calc->sql;
	if(!sql) {
		debug(DEBUG_auto,"sql is NULL in class auto_calc!\n");
		return FAIL;
	}
	for(int i=0;i<this->gid_info.gid_cnt;++i){
		sql->sql_select(Asprintf("select light_min,light_max from "\
			"db_index_pwm where level=%d;",this->gid_info.g_val[i].pwm),\
			(char*)this->gid_info.g_val[i].limit,sizeof(int)*2,1,0);
	}
	return SUCCESS;
}

#if 0
static int *get_light_buf(control_t *this, int index,int *size)
{
	switch(index){
		case 0:	*size = 4;  return this->calc->args.light_enter;
		case 1:	*size = 6;  return this->calc->args.light_transit;
		case 2:	*size = 1;  return &this->calc->args.light_base;
		case 3:	*size = 2;  return this->calc->args.light_exit;
		default:*size = 0;  return NULL;
	}
}
#endif

/* 判断亮度是否超出阀值 */
#define Is_border(pval,_i) ( pval > this->gid_info.g_val[_i].limit[1] || pval< this->gid_info.g_val[_i].limit[0] )
/**
 * 限幅控制
 * 返回值：0未超出幅值, 1超出幅值
 * 0~8 bit 入口段
 * 9~15 bit 过渡段
 * 16~24 bit 基本段
 * 25~32 bit 出口段
 */

static int control_lightcmp(control_t *this)
{
	assert_param(this,FAIL);
	int res = 0, index = 0;
	int *pbuf[] ={
		this->calc->args.light_enter,
		this->calc->args.light_transit,
		this->calc->args.light_base,
		this->calc->args.light_exit
	};

	for(int i=0; i<this->gid_info.gid_cnt; ++i){

		index = this->gid_info.g_val[i].gid/10;
		if(index > sizeof(pbuf)/sizeof(pbuf[0])){
			debug(DEBUG_auto,"array for light error! index=%d.\n",index);
			return FAIL;
		}
		switch(this->gid_info.g_val[i].gid%10){
			case 0:	res |= Is_border(pbuf[index][0], i) ? 1<<(index*8+0) : 0;	break;
			case 1:	res |= Is_border(pbuf[index][1], i) ? 1<<(index*8+1) : 0;	break;
			case 2:	res |= Is_border(pbuf[index][2], i) ? 1<<(index*8+2) : 0;	break;
			case 3:	case 4:	break;
			case 5:	res |= Is_border(pbuf[index][5], i) ? 1<<(index*8+5) : 0;	break;
			case 6:	res |= Is_border(pbuf[index][6], i) ? 1<<(index*8+6) : 0;	break;
			case 7:	res |= Is_border(pbuf[index][7], i) ? 1<<(index*8+7) : 0;	break;
			default:break;
		}	//enf of switch
	}	// end of for(int i=0;
	debug(DEBUG_auto,"*****return res vales=0x%08x.\n",res);
	return res;
}

static int control_limit(control_t *this)
{
	assert_param(this,FAIL);
	/* get infomaion for group  */
	get_gid_info(this);
	if(this->gid_info.gid_cnt <= 0){
		debug(DEBUG_autocontrol,"get group info error!\n");
		return FAIL;
	}
	/* get current pwm for every group */
	get_gid_pwm(this);
	/* compare every group light */
	return control_lightcmp(this);
}

static int control_openloop(control_t *this)
{
	assert_param(this,FAIL);
	int limit = 0, light = 0;
	int *light_buf[] ={
		this->calc->args.light_enter,
		this->calc->args.light_transit,
		this->calc->args.light_base,
		this->calc->args.light_exit
	};
	sql_t *sql = this->calc->sql;
	if(!sql) {
		debug(DEBUG_auto,"sql is NULL in class auto_calc!\n");
		return FAIL;
	}
	/* get values */
	this->control_getvalues(this);
	limit = this->control_limit(this);

	if(limit){
		for(int i=0,j=1; i < 32; ++i, j <<= 1){
			if(limit & j){
				light = 0;
				sql->sql_select(Asprintf("select level from db_index_pwm where %d "\
					"between light_min and light_max;",light_buf[i/8][i%8]), (char*)&light,sizeof(int),1,0);
				if(light)
					this->control_output(this,10*i/8+i%8, light);
			}	// end of if(limit & j){
		}	// end of for(int i=0,j=1;
	}	// end of if(limit){
	return SUCCESS;
}

static int control_output(control_t *this,int group,int light)
{
	assert_param(this,FAIL);

	return this->single->sin_open(this->single,cmd_group,group<<24,light<<8);

}

static void *control_pthread(void *args)
{
	if(!args){
		debug(DEBUG_autocontrol,"In control thread args error!\n");
		return NULL;
	}
	control_t *this = (control_t*)args;
	int sensor_cnt = 0;
	this->calc->sql->sql_select("select tun_sensor from db_tunnel_info;",(char*)&sensor_cnt,sizeof(int),1,0);
	while(this->pthread_start ){
		/* do something */
		if(this->calc->args.bothway == 0 && sensor_cnt < 2 ){	//单向车道
			this->control_openloop(this);
		}
		msleep(2000);
	}
	pthread_exit(NULL);
}

static int control_start(control_t *this)
{
	assert_param(this,FAIL);

	this->pthread_start = 1;
	if(pthread_create(&this->control_thread, NULL,control_pthread,this)){
		debug(DEBUG_autocontrol,"In class auto control pthread create error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int control_stop(control_t *this)
{
	assert_param(this,FAIL);
	if(this->pthread_start){
		this->pthread_start = 0;
		pthread_join(this->control_thread,NULL);
	}
	return SUCCESS;
}

static int control_PID(control_t *this)
{
	return FAIL;
}

static void control_release(control_t **this, int Is_ptr)
{
	assert_param(this,;);
	control_t *_this = Is_ptr ? *this : (control_t *)this;
	if(_this->control_stop(_this) == FAIL){
		debug(DEBUG_autocontrol,"auto control stop thread error!\n");
	}
	DELETE(_this->calc, calc_release,true);
	if(Is_ptr)  FREE(*this);
}

control_t *control_init(struct control_t *this, void *single)
{
	control_t *temp = this;
	if(!temp){
		this = malloc(sizeof(control_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(control_t));
	this->single = single;
	if(!this->single) goto out;
	this->calc = calc_init(NULL);
	if(!this->calc) goto out;

	this->control_getvalues 	= control_getvalues;
	this->control_limit  		= control_limit;
	this->control_openloop 	= control_openloop;
	this->control_output 		= control_output;
	this->control_start   		= control_start;
	this->control_stop   		= control_stop;
	this->control_PID    		= control_PID;
	this->control_release 		= control_release;
	return this;
out:
	debug(DEBUG_autocontrol,"control init error!\n");
	if(!temp) FREE(this);
	return NULL;
}
