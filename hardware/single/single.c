/******************************************************************
** 文件名:	single.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "single.h"
#include "database.h"
#include "Interface.h"


typedef struct {u16 Single_Addr;  u8 stat;  u8 light;} status_recv_t;
typedef struct {u16 Single_Addr;  u16 light_V; u16 light_E; } electric_recv_t;
typedef struct {u32 Coor_Addr;  u32 single_Count; } CoordiAddr_t;
typedef struct {u32 Coor_Addr;  u32 SingleAddr;  u32 Warn_flags; u32 status;   u32 light;   u32 light_V;   u32 light_E; } Sqlbuf_t;

#define SendTimeout 1000000		//1s
#define RecvTimeout  20

#define Task_append(buf,ptr) do{\
	if(ptr->get_Quetype && ptr->Task_Append){\
		int type = ptr->get_Quetype(ptr,buf[0]);\
		ptr->Task_Append(ptr,type&0xff,(type>>8) &0xff,buf,buf[1]+2);\
	}\
}while(0)

#define MakeSinglePack(ptr,ctrl,cmd,Addr,_data) do{\
	(ptr)->Header = 0xFF;(ptr)->Ctrl = 0xff&(ctrl); (ptr)->Group_Addr = 0xff&((Addr)>>24);\
	(ptr)->Coordi_Addr = 0xff&((Addr)>>16);(ptr)->Single_Addr[0] = 0xff&((Addr)>>8);\
	(ptr)->Single_Addr[1] = 0xff&(Addr);(ptr)->Cmd[0] = 0xff&((cmd)>>8);\
	(ptr)->Cmd[1] = 0xff&(cmd);(ptr)->Data[0] = 0xff&((_data)>>8);(ptr)->Data[1] = 0xff&(_data);\
}while(0)

#define Makelectric_tag(pf,_addr,_vol,_elec)  do{\
	*pf++ = 0xff&((_vol)>>8);   *pf++ = 0xff&(_vol);\
	*pf++ = 0xff&((_elec)>>8); *pf++ = 0xff&(_elec);\
	*pf++ = 0xff&((_addr)>>8);*pf++ = 0xff&(_addr);\
}while(0)

#define Makestatus_tag(pf,_addr,_light,_stat)  do{\
	*pf++ = 0xff&(_light);  *pf++ = 0xff&(_stat);\
	*pf++ = 0xff&((_addr)>>8);  *pf++ = 0xff&(_addr);\
}while(0)

#define MakeSingleQueryRes(pf,len,cmd,_addr,res,reslut) do{\
	*pf++ = 0x52; *pf++ = len+2; *pf++ = 0x80; *pf++ = len;\
	*pf++ = 0xff&((cmd)>>8); *pf++ = 0xff&(cmd); *pf++ = 0xff&((_addr)>>16);\
	*pf++ = 0xff&((_addr)>>8); *pf++ = 0xff&(_addr);\
	if(len > 8) {\
		*pf++ = 0xff&((res)>>24);*pf++ = 0xff&((res)>>16);\
		*pf++ = 0xff&((res)>>8);*pf++ = 0xff&(res);\
	}else{\
		*pf++ = 0xff&((res)>>8);*pf++ = 0xff&(res);\
	} *pf++ = reslut;\
}while(0)

static int sin_Config(Single_t *this,sin_cfg_t cmd,void *package)
{
	assert_param(this,NULL,FAIL);
	assert_param(package,NULL,FAIL);

	light_t single;
	TableSingle_t   *sp = package;
	TableCoordi_t *cp = package;
	appitf_t *topuser = this->topuser;
	u32 Addr = 0;
	topuser->Serial->serial_flush(topuser->Serial,COM_485);
	switch(cmd){
		case cfg_sinMap:
			Addr = (sp->Coor_id<<16) | sp->Base_Addr;
			MakeSinglePack(&single,0x01,0x0200,Addr,sp->Map_Addr);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("Single Config MapAddr Send data:",&single,sizeof(single));
			topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			break;
		case cfg_sinGroup:
			Addr = (sp->Coor_id << 16) | sp->Base_Addr;
			MakeSinglePack(&single,0x01,0x0100,Addr,0);
			single.Data[0] = 0xff &sp->lt_gid;
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("Coordinate Config Group Send data:",&single,sizeof(single));
			topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			break;
		case cfg_coorMap:
			Addr = cp->Base_Addr <<16;
			MakeSinglePack(&single,0x10,0x0200,Addr,0);
			single.Data[0] = 0xff & cp->Map_Addr;
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("Coordinate Config MapAddr Send data:",&single,sizeof(single));
			topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			break;
		default:break;
	}
	if(SUCCESS != this->sin_RecvPackage(this,&single,sizeof(light_t),RecvTimeout)){
		debug(DEBUG_single,"Wait Single Response Timeout!\n");
		return FAIL;
	}else if(0 != single.Data[0]){
		debug(DEBUG_single,"^^^^^Config MapAddr Or Group fail!\n");
		return FAIL;
	}
	return SUCCESS;
}

static void Display_package(const char *Message,void *buffer,int size)
{
	assert_param(Message,NULL,;);
	assert_param(buffer,NULL,;);

	printf("%s",Message);
	for(int i=0;i<size;++i) printf("%02x ",((u8*)buffer)[i]);
	printf("\n");
}

static int sin_RecvPackage(Single_t *this,void *buffer,int size,int timeout)
{
	assert_param(this,NULL,FAIL);
	assert_param(buffer,NULL,FAIL);
	appitf_t *topuser = this->topuser;
	u8 *Pbuf = buffer;
	memset(buffer,0,size);
	do{ topuser->Serial->serial_recv(topuser->Serial,COM_485,(char*)buffer,1,100000);
		if(0xff == *Pbuf) break;
		else printf("."),fflush(stdout);
	}while(timeout--);

	if(timeout <= 0){
		debug(DEBUG_single," Wait Header Timeout!\n");
		return FAIL;
	}
	int res = topuser->Serial->serial_recv(topuser->Serial,COM_485,(char*)(Pbuf+1),size-1,1000000);
	if(SUCCESS != res){
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		return FAIL;
	}
	this->Display(">>>>>Recv data:",Pbuf,size);
	if(SUCCESS != topuser->Crc16(crc_check,Pbuf+size-2,Pbuf,size-2)){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int sin_open(Single_t *this,int cmd, u32 Addr, u32 light)
{
	assert_param(this,NULL,FAIL);

	light_t single;
	appitf_t *topuser = this->topuser;
	int res = FAIL;
	int repcnt = 3;
	topuser->Serial->serial_flush(topuser->Serial,COM_485);
	switch(cmd){
		case cmd_single:
		repeat:
			topuser->Serial->serial_flush(topuser->Serial,COM_485);
			MakeSinglePack(&single,0x01,0x0001,Addr,light);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("Single open Send data:",&single,sizeof(single));
			topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			if(SUCCESS != this->sin_RecvPackage(this,&single,sizeof(light_t),RecvTimeout) ||\
				0 != single.Data[0] ){
				if(repcnt--) goto repeat;	//send package again until repcnt == 0
				/* set warn_flags error! */
				topuser->warn->warn_setflags(topuser->warn,Warn_Single_open,sw_single,Addr&0xffff);
				return FAIL;
			}
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,1,so_single,Addr&0xffff);
			/* set warn_flags ok! */
			topuser->warn->warn_cleanflags(topuser->warn,Warn_Single_open,sw_single,Addr&0xffff);
			return SUCCESS;
		case cmd_group:
			MakeSinglePack(&single,0x02,0x0001,Addr,light);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("group open Send data:",&single,sizeof(single));
			res = topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			/* The first response to pc */
			this->sin_reply(this,02,0x42,res);
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,1,so_group,0xff&(Addr>>16));
			break;
		case cmd_broadcast:
			MakeSinglePack(&single,0x04,0x0001,Addr,light);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("Broadcast open Send data:",&single,sizeof(single));
			res = topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			/* The first response to pc */
			this->sin_reply(this,03,0x42,res);
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,1,so_brocast,0);
			break;
		case cmd_grouplight:
			MakeSinglePack(&single,0x02,0x0001,Addr,light);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("group open Send data:",&single,sizeof(single));
			res = topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			/* The first response to pc */
			this->sin_reply(this,02,0x47,res);
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,1,so_group,0xff&(Addr>>16));
			break;
		case cmd_broadlight:
			MakeSinglePack(&single,0x04,0x0001,Addr,light);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("Broadcast open Send data:",&single,sizeof(single));
			res = topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			/* The first response to pc */
			this->sin_reply(this,03,0x47,res);
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,1,so_brocast,0);
			break;
		default:
			debug(DEBUG_single,"Unknow sin_close cmd:%d\n",cmd);
			return FAIL;
	}
	/* Query single status */
	return SUCCESS;
}

