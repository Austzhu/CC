#include "timertask.h"

static tmtask_t *tmtask = NULL;

static void do_tmtask(int signum)
{
	if(!tmtask) return;
	tmtask->tmtask_exec(tmtask);
}

static int tmtask_start(struct tmtask_t *this)
{
	if(!this) return FAIL;
	if(this->Is_start) return SUCCESS;

	if(SUCCESS == this->tmtask_update(this)){
		if(this->second >= 0)
			alarm(this->second);
		this->Is_start = !0;
		return SUCCESS;
	}
	return FAIL;
}

static int tmtask_stop(struct tmtask_t *this)
{
	if(!this) return FAIL;
	alarm(0);
	this->Is_start = 0;
	return SUCCESS;
}

static void GetCmd_AppendQueue(struct tmtask_t *this,int32_t task_id)
{
	assert_param(this,;);
	int32_t cmd_cnt = 0;
	this->sql->sql_select(Asprintf("select count(id) from "CFG_tb_tasklist \
		" where task_id=%d;",task_id),(char*)&cmd_cnt,sizeof(int32_t),1,0);
	if(cmd_cnt <= 0) return ;
	uint8_t cmd_hex[256] = {0};
	char *cmd_text = malloc(cmd_cnt*512);
	if(!cmd_text) return;
	bzero(cmd_text,cmd_cnt*512);
	this->sql->sql_select(Asprintf("select cmd from"CFG_tb_tasklist \
		" where task_id=%d;",task_id),cmd_text,512,cmd_cnt,1,512);

	for(char *ptext = cmd_text; cmd_cnt--; ptext += 512){
		bzero(cmd_hex,sizeof(cmd_hex));
		debug(DEBUG_tmtask,"time task cmd:%s\n",ptext);
		Str2Hex(cmd_hex,cmd_text);
		int32_t Que_type = this->Queue->get_Quetype(this->Queue,*cmd_hex);
		if(-1 == Que_type)	continue;
		if(SUCCESS != this->Queue->Task_Append(this->Queue,\
			Que_type&0xff,(Que_type>>8) &0xff,cmd_hex,*(cmd_hex+1)+2))
				debug(DEBUG_tmtask,"time task append to queue error!\n");
	}
	FREE(cmd_text);
}


static int do_tmtask_begin(struct tmtask_t *this)
{
	assert_param(this,FAIL);
	tmparam_t tmstart;
	bzero(&tmstart,sizeof(tmparam_t));

	for(int i=0; i<RPTCNT; ++i){
		if(this->task_startid[i] <= 0)
			break;
		this->sql->sql_select(Asprintf("select id,start,end,flow_up,flow_down,"\
			"light_up,light_down,repeat_cnt,repeat_tm,status from"CFG_tb_task \
			" where id=%d;",this->task_startid[i]),(char*)&tmstart,sizeof(tmstart),1,0);
		/* update db_task info */
		--tmstart.task_cnt;
		tmstart.task_start += tmstart.task_tm;
		tmstart.task_status = 2;  /* task is executing */
		this->sql->sql_update(CFG_tb_task,\
			Asprintf("set repeat_cnt=%d,start=%ld,status=%d where id=%d",\
				tmstart.task_cnt,tmstart.task_start,tmstart.task_status,tmstart.task_id));
		/* get the flow and light */
		/* compare flow and light */
		if(tmstart.task_flowup != 0 && tmstart.task_flowdown !=0 ){
			this->sensor->sensor_get_stream(this->sensor);
			if(this->sensor->traffic < tmstart.task_flowdown ||\
				this->sensor->traffic > tmstart.task_flowup )
					continue;
		}
		if(tmstart.task_lightup != 0 && tmstart.task_lightdown !=0 ){
			this->sensor->sensor_get_light(this->sensor);
			if(this->sensor->light < tmstart.task_lightdown ||\
				this->sensor->light > tmstart.task_lightup )
					continue;
		}
		/* get cmd from db_tasklist */
		/* format text cmd to hex cmd */
		/* append cmd to taskqueue */
		GetCmd_AppendQueue(this,this->task_startid[i]);
	}
	return SUCCESS;
}
static int do_tmtask_end(struct tmtask_t *this)
{
	assert_param(this,FAIL);
	tmparam_t tmend;
	bzero(&tmend,sizeof(tmparam_t));

	for(int i=0; i<RPTCNT; ++i){
		if(this->task_endid[i] <= 0)
			break;
		this->sql->sql_select(Asprintf("select id,start,end,flow_up,flow_down,"\
			"light_up,light_down,repeat_cnt,repeat_tm,status from "CFG_tb_task \
			" where id=%d;",this->task_endid[i]),(char*)&tmend,sizeof(tmend),1,0);
		/* update db_task info */
		tmend.task_end += tmend.task_tm;
		if(tmend.task_cnt <= 0)
			tmend.task_status = 1;
		else
			tmend.task_status = 0;
		this->sql->sql_update(CFG_tb_task,Asprintf("set ,end=%ld,status=%d "\
				"where id=%d",tmend.task_end,tmend.task_status,tmend.task_id));
	}
	return SUCCESS;
}

