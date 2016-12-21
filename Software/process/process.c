/******************************************************************
** 文件名:	process.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "process.h"
#include "database.h"

#define Mktime(_tm,p_data,_time) do{\
	(_tm)->tm_year		= *p_data++ + 100;\
	(_tm)->tm_mon	= *p_data++  -  1;\
	(_tm)->tm_mday	= *p_data++;\
	(_tm)->tm_hour	= *p_data++;\
	(_tm)->tm_min  	= *p_data++;\
	(_tm)->tm_sec   	= *p_data++;\
	_time = mktime(_tm);\
}while(0)


#define gettime(pbf,_tm) do{\
	*pbf++ = (_tm)->tm_year %100;\
	*pbf++ = (_tm)->tm_mon + 1;\
	*pbf++ = (_tm)->tm_mday;\
	*pbf++ = (_tm)->tm_wday;\
	*pbf++ = (_tm)->tm_hour;\
	*pbf++ = (_tm)->tm_min;\
	*pbf++ = (_tm)->tm_sec;\
}while(0)

int Reset2DoFunctions(void)
{
	return SUCCESS;
}

int time_tick(u8 *package)
{
	assert_param(package,FAIL);
	struct timeval tv = {0};
	struct tm tm = {
		.tm_year 	= *(package+0) + 100,
		.tm_mon	= *(package+1)  - 1,
		.tm_mday	= *(package+2),
		.tm_hour	= *(package+4),
		.tm_min  	= *(package+5),
		.tm_sec   	= *(package+6),
	};
	tv.tv_sec 		= mktime(&tm);
	debug(DEBUG_chktime,"set linux time: %s",ctime(&tv.tv_sec));
	return  settimeofday(&tv,NULL) == 0 ? SUCCESS : FAIL;
}

int Query_time(u8 *buf,int bufsize)
{
	assert_param(buf,FAIL);
	u8 *const pbuf = buf;
	time_t SystemTime = 0;
	struct tm TimeRTC = {0};
	struct tm TimeSystem = {0};
	bzero(buf,bufsize);
	*buf++ = 0x51; *buf++ = 0x13; *buf++ = 0x80;
	*buf++ = 0x11; *buf++ = 0xA2; *buf++ = 0x04;

	/* system time */
	time(&SystemTime);
	TimeSystem = *localtime(&SystemTime);
	gettime(buf,&TimeSystem);
	/* get rtc time */
	int  fd = open("/dev/rtc0",O_RDONLY,0);
	if(fd < 0) goto out;
	if(ioctl(fd,RTC_RD_TIME,&TimeRTC))
		goto out;
	close(fd);
	gettime(buf,&TimeRTC);

	return *(pbuf+20) = SUCCESS;
out:
	if(fd > 0) close(fd);
	return *(pbuf+20) = FAIL;
}

int CC_Inquire_Version(u8 *buf,int size)
{
	assert_param(buf,FAIL);
	bzero(buf,size);
	u8 * const pf = buf;
	*buf++ = 0x51; *buf++ = 0x25; *buf++ = 0x80;
	*buf++ = 0x23; *buf++ = 0xA2; *buf++ = 0x05;
	sprintf((char*)buf,"%s",VERSION_NUMBER);
	*(pf+0x23+3) = SUCCESS;
	return SUCCESS;
}

int SingleConfig(u8 *package,appitf_t *app)
{
	assert_param(package,FAIL);
	assert_param(app,FAIL);

	TableSingle_t   tab_single;
	tab_single.Wl_Addr 	= package[0];
	tab_single.lt_gid		= package[1];
	tab_single.Coor_id 		= package[2];
	tab_single.Base_Addr 	= (package[3] <<8) | package[4];
	tab_single.Map_Addr 	= (package[5] <<8) | package[6];
	debug(DEBUG_CC_Config,"table single:Wl_Addr=0x%02x,lt_gid=0x%02x,"\
		"Coor_id=0x%02x,Base_Addr=0x%04x,Map_Addr=0x%04x\n",tab_single.Wl_Addr,\
		tab_single.lt_gid,tab_single.Coor_id,tab_single.Base_Addr,tab_single.Map_Addr);

	if(SUCCESS != app->single->sin_config(app->single,cfg_sinGroup,&tab_single)) return FAIL;
	if(SUCCESS != app->single->sin_config(app->single,cfg_sinMap,&tab_single)) return FAIL;
	/* check sqlcolumn is exist */
	int res = -1;
	if(sql_isexist(app->sqlite,"db_light",tab_single.Base_Addr)){
		res = app->sqlite->sql_update("db_light",Asprintf("set Wl_Addr=%d,lt_gid=%d,"\
			"Coor_id=%d,Map_Addr=%d where Base_Addr=%d",tab_single.Wl_Addr,\
			tab_single.lt_gid,tab_single.Coor_id,tab_single.Map_Addr,tab_single.Base_Addr));
	}else{
		res = app->sqlite->sql_insert(Asprintf("insert into db_light(Wl_Addr,Base_Addr,lt_gid,"\
			"Coor_id,Map_Addr)  values(%d,0x%04X,%d,%d,%d);",tab_single.Wl_Addr,\
			tab_single.Base_Addr,tab_single.lt_gid,tab_single.Coor_id,tab_single.Map_Addr));
		if(SUCCESS == res)
			res = app->sqlite->sql_insert(Asprintf("insert into db_info_light(Base_Addr) "\
				"values(0x%04X);",tab_single.Base_Addr));
		if(SUCCESS != res)
			app->sqlite->sql_delete(Asprintf("delete from db_light where Base_Addr=%d;",tab_single.Base_Addr));
	}
	return res;
}