static int sin_close(Single_t *this,int cmd, u32 Addr)
{
	assert_param(this,NULL,FAIL);

	light_t single;
	appitf_t *topuser = this->topuser;
	int res = FAIL;
	int repcnt = 3;
	topuser->Serial->serial_flush(topuser->Serial,COM_485);
	switch(cmd){
		case cmd_single:
		repeat:
			topuser->Serial->serial_flush(topuser->Serial,COM_485);
			MakeSinglePack(&single,0x01,0x0002,Addr,0);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("group open Send data:",&single,sizeof(single));
			topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			if(SUCCESS != this->sin_RecvPackage(this,&single,sizeof(light_t),RecvTimeout) ||\
				0 != single.Data[0] ){
				if(repcnt--)  goto repeat;
				/* set warn_flags error! */
				topuser->warn->warn_setflags(topuser->warn,Warn_Single_close,sw_single,Addr&0xffff);
				return FAIL;
			}
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,0,so_single,Addr&0xffff);
			/* set warn_flags ok! */
			topuser->warn->warn_cleanflags(topuser->warn,Warn_Single_close,sw_single,Addr&0xffff);
			return SUCCESS;
		case cmd_group:
			MakeSinglePack(&single,0x02,0x0002,Addr,0);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("group close Send data:",&single,sizeof(single));
			res = topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			/* The first response to pc */
			this->sin_reply(this,02,0x43,res);
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,0,so_group,0xff&(Addr>>16));
			break;
		case cmd_broadcast:
			MakeSinglePack(&single,0x04,0x0002,Addr,0);
			topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
			this->Display("group close Send data:",&single,sizeof(single));
			res = topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
			/* The first response to pc */
			this->sin_reply(this,03,0x43,res);
			/*  set operate_flags */
			topuser->warn->warn_setflags(topuser->warn,0,so_brocast,0);
			break;
		default:
			debug(DEBUG_single,"Unknow sin_close cmd:%d\n",cmd);
			return FAIL;
	}
	/* Query single status */
	return SUCCESS;
}

