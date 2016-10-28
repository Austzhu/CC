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


static int warn_set_flags(Warn_t *this,int addr,int flags,const char *table)
{
	assert_param(this,NULL,FAIL);
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;
	if(NULL == table)
		return sqlite->sql_update("db_info_light",Asprintf("set operate_flags=%d "\
				"where Base_Addr=0x%02X;",flags,addr));
	else
		return sqlite->sql_update(table,Asprintf("set Warn_flags="\
				"Warn_flags|%d where Base_Addr=%d;",flags,addr));
}

static int warn_clean_flags(Warn_t *this,int addr,int flags,const char *table)
{
	assert_param(this,NULL,FAIL);
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;
	if(NULL == table)
		return sqlite->sql_update("db_info_light",Asprintf("set operate_flags=%d "\
				"where Base_Addr=0x%02X;",flags,addr));
	else
		return sqlite->sql_update(table,Asprintf("set Warn_flags="\
				"Warn_flags&%d where Base_Addr=%d;",~flags,addr));
}

static fault_t  Judge_fault(int judge, void *table)
{
	switch(judge){
		case judge_single:
			break;
		case judge_coordi:
			break;
		default:break;
	}
	return fault_none;
}

static int warn_verdict(struct Warn_t *this)
{
	assert_param(this,NULL,FAIL);

	int warn_single_cnt = 0,  warn_coordi_cnt = 0;
	Tableinfolight_t *warn_single = NULL;
	TableCoordi_t *warn_coordi = NULL;
	sql_t *sqlite = ((appitf_t*)this->topuser)->sqlite;
	/* get the count of single and Coordinate fault */
	sqlite->sql_select("select count(Warn_flags) from db_info_light "\
						"where Warn_flags!=0;",(s8*)&warn_single_cnt,sizeof(int),1,0);
	sqlite->sql_select("select count(Warn_flags) from db_coordinator "\
						"where Warn_flags!=0;",(s8*)&warn_coordi_cnt,sizeof(int),1,0);
	if(warn_single_cnt <= 0 || warn_coordi_cnt <= 0)  goto out;
	warn_single = calloc(warn_single_cnt,sizeof(Tableinfolight_t));
	warn_coordi = calloc(warn_coordi_cnt,sizeof(TableCoordi_t));
	if(!warn_single || !warn_coordi )  goto out;
	int res = SUCCESS;
	res |= sqlite->sql_select("select * from db_info_light where Warn_flags!=0;",\
							(s8*)warn_single,sizeof(Tableinfolight_t),warn_single_cnt,0);
	res |= sqlite->sql_select("select * from db_coordinator where Warn_flags!=0;",\
							(s8*)warn_coordi,sizeof(TableCoordi_t),warn_coordi_cnt,0);
	if(SUCCESS != SUCCESS )  goto out;

	fault_t  fault;
	Tableinfolight_t *psingle = warn_single;
	TableCoordi_t *pcoordi = warn_coordi;
	/* set warn for table_single */
	for(int i=0; i< warn_single_cnt ; ++i){
		fault = Judge_fault(judge_single,psingle);
		if(fault_none != fault)
			this->warn_Insert(this,psingle->Base_Addr,fault);
		++psingle;
	}
	/* set warn for table_coordinator */
	for(int i=0;i<warn_coordi_cnt;++i){
		fault = Judge_fault(judge_coordi,pcoordi);
		if(fault_none != fault)
			this->warn_Insert(this,pcoordi->Base_Addr,fault);
		++pcoordi;
	}
	free(warn_coordi);
	free(warn_single);
	return SUCCESS;
 out:
 	free(warn_coordi);
 	free(warn_single);
 	return FAIL;
}

static void warn_relese(Warn_t *this)
{
	free(this);
}

Warn_t *warn_init(void *topuser)
{
	assert_param(topuser,NULL,NULL);

	Warn_t *warn = (Warn_t*)calloc(1,sizeof(Warn_t));
	if(!warn) return NULL;
	warn->topuser = topuser;
	warn->warn_relese = warn_relese;
	warn->warn_setflags = warn_set_flags;
	warn->warn_cleanflags = warn_clean_flags;
	warn->warn_verdict = warn_verdict;

	return warn;
}