int CoordiConfig(u8 *package,  appitf_t *app)
{
	assert_param(package,FAIL);
	assert_param(app,FAIL);

	TableCoordi_t tab_coor;
	tab_coor.Wl_Addr 		= *package++;
	tab_coor.Coor_gid 		= *package++;
	memset(tab_coor.CC_id,0,sizeof(tab_coor.CC_id));
	Hex2Str(tab_coor.CC_id,package,6);package += 6 ;
	tab_coor.Base_Addr 	= *package++;
	tab_coor.Map_Addr		= *package;

	debug(DEBUG_CC_Config,"CCUID: %s,Wl_Addr=0x%02x,Coor_gid=0x%02x,"\
		"Base_Addr=0x%02x,Map_Addr=0x%02x\n",tab_coor.CC_id,\
		tab_coor.Wl_Addr,tab_coor.Coor_gid,tab_coor.Base_Addr,tab_coor.Map_Addr);

	if(SUCCESS != app->single->sin_config(app->single,cfg_coorMap,&tab_coor)) return FAIL;

	if(sql_isexist(app->sqlite,"db_coordinator",tab_coor.Base_Addr)){
		return app->sqlite->sql_update("db_coordinator",Asprintf("set Wl_Addr=%d,Coor_gid=%d,"\
			"CC_id='%s',Map_Addr=%d where Base_Addr=%d",tab_coor.Wl_Addr,\
			tab_coor.Coor_gid,tab_coor.CC_id,tab_coor.Map_Addr,tab_coor.Base_Addr));
	}else{
		return app->sqlite->sql_insert( Asprintf("insert into db_coordinator(Wl_Addr,Base_Addr,"\
			"Coor_gid,CC_id,Map_Addr) values(%d,%d,%d,'%s',%d);",tab_coor.Wl_Addr,\
			tab_coor.Base_Addr,tab_coor.Coor_gid,tab_coor.CC_id,tab_coor.Map_Addr));
	}
}

int delete_sql(u8 *Pdata,appitf_t *app)
{
	assert_param(Pdata,FAIL);
	assert_param(app,FAIL);

	struct tm tim = {0};
	time_t start = 0,end = 0;

	switch(*Pdata++){
		case 0x00:		//协调器记录表
			if(*Pdata++ == 0x00){
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_coordinator where Base_Addr=0x%x;",*Pdata));
			}else{
				debug(DEBUG_DelSql,"db_coordinator have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		case 0x01:		//单灯记录表
			if(*Pdata++ == 0x00){
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_light where Base_Addr=0x%x;",*Pdata<<8 | *(Pdata+1)));
			}else{
				debug(DEBUG_DelSql,"db_light have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		case 0x02:		//任务表
			if(*Pdata == 0x01){//名称
				return app->sqlite->sql_delete(Asprintf("delete from db_task where Name=%s;",Pdata+1));
			}else if(*Pdata++ == 0x02){//时间范围
				Mktime(&tim,Pdata,start);
				Mktime(&tim,Pdata,end);
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_task where Start_Date < %ld AND Start_Date > %ld",end,start));
			}else{
				debug(DEBUG_DelSql,"db_task have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		case 0x03:		//报警日志记录表
			if(*Pdata++ == 0x02){//名称
				Mktime(&tim,Pdata,start);
				Mktime(&tim,Pdata,end);
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_warn where Start_Date < %ld AND Start_Date > %ld",end,start));
			}else{
				debug(DEBUG_DelSql,"db_warn have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		default:break;
	}
	return FAIL;
}

int tunnel_config(u8 *package,appitf_t *app)
{
	assert_param(package,FAIL);
	assert_param(app,FAIL);

	struct { u8 one_way;  u8 speed;  u16 lenth;  u8 sensor; } tun = {
		.one_way = *package,
		.speed 	= *(package+1),
		.lenth 	= *(package+2)<<8 | *(package+3),
		.sensor	= *(package+4),
	};
	debug(1,"config > one_way:%d,speed:%d,lenth:%d,sensor:%d.\n",\
		tun.one_way,tun.speed,tun.lenth,tun.sensor);
	return app->sqlite->sql_insert(Asprintf("insert into db_tunnel_info(tun_bothway,tun_speed,"\
		"tun_length,tun_sensor) values(%d,%d,%d,%d);",tun.one_way,tun.speed,tun.lenth,tun.sensor));
}