static int sin_reply(Single_t *this,int cmd,int subcmd,int res)
{
	assert_param(this,NULL,FAIL);

	u8 Ackbuf[24] = {0};
	Ackbuf[0] = Ackbuf[7] = 0x68;
	memcpy(Ackbuf+1,this->topuser->param.CCUID,6);
	Ackbuf[8] = 0x80; Ackbuf[9] = 0x03;
	Ackbuf[10] = cmd; Ackbuf[11] = subcmd;
	Ackbuf[12] = res;
	this->topuser->ethernet->ether_packagecheck(Ackbuf,13);
	return this->topuser->ethernet->ether_send(this->topuser->ethernet,Ackbuf,15);
}

static int Instert_Table(Single_t *this,  Sqlbuf_t *lightinfo,\
					int info_size,   void*src,  int Src_size,  int flags)
{
	assert_param(this,NULL,FAIL);
	assert_param(lightinfo,NULL,FAIL);
	assert_param(src,NULL,FAIL);

	electric_recv_t *const p_elec = src;
	status_recv_t *const p_stat = src;
	Sqlbuf_t *info_start = lightinfo;
	Sqlbuf_t *info_end  = info_start + info_size/sizeof(Sqlbuf_t) - 1;

	int src_count = (flags==Query_elec) ? \
	Src_size/sizeof(electric_recv_t) : Src_size/sizeof(status_recv_t);
	for(int i=0;i<src_count;++i){
		u32 CMP = (flags == Query_elec) ?\
			bigend2littlend_2(p_elec[i].Single_Addr):bigend2littlend_2(p_stat[i].Single_Addr);
		info_start = lightinfo;
		info_end  = info_start + info_size/sizeof(Sqlbuf_t) - 1;
		while(info_start <= info_end){
			if(info_start->SingleAddr == CMP){
				switch(flags){
					case Query_elec:
						info_start->light_V = bigend2littlend_2(p_elec[i].light_V);
						info_start->light_E = bigend2littlend_2(p_elec[i].light_E);
						/* set electric flags */
						if(0xfefe == info_start->light_V  ||  0xfefe == info_start->light_E)
							info_start->Warn_flags |= Warn_Single_electric;
						break;
					case Query_stat:
						info_start->status = p_stat[i].stat;
						info_start->light = p_stat[i].light;
						#ifdef Config_PWM_
							info_end->light = (p_stat[i].light<PeakPwm)?PeakPwm-p_stat[i].light:0xfe;
						#endif
						/* set flags about status */
						if(0xfe == info_start->status  || 0xfe == info_start->light)
							info_start->Warn_flags |= Warn_Single_status;
						break;
					default:break;
				}break;
			}else if(info_end->SingleAddr == CMP){
				switch(flags){
					case Query_elec:
						info_end->light_V = bigend2littlend_2(p_elec[i].light_V);
						info_end->light_E = bigend2littlend_2(p_elec[i].light_E);
						/* set electric flags */
						if(0xfefe == info_end->light_V  ||  0xfefe == info_end->light_E)
							info_end->Warn_flags |= Warn_Single_electric;
						break;
					case Query_stat:
						info_end->status = p_stat[i].stat;
						info_end->light = p_stat[i].light;
						#ifdef Config_PWM_
							info_end->light = (p_stat[i].light<PeakPwm)?PeakPwm-p_stat[i].light:0xfe;
						#endif
						/* set flags about status */
						if(0xfe == info_end->status  || 0xfe == info_end->light)
							info_end->Warn_flags |= Warn_Single_status;
						break;
					default:break;
				}break;
			}	//end of  else if(info_end == CMP)
			++info_start;   --info_end;
		}	//end of  while(info_start <= info_end)
	}	//end of for(int i=0;i<src_count;++i)
	return SUCCESS;
}

