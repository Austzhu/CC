/******************************************************************
** 文件名:	Warn.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "Warn.h"
#include "Interface.h"

typedef struct {
	int fault;
	const char *message;
}fault_Message_t;

fault_Message_t Message[] = {
	{0x01,"Devices Single fault!"},
	{0x02,"Devices Coordinate fault!"},
	{0x04,"Communication fault!"},
	{0x08,"Devices light fault!"},
	{0x10,"Power voltage fault!"},
	{0x20,"Devices light module fault!"},
};

static int set_flags(Warn_t *this,int flags,int set_enu,int condition)
{
	assert_param(this,FAIL);
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;

	switch(set_enu){
		case sw_single:
			return sqlite->sql_update("db_info_light",Asprintf("set Warn_flags="\
					"Warn_flags|%d where Base_Addr=0x%x;",flags,condition));
		case sw_group:
			return sqlite->sql_update("db_info_light ",Asprintf("set Warn_flags="\
							"Warn_flags|%d where Base_Addr in (select Base_Addr "\
								"from db_light b where b.lt_gid=0x%x);",flags,condition));
		case sw_brocast:
			return sqlite->sql_update("db_info_light ",Asprintf("set Warn_flags="\
							"Warn_flags|%d;",flags));
		case sw_inCoordi:
			return sqlite->sql_update("db_info_light ",Asprintf("set Warn_flags="\
							"Warn_flags|%d where Base_Addr in (select Base_Addr "\
										"from db_light where Coor_id=0x%x);",flags,condition));
		case sw_coordi:
			return sqlite->sql_update("db_coordinator ",Asprintf("set Warn_flags="\
							"Warn_flags|%d where Base_Addr=0x%x;",flags,condition));
		case so_single:
			return sqlite->sql_update("db_info_light",Asprintf("set operate_flags=%d "\
					"where Base_Addr=0x%x;",flags,condition));
		case so_group:
			sqlite->sql_update("db_info_light ",Asprintf("set operate_flags=%d "\
			"where Base_Addr in (select Base_Addr from db_light b where b.lt_gid=0x%x);",flags,condition));
		case so_brocast:
			return sqlite->sql_update("db_info_light ",Asprintf("set operate_flags=%d",flags));
		default:break;
	}
	return FAIL;
}

static int clean_flags(Warn_t *this,int flags,int set_enu,int condition)
{
	assert_param(this,FAIL);
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;

	switch(set_enu){
		case sw_single:
			return sqlite->sql_update("db_info_light",Asprintf("set Warn_flags="\
					"Warn_flags&~%d where Base_Addr=0x%x;",flags,condition));
		case sw_group:
			return sqlite->sql_update("db_info_light ",Asprintf("set Warn_flags="\
							"Warn_flags&~%d where Base_Addr in (select Base_Addr "\
								"from db_light b where b.lt_gid=0x%x);",flags,condition));
		case sw_brocast:
			return sqlite->sql_update("db_info_light ",Asprintf("set Warn_flags="\
							"Warn_flags&~%d;",flags));
		case sw_inCoordi:
			return sqlite->sql_update("db_info_light ",Asprintf("set Warn_flags="\
							"Warn_flags&~%d where Base_Addr in (select Base_Addr "\
										"from db_light where Coor_id=0x%x);",flags,condition));
		case sw_coordi:
			return sqlite->sql_update("db_coordinator ",Asprintf("set Warn_flags="\
							"Warn_flags&~%d where Base_Addr=0x%x;",flags,condition));
		case so_single:
			return sqlite->sql_update("db_info_light",Asprintf("set operate_flags=0 "\
					"where Base_Addr=0x%x;",condition));
		case so_group:
			sqlite->sql_update("db_info_light ",Asprintf("set operate_flags=0 "\
			"where Base_Addr in (select Base_Addr from db_light b where b.lt_gid=0x%x);",condition));
		case so_brocast:
			return sqlite->sql_update("db_info_light ","set operate_flags=0;");
		default:break;
	}
	return FAIL;
}

static int JudgeforSingle(Warn_t *this,struct sin_warn_t *info,int infosize)
{
	assert_param(this,FAIL);
	assert_param(info,FAIL);

	int power = 0;
	for(int i=0;i<infosize;++i){
		info[i].fault_flags = fault_none;
		/* 灯具故障,开灯操作，有电压，没电流 */
		if(info[i].opt_flags !=0 && info[i].elec == 0 && info[i].vol != 0 )
			info[i].fault_flags |= fault_light;
		/* 供电电压不足,开灯状态,低于额定电压的70% */
		if(info[i].elec > 0 && info[i].vol < info[i].Rvol*0.7 )
			info[i].fault_flags |= fault_valtage;
		/* 灯具模组坏了，满占空比的时候,实际功率<额定功率*功率因数*70% */
		power = info[i].elec * info[i].vol /1000000;
		if(info[i].elec > 0 && power < info[i].Rpow*info[i].Rpf*0.7)
			info[i].fault_flags |= fault_module;
		/* 单灯设备故障 */
		if((info[i].Warn_flags&Warn_Single_All) == Warn_Single_All)
			info[i].fault_flags |= fault_single;
		else if(info[i].Warn_flags != Warn_Single_ok)
			info[i].fault_flags |= fault_communication;
		/* 将错误信息写入报警日志表中 */
		if(fault_none != info[i].fault_flags){
			this->warn_Insert(this,info[i].addr,info[i].fault_flags);
			this->warn_cleanflags(this,Warn_Single_All,sw_single,info[i].addr);
		}
	}
	return SUCCESS;
}

