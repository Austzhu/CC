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
	assert_param(this,false);
	int res = true;

	res &= this->calc->calc_light_enter(this->calc);
	res &= this->calc->calc_light_transit(this->calc);
	res &= this->calc->calc_light_base(this->calc);
	res &= this->calc->calc_light_exit(this->calc);

	return (res==true) ? true : false;
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
	assert_param(this,false);
	sql_t *sql = this->calc->sql;
	if(!sql) {
		debug(DEBUG_auto,"sql is NULL in class auto_calc!\n");
		return false;
	}
	for(int i=0;i<this->gid_info.gid_cnt;++i){
		sql->sql_select(Asprintf("select light_min,light_max from "\
			"db_index_pwm where level=%d;",this->gid_info.g_val[i].pwm),\
			(char*)this->gid_info.g_val[i].limit,sizeof(int)*2,1,0);
	}
	return true;
}

/**
 * 限幅控制
 * 返回值：0未超出幅值, 1超出幅值
 * 0~8 bit 入口段
 * 9~20 bit 过渡段
 * 21~24 bit 基本段
 * 25~32 bit 出口段
 */

static int control_lightcmp(control_t *this)
{
	int res = 0;

	return res;
}

static int control_limit(control_t *this)
{
	assert_param(this,false);
	/* get infomaion for group  */
	get_gid_info(this);
	if(this->gid_info.gid_cnt <= 0){
		debug(DEBUG_autocontrol,"get group info error!\n");
		return false;
	}
	/* get current pwm for every group */
	get_gid_pwm(this);
	/* compare every group light */
	return control_lightcmp(this);
}

static int control_openloop(control_t *this)
{
	assert_param(this,false);

	return false;
}

static int control_output(control_t *this,int group)
{
	assert_param(this,false);
	return false;
}

static void *control_pthread(void *args)
{
	if(!args){
		debug(DEBUG_autocontrol,"In control thread args error!\n");
		return NULL;
	}
	control_t *this = (control_t*)args;

	while(this->pthread_start){
		/* do something */
		msleep(5000);
		printf("#");fflush(stdout);
	}
	pthread_exit(NULL);
}

static int control_start(control_t *this)
{
	assert_param(this,false);

	this->pthread_start = 1;
	if(pthread_create(&this->control_thread, NULL,control_pthread,this)){
		debug(DEBUG_autocontrol,"In class auto control pthread create error!\n");
		return false;
	}
	return true;
}

static int control_stop(control_t *this)
{
	assert_param(this,false);
	if(this->pthread_start){
		this->pthread_start = 0;
		pthread_join(this->control_thread,NULL);
	}
	return true;
}

static int control_PID(control_t *this)
{
	return false;
}

static void control_release(control_t **this, int Is_ptr)
{
	assert_param(this,;);
	control_t *_this = Is_ptr ? *this : (control_t *)this;
	if(_this->control_stop(_this) == false){
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