static int update_status(Single_t *this,Sqlbuf_t *light_info,int info_size)
{
	assert_param(this,NULL,FAIL);
	assert_param(light_info,NULL,FAIL);

	Sqlbuf_t *p_info = light_info;
	int sin_count = info_size/sizeof(Sqlbuf_t);
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	const char *sql = "update db_info_light set Warn_flags=?1,light_V=?2,light_E=?3,"\
			"light_p=?4,rtime=?5,light_status=?6,light_val=?7 where Base_Addr=?8 ;";
	if( SQLITE_OK != sqlite3_open("./cc_corl.db",&db) ){
		debug(DEBUG_sqlite3,"In %s %s %d:Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);
	/* 显式的开启一个事物处理,大幅度提高插入效率 */
	sqlite3_exec(db,"BEGIN TRANSACTION", NULL, NULL,NULL);
	/* 准备对象 */
	if( SQLITE_OK != sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL) ){
		debug(DEBUG_sqlite3,"In %s %s %d:Prepare Sqlite fail!\n",__FILE__,__func__,__LINE__) ;
		goto out;
	}
	for(int i=0;i<sin_count;++i){
		sqlite3_bind_int(stmt, 1, p_info[i].Warn_flags);
		sqlite3_bind_int(stmt, 2, p_info[i].light_V);
		sqlite3_bind_int(stmt, 3, p_info[i].light_E);
		sqlite3_bind_int(stmt, 4, p_info[i].light_V * p_info[i].light_E/10000);
		sqlite3_bind_int(stmt, 5, time(NULL));
		sqlite3_bind_int(stmt, 6, p_info[i].status);
		sqlite3_bind_int(stmt, 7, p_info[i].light);
		sqlite3_bind_int(stmt, 8, p_info[i].SingleAddr);
		if(SQLITE_DONE != sqlite3_step(stmt))
			debug(DEBUG_sqlite3,"In %s %s %d:Sqlite3_step fail\n",__FILE__,__func__,__LINE__);
		sqlite3_reset(stmt);
	}
	/* 必须执行提交,否则数据在内存中没有写入数据库 */
	sqlite3_exec(db,"COMMIT", NULL, NULL,NULL);

	if (stmt)sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	return SUCCESS;
 out:
	if (stmt)sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
 	return FAIL;
}
static int Response_PC(Single_t *this,  Sqlbuf_t *light_info,\
							int info_size,   int cmd,   int reslut,   int flags)
{
	assert_param(this,NULL,FAIL);

	int  _flag = flags&0x0f;
	appitf_t *topuser = this->topuser;
	u8 AckShortbuf[300] = {0} ,*pf = AckShortbuf;

	*pf++ = 0x52;*pf++ = 0x04;*pf++ = cmd;*pf++ = 0x02;
	*pf++ = (_flag == Query_elec)?0x46:0x45;  *pf++ = reslut;
	if(SUCCESS != reslut || NULL == light_info){
		Task_append(AckShortbuf,topuser->Queue);
		return SUCCESS;
	}

	int num = 0;
	u8 *p_buf = AckShortbuf + 5;
	Sqlbuf_t *p_info = light_info;
	int sin_count = info_size/sizeof(Sqlbuf_t);
	/* 跳过回复上位机 */
	if((flags&0xf0) == Query_nores) goto out;
	for(int i=0;i<sin_count;++i){
		if(_flag != Query_elec)
			Makestatus_tag(p_buf,p_info[i].SingleAddr,p_info[i].light,p_info[i].status);
		else
			Makelectric_tag(p_buf,p_info[i].SingleAddr,p_info[i].light_V,p_info[i].light_E);
		if(++num >= 40 || i == sin_count-1){
			*p_buf = reslut;
			AckShortbuf[1] = num*(_flag==Query_elec ? 6:4) + 4;
			AckShortbuf[3] = AckShortbuf[1] - 2;
			Task_append(AckShortbuf,topuser->Queue);
			p_buf = AckShortbuf + 5;
			num = 0;
		}
	}
out:
	return update_status(this, light_info, info_size);
}


static int Query_Action(Single_t *this, int flags, Sqlbuf_t*light_info,  int size)
{
	assert_param(this,NULL,FAIL);
	assert_param(light_info,NULL,FAIL);

	appitf_t *topuser = this->topuser;
	const Sqlbuf_t *p_info = light_info;
	u8* const Sendbuf = (u8*)calloc(1,size*2+16);
	if(!Sendbuf) goto out;

	u8 *pbuf = Sendbuf+12;
	u32 addr = p_info->Coor_Addr << 16;
	int count  = 0,    single_count_max = 0;
	int ctrl = (flags == Query_elec) ? 0x30:0x20;
	for(int i=0;i++<size;){
		if(p_info->Coor_Addr != (0xff&(addr>>16)) ||\
			i == size){
			if(i == size){
				*pbuf++ = 0xff&(p_info->SingleAddr >> 8);
				*pbuf++ = 0xff&p_info->SingleAddr;
				count += 2;
			}
			MakeSinglePack((light_t*)Sendbuf,ctrl,0x0001,addr,count);
			topuser->Crc16(crc_get,Sendbuf+10,Sendbuf,10);
			topuser->Crc16(crc_get,Sendbuf+12+count,Sendbuf+12,count);
			this->Display("Query Send data:",Sendbuf,count+14);
			for(int repeat=3;repeat>=0;--repeat){
				topuser->Serial->serial_flush(topuser->Serial,COM_485);
				topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)Sendbuf,count+14,SendTimeout);
				if(SUCCESS == this->sin_RecvPackage(this,Sendbuf,sizeof(light_t),RecvTimeout) &&\
					0 == Sendbuf[8] && 0 == Sendbuf[9] ) break;
				if(repeat <= 0) goto out;
			}
			if(i == size) break;
			/* update Coordinate */
			addr = p_info->Coor_Addr << 16;
			pbuf = Sendbuf+12;
			count = 0;
		}	//end of if(p_info->Coor_Addr != 0xff&(addr>>16))
		*pbuf++ = 0xff&(p_info->SingleAddr >> 8);
		*pbuf++ = 0xff&p_info->SingleAddr;
		count += 2;  ++p_info;
		if(single_count_max < count) single_count_max = count;
	}	//end of for(int i=0;i<size;++i)
	free(Sendbuf);
	//debug(1,"Wait Query %d S.\n",single_count_max/4);
	//sleep(single_count_max/4);
	return SUCCESS;
out:
	free(Sendbuf);
	return FAIL;
}