static int Judge_fault(Warn_t *this,struct Warn_info_t *info,int infosize)
{
	assert_param(this,FAIL);
	assert_param(info,FAIL);

	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;
	int count = 0, errcount = 0;
	for(int i=0;i<infosize;++i){
		sqlite->sql_select(Asprintf("select count(*) from db_light "\
			"where Coor_id=0x%x;",info[i].Coor_Addr),(s8*)&count,sizeof(int),1,0);
		sqlite->sql_select(Asprintf("select count(*) from db_info_light a,db_light b "\
			"where a.Base_Addr=b.Base_Addr  AND  b.Coor_id=0x%x AND a.Warn_flags&%u=%u;",\
					info[i].Coor_Addr,Warn_Single_All,Warn_Single_All),(s8*)&errcount,sizeof(int),1,0);
		if(count == errcount){	/* 一个协调器下面的所有单灯都故障,则认为协调器可能有问题 */
			info[i].fault_flags = fault_coordi;
			this->warn_Insert(this,info[i].Coor_Addr,fault_coordi);
			this->warn_cleanflags(this,Warn_Single_All,sw_inCoordi,info[i].Coor_Addr);
		}else{
			info[i].fault_flags = fault_none;
			JudgeforSingle(this,info[i].single_info,info[i].Single_count);
		}
	}
	return SUCCESS;
}

static int warn_verdict(struct Warn_t *this)
{
	assert_param(this,FAIL);

	int Coordi_count = 0; int res = -1;
	struct Warn_info_t *Warn_info = NULL;
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;
	sqlite->sql_select("select count(*) from db_coordinator;",(s8*)&Coordi_count,sizeof(int),1,0);
	if(Coordi_count <=0) goto out;

	Warn_info = calloc(Coordi_count,sizeof(struct Warn_info_t));
	if(!Warn_info) goto out;

	res = sqlite->sql_select("select Base_Addr,Warn_flags from db_coordinator;",\
						(s8*)Warn_info,sizeof(struct Warn_info_t),Coordi_count,0);
	if(res != SUCCESS) goto out;
	for(int i=0;i<Coordi_count;++i){
		sqlite->sql_select(Asprintf("select count(*) from db_info_light a,"\
			"db_light b where a.Base_Addr=b.Base_Addr and b.Coor_id=0x%x ;",\
			Warn_info[i].Coor_Addr),(s8*)&Warn_info[i].Single_count,sizeof(int),1,0);
		if(Warn_info[i].Single_count <= 0) continue;
		Warn_info[i].single_info = calloc(Warn_info[i].Single_count,sizeof(sin_warn_t));
		if(!Warn_info[i].single_info) goto out;
		res = sqlite->sql_select(Asprintf("select a.Base_Addr,a.Warn_flags,a.operate_flags,a.Rate_p,a.Rate_v,"\
			"a.Rate_PF,a.light_val,a.light_E,a.light_V from db_info_light a,db_light b where a.Base_Addr=b.Base_Addr "\
			"and b.Coor_id=0x%x ;",Warn_info[i].Coor_Addr),(s8*)Warn_info[i].single_info,sizeof(sin_warn_t),Warn_info[i].Single_count,0);
		if(SUCCESS != res) goto out;
	}
	/* 判断错误 */
	res = Judge_fault(this,Warn_info,Coordi_count);
	if(res != SUCCESS)  goto out;

	for(int i=0;i<Coordi_count;++i){
		free(Warn_info[i].single_info);
	}	free(Warn_info);
	return SUCCESS;
out:
	debug(1,"warn_verdict error!\n");
	for(int i=0;i<Coordi_count;++i){
		free(Warn_info[i].single_info);
	}	free(Warn_info);
	return FAIL;
}

static const char *get_mark(int type)
{
	for(int i=0,size=sizeof(Message)/sizeof(Message[0]); i<size;++i){
		if(type == Message[i].fault)
			return Message[i].message;
	}
	return NULL;
}

static int warn_Insert(struct Warn_t *this,int addr,int type)
{
	assert_param(this,FAIL);

	const char *pmessage = NULL;
	int Index = 0x01;
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;
	do{
		if(type&Index){
			pmessage = get_mark(Index);
			sqlite->sql_insert(Asprintf("insert into db_warn(Add_time,Type,"\
				"Grade,State,Addr,Remark) values(%ld,%d,%d,1,%d,\'%s\');",time(NULL),\
				Index,Index==fault_coordi?0:1,addr,NULL==pmessage?"unknow warn type!":pmessage));
		}
	}while(Index<<=1);
	/* 删除两个月之前的表 */
	sqlite->sql_delete(Asprintf("delete from db_warn where Add_time < (%ld-60*24*60*60)",time(NULL)) );
	return SUCCESS;
}

static void warn_relese(Warn_t **this)
{
	memset(*this,0,sizeof(Warn_t));
	free(*this);
	*this = NULL;
}

Warn_t *warn_init(void *topuser)
{
	Warn_t *warn = (Warn_t*)calloc(1,sizeof(Warn_t));
	if(!warn) return NULL;
	warn->topuser = topuser;
	warn->warn_relese = warn_relese;
	warn->warn_setflags = set_flags;
	warn->warn_cleanflags = clean_flags;
	warn->warn_verdict = warn_verdict;
	warn->warn_Insert = warn_Insert;

	if(!warn->topuser || !warn->warn_relese ||\
		!warn->warn_setflags || !warn->warn_cleanflags || \
		!warn->warn_verdict || !warn->warn_Insert ){
		free(warn);
		return NULL;
	}
	return warn;
}
