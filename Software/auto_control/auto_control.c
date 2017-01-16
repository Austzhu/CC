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

static int32_t inline select_pwm_level(sql_t *sql, void *buf, int32_t keyword)
{
	if(!sql || !buf) return FAIL;
	return sql->sql_select(Asprintf("select level from db_index_pwm "\
		"where %d between light_min and light_max;",keyword),(char*)buf,sizeof(int),1,0);
}

/**
 * [获取入口段、过渡段、基本段、出口段的标准亮度值]
 * param  	this [description]
 * return      [成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_getvalues(control_t *this)
{
	typedef int (*calc_func_t)(calc_t*);
	if(!this || !this->calc) return FAIL;

	//int(*calc_func[])(calc_t*) = {
	calc_func_t calc_func[] = {
		this->calc->calc_light_enter,
		this->calc->calc_light_transit,
		this->calc->calc_light_base,
		this->calc->calc_light_exit,
		NULL,
	};
	for(calc_func_t *func=calc_func; *func != NULL; ++func){
		if( SUCCESS != (*func)(this->calc) ){
			debug(DEBUG_auto,"%s>%s_%d,error!",__FILE__,__func__,__LINE__);
			return FAIL;
		}
	}
	#if DEBUG_auto&0
		printf("\nenter_0=%d,enter_1=%d,transit_0=%d,"\
			"transit_1=%d,transit_2=%d,base_0=%d,exit_0=%d,exit_1=%d\n",\
			this->calc->args.light_enter[0],this->calc->args.light_enter[1],this->calc->args.light_transit[0],\
			this->calc->args.light_transit[1],this->calc->args.light_transit[2],this->calc->args.light_base[0],\
			this->calc->args.light_exit[0],this->calc->args.light_exit[1]);
	#endif
	return SUCCESS;
}

static void ctrl_set_light_current(control_t *this)
{
	assert_param(this,;);
	this->light_info[0].light_current = this->args->light_enter;
	this->light_info[1].light_current = this->args->light_enter;
	this->light_info[2].light_current = this->args->light_transit;
	this->light_info[3].light_current = this->args->light_transit;
	this->light_info[4].light_current = this->args->light_transit;
	this->light_info[5].light_current = this->args->light_base;
	this->light_info[6].light_current = this->args->light_base;
	this->light_info[7].light_current = this->args->light_base;
	this->light_info[8].light_current = this->args->light_exit;
	this->light_info[9].light_current = this->args->light_exit;
}

/**
 * [获取light_info_t的信息]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int32_t ctrl_update_info(control_t *this,sql_t *sql)
{
	assert_param(this,FAIL);
	light_info_t *info = NULL;
	int32_t keyword[2] = {0};
	debug(DEBUG_auto,"\nafter calc tunnel light values:");
	for(int i=0; i<CFG_COLUMN; ++i){
		info = this->light_info + i;
		/* 获取各段计算出的亮度 */
		if(info->light_current == NULL){
			ctrl_set_light_current(this);
			if(info->light_current == NULL)
				return FAIL;
		}
		switch(i){
			/* enter column */
			case 0:	//ENTER_0
				keyword[left] = info->light_current[0];
				keyword[right] = info->light_current[3];
				break;
			case 1:	//ENTER_1
				keyword[left] = info->light_current[1];
				keyword[right] = info->light_current[2];
				break;
			/* intensify column */
			case 2:	//intensify_0
				keyword[left] = info->light_current[0];
				keyword[right] = info->light_current[5];
				break;
			case 3:	//intensify_1
				keyword[left] = info->light_current[1];
				keyword[right] = info->light_current[4];
				break;
			case 4:	//intensify_2
				keyword[left] = info->light_current[2];
				keyword[right] = info->light_current[3];
				break;
			/* base column */
			case 5 ... 6://base_0
				keyword[left] = keyword[right] = info->light_current[0];
				break;
			case 7:	//base_1
				keyword[left] = keyword[right] = info->light_current[1];
				break;
			/* exit column */
			case 8:	//exit_0
				keyword[left] = keyword[right] = info->light_current[0];
				break;
			case 9:	//exit_1
				keyword[left] = keyword[right] = info->light_current[1];
				break;
			default:	goto out;
		}	// end of switch(i)
		if(ONE_WAY == this->args->bothway ){		//单向车道
			if(SUCCESS !=select_pwm_level(sql, \
				&info->pwm_level[left], keyword[left] ) ) goto out;
			info->pwm_level[right] = info->pwm_level[left];
			debug(DEBUG_auto,"%d ",keyword[left]);
		}else{		//双向车道
			if(SUCCESS !=select_pwm_level(sql, \
				&info->pwm_level[left], keyword[left] ) ) goto out;
			if(SUCCESS !=select_pwm_level(sql, \
				&info->pwm_level[right], keyword[right] ) ) goto out;
		}	//end of if(ONE_WAY...)
	}		//end of for(...)
	debug(DEBUG_auto,"\n");
	return SUCCESS;