static int Query_electric(Single_t *this,int flags, int Is_res)
{
	assert_param(this,NULL,FAIL);

	appitf_t *topuser = (appitf_t*)this->topuser;
	sql_t *sqlite = topuser->sqlite;
	light_t single;
	Sqlbuf_t *light_info = NULL;
	electric_recv_t *recvbuf = NULL;
	CoordiAddr_t *CoordiAddr = NULL;
	int single_count_max = 0;
	int lightinfo_size = 0;
	int Coordinate_count = 0;

	/* if (-1 != flags) is group electric Query */
	if(-1 != flags){
		/* get single light count */
		sqlite->sql_select(Asprintf("select count(Base_Addr) from db_light "\
			"where lt_gid=%d;",flags),(char*)&lightinfo_size,sizeof(lightinfo_size),1,0);
		/* get Coordinate count */
		sqlite->sql_select(Asprintf("select count(distinct Coor_id) from db_light "\
			"where lt_gid=%d;",flags),(char*)&Coordinate_count,sizeof(int),1,0);
		if(lightinfo_size <= 0 || Coordinate_count <= 0) {
			debug(DEBUG_single,"get count for lightinfo or coordinate error!\n");
			goto out;
		}
		/* malloc for Coordinator list */
		CoordiAddr = calloc(Coordinate_count,sizeof(CoordiAddr_t));
		if(!CoordiAddr) goto out;
		/* get coordinator list */
		if(SUCCESS != sqlite->sql_select( Asprintf("select distinct Coor_id from db_light  "\
			"where lt_gid=%d;",flags),(char*)CoordiAddr,sizeof(CoordiAddr_t),Coordinate_count,0)){
			debug(DEBUG_single,"get coordinator Addr list fail!\n");
			goto out;
		}
		/* get single count in the same coordinator */
		for(int i=0; i<Coordinate_count; ++i){
			sqlite->sql_select(Asprintf("select count(Base_Addr) from db_light "\
				"where Coor_id=%d AND lt_gid=%d;",CoordiAddr[i].Coor_Addr,\
									flags),(char*)&CoordiAddr[i].single_Count,sizeof(int),1,0);
			if(CoordiAddr[i].single_Count <= 0) goto out;
			if(single_count_max < CoordiAddr[i].single_Count)
				single_count_max = CoordiAddr[i].single_Count;
		}
		light_info = calloc(lightinfo_size,sizeof(Sqlbuf_t));
		recvbuf = calloc(single_count_max,sizeof(electric_recv_t) + 1);
		if(!light_info || !recvbuf ) goto out;
		/* get data from db_info_light */
		if( SUCCESS != sqlite->sql_select(Asprintf("select db_light.Coor_id,db_light.Base_Addr,"\
			"db_info_light.Warn_flags,db_info_light.light_status,db_info_light.light_val from db_info_light,"\
			"db_light  where db_light.lt_gid=%d AND db_light.Base_Addr=db_info_light.Base_Addr order by "\
			"db_light.Coor_id;",flags),(char*)light_info,sizeof(Sqlbuf_t),lightinfo_size,0)){
			debug(DEBUG_single,"get light info fail!\n");
			goto out;
		}
	/********************Upper is group Query********************/
	}else{
		lightinfo_size = Get_light_info_count(sqlite,"Base_Addr");
		Coordinate_count = Get_CountByColumn(sqlite,"db_light","distinct Coor_id");

		if(lightinfo_size <= 0 || Coordinate_count <= 0) goto out;
		/* malloc for Coordinator list */
		CoordiAddr = calloc(Coordinate_count,sizeof(CoordiAddr_t));
		if(!CoordiAddr) goto out;
		/* get coordinator list */
		if(SUCCESS != sqlite->sql_select( "select distinct Coor_id from db_light;",\
						(char*)CoordiAddr,sizeof(CoordiAddr_t),Coordinate_count,0)){
			debug(DEBUG_single,"get coordinator Addr list fail!\n");
			goto out;
		}
		/* get single count in the same coordinator */
		for(int i=0; i<Coordinate_count; ++i){
			sqlite->sql_select(Asprintf("select count(Base_Addr) from db_light where Coor_id=%d;",\
							CoordiAddr[i].Coor_Addr),(char*)&CoordiAddr[i].single_Count,sizeof(int),1,0);
			if(CoordiAddr[i].single_Count <= 0) goto out;
			if(single_count_max < CoordiAddr[i].single_Count)
				single_count_max = CoordiAddr[i].single_Count;
		}
		light_info = calloc(lightinfo_size,sizeof(Sqlbuf_t));
		recvbuf = calloc(single_count_max,sizeof(electric_recv_t) + 1);
		if(!light_info || !recvbuf ) goto out;
		/* get data from db_info_light */
		if( SUCCESS != sqlite->sql_select("select db_light.Coor_id,db_light.Base_Addr,"\
			"db_info_light.Warn_flags,db_info_light.light_status,db_info_light.light_val "\
			"from  db_info_light, db_light where db_light.Base_Addr=db_info_light.Base_Addr "\
			"order by db_light.Coor_id;",(char*)light_info,sizeof(Sqlbuf_t),lightinfo_size,0)){
			debug(DEBUG_single,"get light info fail!\n");
			goto out;
		}
	}	//end  of if(-1 != flags)

	/* send the Query cmd */
	if(SUCCESS != Query_Action(this,Query_elec,light_info,lightinfo_size)){
		debug(DEBUG_single,"Query_Action_electric error!\n");
		goto out;
	}
	/*  get electric from coordinator */
	int getcnt = GetSingleCunt<<8;
	int Addr = 0 ,Waittime = 0;
	for(int j=0;j<Coordinate_count;){
		Addr = CoordiAddr[j].Coor_Addr << 16;
		Waittime = CoordiAddr[j].single_Count*10;
		printf("\nWait time =%d\n",Waittime);
		MakeSinglePack(&single,0x50,0x0001,Addr,getcnt);
		topuser->Serial->serial_flush(topuser->Serial,COM_485);
		topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
		this->Display("Query electric Send data:",&single,sizeof(single));
		topuser->Serial->serial_send(topuser->Serial,COM_485,\
							(s8*)&single,sizeof(light_t),SendTimeout);
		/* when get error triplicate */
		for(int repeat=3;repeat--;){
			if(SUCCESS == this->sin_RecvPackage(this,&single,\
				sizeof(light_t),RecvTimeout+Waittime) )  break;
			Waittime = 0;
			if(repeat <= 0) goto out;
		}
		int reslen = (single.Data[0]<<8) | single.Data[1];
		if(reslen >0 && SUCCESS == topuser->Serial->serial_recv(topuser->Serial,\
			COM_485,(char*)recvbuf,reslen+2,3000000)){
			if(SUCCESS != topuser->Crc16(crc_check,(u8*)recvbuf+reslen,(u8*)recvbuf,reslen)){
				debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
				goto out;
			}
			this->Display("Recv state:",recvbuf,reslen+2);
			/* instert status to sql */
			Instert_Table(this,light_info,lightinfo_size*sizeof(Sqlbuf_t),recvbuf,reslen,Query_elec);
		}else debug(DEBUG_single,"Responselen <=0 or DeviceRecv485_v2 fail!\n");
		/* Is take out of coordinator */
		if(reslen/6 >= CoordiAddr[j].single_Count){
			++j;
		}else  CoordiAddr[j].single_Count -=  reslen/6;
	}
	/* The second response to PC */
	if(-1 == flags )
		Response_PC(this,light_info,lightinfo_size*sizeof(Sqlbuf_t),0x03,SUCCESS,Query_elec|Is_res);
	else
		Response_PC(this,light_info,lightinfo_size*sizeof(Sqlbuf_t),0x02,SUCCESS,Query_elec|Is_res);
	free(light_info);
	free(recvbuf);
	free(CoordiAddr);
	return SUCCESS;
out:
	/* The second response to PC */

	if(-1 == flags )
		Response_PC(this,NULL,0,0x03,FAIL,Query_elec|Is_res);
	else
		Response_PC(this,NULL,0,0x02,FAIL,Query_elec|Is_res);
	free(light_info);
	free(recvbuf);
	free(CoordiAddr);
	return FAIL;
}