static int tmtask_exec(struct tmtask_t *this)
{
	assert_param(this,FAIL);

	if(this->task_starttm < this->task_endtm){		//start
		if(this->task_starttm >0)
			do_tmtask_begin(this);
	}else if(this->task_starttm > this->task_endtm){	//end
		if(this->task_endtm >0)
			do_tmtask_end(this);
	}else{	// both start and end
		if(this->task_starttm >0){
			do_tmtask_begin(this);
			do_tmtask_end(this);
		}
	}

	/* update time task */
	if(SUCCESS == this->tmtask_update(this)){
		if(this->second >= 0)
			alarm(this->second);
	}else{
		return FAIL;
	}
	return SUCCESS;
}

static int tmtask_update(struct tmtask_t *this)
{
	assert_param(this,FAIL);
	time_t tmtask_now = time(NULL);
	/* 需要更新开始的任务时间和id */
	this->task_starttm = 0;
	bzero(this->task_startid,sizeof(this->task_startid));
	this->sql->sql_select(Asprintf("select min(start),from "CFG_tb_task \
			" where repeat_cnt != 0 and status = 0 and %ld < start;",\
				tmtask_now), (char*)&this->task_starttm,sizeof(int32_t),1,0);
	if(this->task_starttm > 0)
		this->sql->sql_select(Asprintf("select id,from "CFG_tb_task \
			" where repeat_cnt != 0 and status = 0 and %d = start;",\
				this->task_starttm), (char*)this->task_startid,sizeof(int32_t),RPTCNT,0);

	/* 需要更新结束的任务时间和id */
	this->task_endtm = 0;
	bzero(this->task_endid,sizeof(this->task_endid));
	this->sql->sql_select(Asprintf("select min(end),from "CFG_tb_task \
			" where status = 2;"), (char*)&this->task_endtm,sizeof(int32_t),1,0);
	if(this->task_endtm > 0)
		this->sql->sql_select(Asprintf("select id,from "CFG_tb_task" where status = 2 "\
			"and  %d = end;",this->task_endtm), (char*)this->task_endid,sizeof(int32_t),RPTCNT,0);

	this->second = (this->task_starttm > this->task_endtm) ? \
		this->task_endtm-tmtask_now : this->task_starttm - tmtask_now ;
	return SUCCESS;
}


static void tmtask_release(struct tmtask_t **this)
{
	if(!tmtask) return ;

	assert_param(this,;);
	assert_param(*this,;);
	FREE((*this)->sql);
	FREE(*this);
	tmtask = NULL;
}

struct tmtask_t *tmtask_init(tmtask_t *this, sensor_t *ser, Queue_t *que)
{
	if(tmtask) return tmtask;

	tmtask_t *pthis = this;
	if(!pthis){
		this = malloc(sizeof(tmtask_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(tmtask_t));

	this->tmtask_start = tmtask_start;
	this->tmtask_stop = tmtask_stop;
	this->tmtask_exec = tmtask_exec;
	this->tmtask_update = tmtask_update;
	this->tmtask_release = tmtask_release;
	if(SIG_ERR == signal(SIGALRM,do_tmtask)){
		debug(DEBUG_tmtask,"signal error!\n");
		goto out;
	}
	this->sensor = ser;
	if(!this->sensor) goto out;
	this->Queue = que;
	if(!this->Queue) goto out;

	this->sql = sql_Init(NULL);
	if(!this->sql) goto out;

	tmtask = this;
	return this;

out:
	if(!pthis)
		FREE(this);
	return NULL;
}