out:
	return FAIL;
}

/**
 * [根据计算出的数据执行调光]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_exec(control_t *this, sql_t *sql)
{
	assert_param(this,FAIL);
	assert_param(sql,FAIL);
	/* 获取隧道各段计算后要求达到的亮度标准 */
	if(SUCCESS != ctrl_update_info(this,sql)){
		debug(DEBUG_auto,"update light information error!\n");
		return FAIL;
	}
	/* 获取隧道一共有多少个组 */
	if(this->group_cnt <= 0){
		sql->sql_select("select count(distinct gid) from "\
			"db_group_info;",(char*)&this->group_cnt,sizeof(int),1,0);
		if(this->group_cnt <= 0) {
			debug(DEBUG_auto,"table db_group_info is null!\n");
			return FAIL;
		}
	}
	struct g_info_t {
		int32_t gid;
		int32_t pwm;
	} *g_info = malloc(this->group_cnt * sizeof(struct g_info_t));
	if(NULL == g_info){
		debug(DEBUG_auto,"malloc for g_info error!\n");
		return FAIL;
	}
	/* 提取数据库中所有的组信息,以及当前组的pwm值 */
	sql->sql_select("select distinct gid,pwm_value from db_group_info "\
		"order by gid;",(char*)g_info,sizeof(struct g_info_t),this->group_cnt,0);
	/* 打印数据库中提取出来的group 和 pwm 信息 */
	#if DEBUG_auto && DEBUG_autocontrol
		printf("-------------------------------------------------------------------------\n");
		printf("---------group pwm info----------\nid\tgid\tpwm value\n---\t----\t------------\n");
		for(int i=0; i<this->group_cnt; ++i)
			printf("%d\t%d\t%d\n",i,g_info[i].gid,g_info[i].pwm);
		printf("----------tunnel set pwm value----------\n");
		printf("id\tleft pwm\tright pwm\n----\t-----------\t---------------\n");
		for(int i=0; i<CFG_COLUMN; ++i){
			printf("%d\t%d\t\t%d\n",i,this->light_info[i].pwm_level[left],this->light_info[i].pwm_level[right]);
		}
		printf("-------------------------------------------------------------------------\n");
	#endif
	/* 根据组号和pwm等级，与隧道对应的段对比,不在范围则实施控制操作 */
	uint32_t index = 0;
	struct light_info_t *light_info = NULL;
	for(struct g_info_t *pinfo = g_info; (pinfo-g_info) < this->group_cnt; ++pinfo){
		/* 指向对应段的数据 */
		index = (pinfo->gid%10)/5;
		light_info = this->light_info + pinfo->gid/10;
		/**
		 * 计算出的亮度与当前的pwm对应的亮度范围之外
		 * index=pinfo->gid/10 = 0~9  index = 0~1
		 * index 双向车道0表示左边对应的段，1表示右边对应的段
		 * 单向车道时 左边和右边效果一样
		 */
		//if(abs(light_info->pwm_level[(pinfo->gid%10)/5] - pinfo->pwm) > 0){
		if(light_info->pwm_level[index] != pinfo->pwm){
			debug(DEBUG_auto,"group=%d, light value=%d\n",pinfo->gid,light_info->pwm_level[index]);
			this->ctrl_output(this,pinfo->gid, light_info->pwm_level[index]);
		}
	}	//end of for(pinfo = g_info;
	FREE(g_info);
	return SUCCESS;
}

/**
 * 		[开环控制]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_openloop(control_t *this)
{
	assert_param(this,FAIL);

	sql_t *sql = this->calc->sql;
	if(!sql) {	 err_Print(DEBUG_auto,"sql is null!\n");	return FAIL; }
	/* get values */
	if(SUCCESS != this->ctrl_getvalues(this)){
		err_Print(DEBUG_auto,"control getvalues error!\n");
		return FAIL;
	}
	return ctrl_exec(this,sql);

}