static int sin_Queryelectric(Single_t *this,int cmd, u32 Addr)
{
	assert_param(this,NULL,FAIL);
	light_t single;
	appitf_t *topuser = (appitf_t*)this->topuser;
	u32 status = 0;
	u8 Ackbuf[32] ={0};
	u8 *pbuf = Ackbuf;
	int rtn = 0;
	switch(cmd){
		case cmd_single:
			for(int repeat=3;repeat;--repeat){
				if(repeat <= 0){/* set error flags */break;}
				topuser->Serial->serial_flush(topuser->Serial,COM_485);
				MakeSinglePack(&single,0x01,0x0010,Addr,0);
				topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
				this->Display("Single Query status Send data:",&single,sizeof(single));
				topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
				rtn = this->sin_RecvPackage(this,&single,sizeof(light_t),RecvTimeout);
				if(rtn != SUCCESS) continue;
				status = single.Data[0]<<24 | single.Data[1] << 16;
				MakeSinglePack(&single,0x01,0x0020,Addr,0);
				topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
				this->Display("Single Query status Send data:",&single,sizeof(single));
				topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
				rtn = this->sin_RecvPackage(this,&single,sizeof(light_t),RecvTimeout);
				if(rtn == SUCCESS) break;
			}
			status |= single.Data[0]<<8 | single.Data[1];
			MakeSingleQueryRes(pbuf,0x0A,((01<<8)|0x45),Addr,status,rtn);
			Task_append(Ackbuf,topuser->Queue);
			return rtn;
		case cmd_group:
			/* first response to pc */
			this->sin_reply(this,0x02,0x46,SUCCESS);
			return Query_electric(this,0xff&(Addr>>24),Query_res);
		case cmd_broadcast:
			/* first response to pc */
			this->sin_reply(this,0x03,0x46,SUCCESS);
			return Query_electric(this,-1,Query_res);
		default:break;
	}
	return SUCCESS;
}

static int Query_status(Single_t*this,int flags,int Is_res)
{
	assert_param(this,NULL,FAIL);

	appitf_t *topuser = (appitf_t*)this->topuser;
	sql_t *sqlite = topuser->sqlite;
	light_t single;
	Sqlbuf_t *light_info = NULL;
	status_recv_t *recvbuf = NULL;
	CoordiAddr_t *CoordiAddr = NULL;
	int single_count_max = 0;
	int lightinfo_size = 0;
	int Coordinate_count = 0;

	/* if (-1 != flags) is group electric Query */
	if(-1 != flags){
		/* get single light count */
		sqlite->sql_select(Asprintf("select count(Base_Addr) from db_light "\
			"where lt_gid=%d;",flags),(char*)&lightinfo_size,sizeof(lightinfo_size),1,0);
		/* get Coordinate count */
		sqlite->sql_select(Asprintf("select count(distinct Coor_id) from db_light "\
			"where lt_gid=%d;",flags),(char*)&Coordinate_count,sizeof(int),1,0);
		if(lightinfo_size <= 0 || Coordinate_count <= 0) {
			debug(DEBUG_single,"get count for lightinfo or coordinate error!\n");
			goto out;
		}
		/* malloc for Coordinator list */
		CoordiAddr = calloc(Coordinate_count,sizeof(CoordiAddr_t));
		if(!CoordiAddr) goto out;
		/* get coordinator list */
		if(SUCCESS != sqlite->sql_select( Asprintf("select distinct Coor_id from db_light  "\
			"where lt_gid=%d;",flags),(char*)CoordiAddr,sizeof(CoordiAddr_t),Coordinate_count,0)){
			debug(DEBUG_single,"get coordinator Addr list fail!\n");
			goto out;
		}
		/* get single count in the same coordinator */
		for(int i=0; i<Coordinate_count; ++i){
			sqlite->sql_select(Asprintf("select count(Base_Addr) from db_light "\
				"where Coor_id=%d AND lt_gid=%d;",CoordiAddr[i].Coor_Addr,\
									flags),(char*)&CoordiAddr[i].single_Count,sizeof(int),1,0);
			if(CoordiAddr[i].single_Count <= 0) goto out;
			if(single_count_max < CoordiAddr[i].single_Count)
				single_count_max = CoordiAddr[i].single_Count;
		}
		light_info = calloc(lightinfo_size,sizeof(Sqlbuf_t));
		recvbuf = calloc(single_count_max,sizeof(status_recv_t) + 1);
		if(!light_info || !recvbuf ) goto out;
		/* get data from db_info_light */
		if( SUCCESS != sqlite->sql_select(Asprintf("select db_light.Coor_id,db_light.Base_Addr,"\
			"db_info_light.Warn_flags,db_info_light.light_status,db_info_light.light_val,db_info_light.light_V,"\
			"db_info_light.light_E  from db_info_light,db_light  where db_light.lt_gid=%d AND db_light.Base_Addr="\
			"db_info_light.Base_Addr order by db_light.Coor_id;",flags),(char*)light_info,sizeof(Sqlbuf_t),lightinfo_size,0)){
			debug(DEBUG_single,"get light info fail!\n");
			goto out;
		}
	/********************Upper is group Query********************/
	}else{
		lightinfo_size = Get_light_info_count(sqlite,"Base_Addr");
		Coordinate_count = Get_CountByColumn(sqlite,"db_light","distinct Coor_id");
		if(lightinfo_size <= 0 || Coordinate_count <= 0) goto out;
		/* malloc for Coordinator list */
		CoordiAddr = calloc(Coordinate_count,sizeof(CoordiAddr_t));
		if(!CoordiAddr) goto out;
		/* get coordinator list */
		if(SUCCESS != sqlite->sql_select( "select distinct Coor_id from db_light;",\
						(char*)CoordiAddr,sizeof(CoordiAddr_t),Coordinate_count,0)){
			debug(DEBUG_single,"get coordinator Addr list fail!\n");
			goto out;
		}
		/* get single count in the same coordinator */
		for(int i=0; i<Coordinate_count; ++i){
			sqlite->sql_select(Asprintf("select count(Base_Addr) from db_light where Coor_id=%d;",\
							CoordiAddr[i].Coor_Addr),(char*)&CoordiAddr[i].single_Count,sizeof(int),1,0);
			if(CoordiAddr[i].single_Count <= 0) goto out;
			if(single_count_max < CoordiAddr[i].single_Count)
				single_count_max = CoordiAddr[i].single_Count;
		}
		light_info = calloc(lightinfo_size,sizeof(Sqlbuf_t));
		recvbuf = calloc(single_count_max,sizeof(status_recv_t) + 1);
		if(!light_info || !recvbuf ) goto out;
		/* get data from db_info_light */
		if( SUCCESS != sqlite->sql_select("select db_light.Coor_id,db_light.Base_Addr,"\
			"db_info_light.Warn_flags,db_info_light.light_status,db_info_light.light_val,db_info_light.light_V,"\
			"db_info_light.light_E from  db_info_light, db_light where db_light.Base_Addr=db_info_light.Base_Addr "\
			"order by db_light.Coor_id;",(char*)light_info,sizeof(Sqlbuf_t),lightinfo_size,0)){
			debug(DEBUG_single,"get light info fail!\n");
			goto out;
		}
	}	//end  of if(-1 != flags)

	/* send the Query cmd */
	if(SUCCESS != Query_Action(this,Query_stat,light_info,lightinfo_size)){
		debug(DEBUG_single,"Query_Action_electric error!\n");
		goto out;
	}
	/*  get electric from coordinator */
	int getcnt = GetSingleCunt<<8;
	int Addr = 0, Waitime = 0;
	for(int j=0;j<Coordinate_count;){
		Addr = CoordiAddr[j].Coor_Addr << 16;
		Waitime = CoordiAddr[j].single_Count * 10;
		printf("\nWait time =%d\n",Waitime);

		MakeSinglePack(&single,0x40,0x0001,Addr,getcnt);
		topuser->Serial->serial_flush(topuser->Serial,COM_485);
		topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
		this->Display("Query electric Send data:",&single,sizeof(single));
		topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
		/* when get error triplicate */
		for(int repeat=3;repeat--;){
			if(SUCCESS == this->sin_RecvPackage(this,&single,\
					sizeof(light_t),RecvTimeout+Waitime) ) break;
			Waitime = 0;
			if(repeat <= 0) goto out;
		}
		int reslen = (single.Data[0]<<8) | single.Data[1];
		if(reslen >0 && SUCCESS == topuser->Serial->serial_recv(topuser->Serial,\
			COM_485,(char*)recvbuf,reslen+2,3000000)){
			if(SUCCESS != topuser->Crc16(crc_check,(u8*)recvbuf+reslen,(u8*)recvbuf,reslen)){
				debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
				goto out;
			}
			this->Display("Recv state:",recvbuf,reslen+2);
			/* instert status to sql */
			Instert_Table(this,light_info,lightinfo_size*sizeof(Sqlbuf_t),recvbuf,reslen,Query_stat);
		}else debug(DEBUG_single,"Responselen <=0 or DeviceRecv485_v2 fail!\n");
		/* Is take out of coordinator */
		if(reslen/4 >= CoordiAddr[j].single_Count){
			++j;
		}else  CoordiAddr[j].single_Count -=  reslen/4;
	}
	/* The second response to PC */
	if(-1 == flags )
		Response_PC(this,light_info,lightinfo_size*sizeof(Sqlbuf_t),0x03,SUCCESS,Query_stat|Is_res);
	else
		Response_PC(this,light_info,lightinfo_size*sizeof(Sqlbuf_t),0x02,SUCCESS,Query_stat|Is_res);
	free(light_info);
	free(recvbuf);
	free(CoordiAddr);
	return SUCCESS;
out:
	/* The second response to PC */

	if(-1 == flags )
		Response_PC(this,NULL,0,0x03,FAIL,Query_stat|Is_res);
	else
		Response_PC(this,NULL,0,0x02,FAIL,Query_stat|Is_res);
	free(light_info);
	free(recvbuf);
	free(CoordiAddr);
	return FAIL;
}