/**
 * 		[开环输出]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_output(control_t *this, int32_t group, int32_t light)
{
	assert_param(this,FAIL);
	assert_param(this->single,FAIL);
	return this->single->sin_open(this->single,cmd_group,group<<24,light<<8);
}

/**
 * 		[自动控制线程]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static void *ctrl_pthread(void *args)
{
	control_t *this = (control_t*)args;
	if(!this || !this->args){
		debug(DEBUG_autocontrol,"In control thread this or args is error!\n");
		return NULL;
	}

	int sensor_cnt = 0;
	this->calc->sql->sql_select("select tun_sensor from db_tunnel_info;",(char*)&sensor_cnt,sizeof(int),1,0);
	while(this->pthread_start ){
		/* do something */
		if(this->args->bothway == 0 && sensor_cnt < 2 ){	//单向车道
			this->ctrl_openloop(this);
		}
		msleep(2000);
	}
	pthread_exit(NULL);
}


static int32_t ctrl_set_group_info(control_t *this,sql_t *sql)
{
	if(!this || !sql)  return FAIL;

	if(this->group_cnt <= 0){
		sql->sql_select("select count(distinct gid) from "\
			CFG_tb_gpinfo";",(char*)&this->group_cnt,sizeof(int),1,0);
		if(this->group_cnt <= 0) {
			debug(DEBUG_auto,"table db_group_info is null!\n");
			system("sqlite3 "CFG_DB_NAME"<<EOF\ninsert into db_group_info(gid) select distinct lt_gid from db_light;\nEOF");
			sql->sql_select("select count(distinct gid) from "\
				CFG_tb_gpinfo";",(char*)&this->group_cnt,sizeof(int),1,0);
		}
	}
	return this->group_cnt > 0 ? SUCCESS : FAIL;
}
/**
 * 		[启动自动控制]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_start(control_t *this)
{
	assert_param(this,FAIL);
	if(this->pthread_start && this->ctrl_thread != -1)		/* thread is start */
		return SUCCESS;
	/* 更新db_group_info的数据 */
	if(SUCCESS != ctrl_set_group_info(this,this->calc->sql) ) {
		debug(DEBUG_auto,"update group info error!\n");
		return FAIL;
	}
	debug(DEBUG_autocontrol,"auto control mode starting...\n");
	for(int i=0; i<10; ++i){
		this->ctrl_getvalues(this);
		msleep(300);
	}
	//sleep(10);
	this->pthread_start = 1;
	if(pthread_create(&this->ctrl_thread, NULL,ctrl_pthread,this)){
		debug(DEBUG_autocontrol,"In class auto control pthread create error!\n");
		return FAIL;
	}


	return SUCCESS;
}

/**
 * 		[结束自动控制]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_stop(control_t *this)
{
	assert_param(this,FAIL);
	if(this->pthread_start){
		this->pthread_start = 0;
		pthread_join(this->ctrl_thread,NULL);
		this->ctrl_thread = -1;
	}
	return SUCCESS;
}

/**
 * 		[闭环控制]
 * param  	this [description]
 * return  	[成功 SUCCESS,	失败 FAIL]
 */
static int ctrl_PID(control_t *this)
{
	return FAIL;
}

static void ctrl_release(control_t **this, int Is_ptr)
{
	assert_param(this,;);
	control_t *_this = Is_ptr ? *this : (control_t *)this;
	if(_this->ctrl_stop(_this) == FAIL){
		debug(DEBUG_autocontrol,"auto control stop thread error!\n");
	}
	DELETE(_this->calc, calc_release,true);
	if(Is_ptr)  FREE(*this);
}

struct control_t *control_init(struct control_t *this, void *single)
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
	this->args 				= &this->calc->args;
	this->ctrl_getvalues 	= ctrl_getvalues;
	this->ctrl_exec 			= ctrl_exec;
	this->ctrl_openloop 	= ctrl_openloop;
	this->ctrl_output 		= ctrl_output;
	this->ctrl_start   		= ctrl_start;
	this->ctrl_stop   		= ctrl_stop;
	this->ctrl_PID    			= ctrl_PID;
	this->ctrl_release 		= ctrl_release;
	return this;
out:
	debug(DEBUG_autocontrol,"control init error!\n");
	if(!temp) FREE(this);
	return NULL;
}





#if 0
/* 判断亮度是否超出阀值 */
#define Is_border(pval,_i) ( pval > this->gid_info.g_val[_i].limit[1] || pval< this->gid_info.g_val[_i].limit[0] )

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

#endif
/**
 * 限幅控制
 * 返回值：0未超出幅值, 1超出幅值
 * 0~8 bit 入口段
 * 9~15 bit 过渡段
 * 16~24 bit 基本段
 * 25~32 bit 出口段
 */
#if 0
static int ctrl_lightcmp(control_t *this)
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
#endif