static int sin_Querystatus(Single_t *this,int cmd, u32 Addr)
{
	assert_param(this,NULL,FAIL);
	light_t single;
	appitf_t *topuser = (appitf_t*)this->topuser;
	u32 status = 0;
	u8 Ackbuf[32] ={0};
	u8 *pbuf = Ackbuf;
	int rtn = 0;
	switch(cmd){
		case cmd_single:
			for(int repeat=3;repeat--;){
				topuser->Serial->serial_flush(topuser->Serial,COM_485);
				MakeSinglePack(&single,0x01,0x0400,Addr,0);
				topuser->Crc16(crc_get,single.Crc16,(u8*)&single,sizeof(single)-2);
				this->Display("Single Query status Send data:",&single,sizeof(single));
				topuser->Serial->serial_send(topuser->Serial,COM_485,(s8*)&single,sizeof(light_t),SendTimeout);
				rtn = this->sin_RecvPackage(this,&single,sizeof(light_t),RecvTimeout);
				if(rtn == SUCCESS)  break;
				if(repeat <= 0){/* set error flags */}
			}
			status = single.Data[0]<<8 | single.Data[1];
			MakeSingleQueryRes(pbuf,8,((01<<8)|0x45),Addr,status,rtn);
			Task_append(Ackbuf,topuser->Queue);
			break;
		case cmd_group:
			/* first response to pc */
			this->sin_reply(this,0x02,0x45,SUCCESS);
			return Query_status(this,0xff&(Addr>>24),Query_res);
		case cmd_broadcast:
			/* first response to pc */
			this->sin_reply(this,0x03,0x45,SUCCESS);
			return Query_status(this,-1,Query_res);
		default:break;
	}
	return SUCCESS;
}

static void sin_release(struct Single_t **this)
{
	memset(*this,0,sizeof(Single_t));
	free(*this);
	*this = NULL;
}

Single_t *single_Init(struct appitf_t *topuser)
{
	Single_t *single = (Single_t*)malloc(sizeof(Single_t));
	if(!single) return NULL;

	single->topuser = topuser;
	single->Display = Display_package;
	single->sin_RecvPackage = sin_RecvPackage;
	single->sin_open = sin_open;
	single->sin_close = sin_close;
	single->sin_reply = sin_reply;
	single->sin_Queryelectric = sin_Queryelectric;
	single->sin_Querystatus = sin_Querystatus;
	single->sin_config = sin_Config;
	single->sin_release = sin_release;


	if(!single->topuser || !single->sin_open || !single->sin_close || !single->sin_release ||\
		!single->sin_reply || !single->sin_config || !single->sin_Queryelectric ||\
		!single->sin_Querystatus || !single->sin_RecvPackage || !single->Display){
		free(single);
		return NULL;
	}
	return single;
}
