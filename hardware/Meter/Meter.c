/******************************************************************
** 文件名:	Meter.h=c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "Meter.h"
#include "Interface.h"

#define SendTimeout 1000000		//1s
#define RecvTimeout  20
#define WAITDI_RES   200

#define Set_status(ptr,_sta)	do{\
	*(ptr) = 0xff&(_sta>>24);*(ptr+1) = 0xff&(_sta>>16);\
	*(ptr+2) = 0xff&(_sta>>8);*(ptr+3) = 0xff&(_sta); (ptr) += 4;\
}while(0)

static uint8_t inline Get_statusofdi(Meter_t *this,int32_t *result, uint8_t addr)
{
	if(!this || !result) return 0;
	if(addr == 0xfe) return 8;
	*result = this->meter_querydi(this,addr,8);
	if(-1 != *result)	return 8;
	*result = this->meter_querydi(this,addr,16);
	if(-1 != *result)	return 16;
	*result = this->meter_querydi(this,addr,24);
	if(-1 != *result)	return 24;
	return 0;
}

static uint8_t inline Get_statusofdo(Meter_t *this,int32_t *result, uint8_t addr)
{
	if(!this || !result) return 0;
	if(addr == 0xfe)	return 8;
	*result = this->meter_querydo(this,addr,8);
	if(-1 != *result)	return 8;
	*result = this->meter_querydo(this,addr,16);
	if(-1 != *result)	return 16;
	*result = this->meter_querydo(this,addr,24);
	if(-1 != *result)	return 24;
	return 0;
}

static int meter_resatonce(appitf_t *topuser,uint8_t *ackbuf, uint32_t length)
{
	assert_param(topuser,FAIL);
	memcpy(ackbuf+1,topuser->param.CCUID,6);
	get_check_sum(ackbuf,length);
	return topuser->opt_Itf->opt_send(topuser->opt_Itf,ackbuf,length+2);
}

static int meter_sendpackage(Meter_t *this,uint8_t addr,uint8_t do_num,uint32_t stat)
{
	assert_param(this,FAIL);
	/**
	 *  package format:
	 *  slave addr(1byte),ctrl(1byte),start addr(2byte),number of relay(2byte),
	 *  size of next control(1byte),the value for relay the bit is 1 open else close,CRC16(2byte)
	 */
	uint8_t buf[] ={ addr,0x0f,0,0,0,8,1,stat&0xff,0,0,0,0 };
	uint8_t *pf = buf;
	crc_low(pf+8,pf,8);
	this->uart->uart_flush(this->uart);
	display_pack("meter open all Send data",(char*)pf, 10);
	this->uart->uart_send(this->uart,(char*)pf,10,SendTimeout);
	return this->uart->uart_readall(this->uart,(char*)pf);
}

static int meter_insert_sql(Meter_t *this,uint8_t addr,int32_t di_stat,int32_t do_stat,uint16_t dido_cnt)
{
	assert_param(this,FAIL);
	uint8_t di_cnt = 0xff&(dido_cnt>>8), do_cnt = dido_cnt&0xff;
	if(do_stat < 0 || do_cnt == 0)
		do_cnt = Get_statusofdo(this,&do_stat, addr);
	/* 等待小段时间，给继电器响应时间 */
	msleep(WAITDI_RES);
	if(di_stat < 0 || di_cnt == 0)
		di_cnt = Get_statusofdi(this,&di_stat, addr);
	if( 0== do_cnt || 0 == di_cnt)
		return FAIL;
	/* 删除两个月前的数据 */
	time_t del_time = time(NULL) - (2*30*24*60*60);
	this->sql->sql_delete(Asprintf("delete from "CFG_tb_dido" where add_time < %ld;",del_time));
	/* 插入新数据 */
	return this->sql->sql_insert(Asprintf("insert into "\
		CFG_tb_dido"(addr,add_time,ndo,ndi,do_stat,di_stat) "\
		"values(%d,%ld,%d,%d,%d,%d);",addr,time(NULL),do_cnt,di_cnt,do_stat,di_stat));
}

static int meter_update_sql(Meter_t *this,uint8_t addr,int32_t flags,int32_t status)
{
	assert_param(this,FAIL);
	int32_t id = 0;
	this->sql->sql_select(Asprintf("select id from "CFG_tb_dido\
		" where addr=%d order by add_time desc;",addr),(char*)&id,sizeof(int),1,0);
	if(id == 0)	return FAIL;
	switch(flags){
		case UP_DI:
			return this->sql->sql_update(CFG_tb_dido,Asprintf("set di_stat=%d where id=%d",status,id));
		case UP_DO:
			return this->sql->sql_update(CFG_tb_dido,Asprintf("set do_stat=%d where id=%d",status,id));
		default:	return FAIL;
	}
}

static int meter_open(Meter_t *this,uint8_t addr, uint8_t num,uint32_t ndo)
{
	assert_param(this,FAIL);

	int res = SUCCESS;
	uint32_t status = 0;
	if(addr == 0xfe)
		goto broadcast;
	status = this->meter_querydo(this, addr,num);
	if(status == ~0){
		debug(DEBUG_DIDO,"meter open query do error!\n");
		res = FAIL;
		goto out;
	}
broadcast:
	status |= ndo;
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,0x80,0x08,0xe1, 0x01, addr,
		0xff&(status>>24), 0xff&(status>>16),0xff&(status>>8),0xff&status,res,0,0,0
	};
	if(-1 == meter_sendpackage(this,addr,num,status) )
		*(ackbuf+17) = FAIL;
out:
	meter_resatonce(this->topuser, ackbuf,18);
	if(ackbuf[17] != FAIL)
		meter_insert_sql(this,addr,0,status,num);
	return *(ackbuf+17);
}

static int meter_close(Meter_t *this,uint8_t addr, uint8_t num,uint32_t ndo)
{
	assert_param(this,FAIL);
	int32_t res = SUCCESS;
	uint32_t status = ~0;
	if(addr == 0xfe)
		goto broadcast;
	status = this->meter_querydo(this, addr,num);
	if( 0xffffffff == status ){
		debug(DEBUG_DIDO,"meter open query do error!\n");
		res = FAIL;
		goto out;
	}
broadcast:
	status &= ~ndo;
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,0x80,0x08,0xe1, 0x02, addr,
		0xff&(status>>24), 0xff&(status>>16),0xff&(status>>8),0xff&status,res,0,0,0
	};
	if(-1 == meter_sendpackage(this,addr,num,status))
		*(ackbuf+17) = FAIL;
out:
	meter_resatonce(this->topuser, ackbuf,18);
	if(ackbuf[17] != FAIL)
		meter_insert_sql(this,addr,0,status,num);
	return *(ackbuf+17);
}

static int meter_querydi(struct Meter_t *this,uint8_t addr,uint8_t ndi)
{
	assert_param(this,-1);
	/* 设备地址(addr), 指令(02), 起始地址(00,00), 查询数量(00,08) */
	char slave[16] = {addr,2,0,0,0,ndi,0};
	uint8_t *ps = (uint8_t*)slave;
	crc_low(ps+6,ps,6);
	this->uart->uart_flush(this->uart);
	display_pack("meter readi  Send data",slave, 8);
	this->uart->uart_send(this->uart,slave,8,SendTimeout);
	int len = this->uart->uart_readall(this->uart, slave);
	display_pack("meter readi  recv data", slave, len);
	if(*(ps+1) != 0x02){
		debug(DEBUG_DIDO,"meter readi error!\n");
		return -1;
	}
	int chk = crc_cmp_low(ps+len-2,ps,len-2);
	if(SUCCESS != chk){
		debug(DEBUG_DIDO,"Meter read di crc_check error!\n");
		return -1;
	}
	switch(*(ps+2)){
		case 1:return *(ps+3);
		case 2: return ps[3] | ps[4] << 8;
		case 3: return ps[3] | ps[4] << 8 | ps[5] << 16;
		case 4: return ps[3] | ps[4] << 8 | ps[5] << 16 | ps[5] << 24;
		default : return -1;
	}
}

static int meter_querydo(struct Meter_t *this,uint8_t addr,uint8_t ndo)
{
	assert_param(this,-1);
	/* 设备地址, 指令, 起始地址(),查询数量(),CRC */
	char slave[16] = {addr,1,0,0,0,ndo,0};
	uint8_t *ps = (uint8_t *)slave;
	crc_low(ps+6,ps,6);
	this->uart->uart_flush(this->uart);
	display_pack("meter reado  Send data",slave, 8);
	this->uart->uart_send(this->uart,slave,8,SendTimeout);
	int len = this->uart->uart_readall(this->uart, slave);
	//this->uart->uart_recv(this->uart, slave, 6,SendTimeout);
	display_pack("meter reado  recv data", slave, len);
	if(*(ps+1) != 0x01){
		debug(DEBUG_DIDO,"meter reado error!\n");
		return -1;
	}
	int chk = crc_cmp_low(ps+len-2,ps,len-2);
	if(SUCCESS != chk){
		debug(1,"Meter read do crc_check error!\n");
		return -1;
	}
	switch(*(ps+2)){
		case 1:return *(ps+3);
		case 2: return ps[3] | ps[4] << 8;
		case 3: return ps[3] | ps[4] << 8 | ps[5] << 16;
		case 4: return ps[3] | ps[4] << 8 | ps[5] << 16 | ps[5] << 24;
		default : return -1;
	}
}

static int meter_readi(Meter_t *this,uint8_t addr,uint8_t num)
{
	assert_param(this,FAIL);
	int status = meter_querydi(this,addr,num);
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,
		0x80, 0x08, 0xe1, 0x04, addr,
		0xff&(status>>24),0xff&(status>>16),
		0xff&(status>>8),0xff&status, SUCCESS, 0,0,0
	};
	if(status == -1)
		*(ackbuf + 17)  = FAIL;
	meter_resatonce(this->topuser, ackbuf,18);
	if(ackbuf[17] != FAIL)
		meter_update_sql(this, addr,UP_DI, status);
	return *(ackbuf + 17);
}

static int meter_reado(Meter_t *this,uint8_t addr,uint8_t num)
{
	assert_param(this,FAIL);
	int status = meter_querydo(this,addr,num);
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,
		0x80, 0x08, 0xe1, 0x03, addr,
		0xff&(status>>24),0xff&(status>>16),
		0xff&(status>>8),0xff&status, SUCCESS, 0,0,0
	};
	if(status == -1)
		*(ackbuf + 17) = FAIL;
	meter_resatonce(this->topuser, ackbuf,18);
	if(ackbuf[17] != FAIL)
		meter_update_sql(this, addr,UP_DO, status);
	return *(ackbuf + 17);
}

static int meter_flashopen(struct Meter_t *this,uint8_t addr,uint8_t num,uint32_t ndo,int32_t ms)
{
	assert_param(this,FAIL);
	uint32_t status = this->meter_querydo(this, addr,num);
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,
		0x80,0x08,0xe1, 0x05, addr,
		0xff&(status>>24), 0xff&(status>>16),
		0xff&(status>>8),0xff&status, SUCCESS,0,0,0
	};
	if(status == ~0){
		debug(DEBUG_DIDO,"meter flash open query do error!\n");
		*(ackbuf+17) = FAIL;
		goto out;
	}
	if(-1 == meter_sendpackage(this,addr,num,status|ndo) ){
		*(ackbuf+17) = FAIL;
		goto out;
	}
	meter_resatonce(this->topuser, ackbuf,18);
	msleep(ms*100);
	if(-1 == meter_sendpackage(this,addr,num,status ))
		*(ackbuf+17) = FAIL;
out:
	return *(ackbuf+17);
}

static int Query_dido(struct Meter_t*this, uint8_t addr, uint32_t *result)
{
	assert_param(this,FAIL);
	assert_param(result,FAIL);

	Get_statusofdo(this,(int32_t*)result,addr);
	Get_statusofdi(this,(int32_t*)result+1,addr);

	if(*result != -1 && *(result+1) != -1)
		return SUCCESS;
	return FAIL;
}

static int meter_query_dido(struct Meter_t*this, uint8_t cnt,uint8_t *pd)
{
	assert_param(this,FAIL);
	assert_param(pd,FAIL);
	debug(DEBUG_DIDO,"meter count is %d\n",cnt);
	uint8_t addr = 0, ret = SUCCESS;
	uint8_t ackbuf[128] = {0x68,0,0,0,0,0,0,0x68,0x80,cnt*9+4,0xe1,0x06,cnt,0};
	uint8_t *pk = ackbuf + 13;
	uint32_t stat[2] = {0};
	while(cnt--){
		*pk++ = addr = *pd++;
		if(SUCCESS == Query_dido(this,addr,stat)){
			Set_status(pk,*(stat+1));
			Set_status(pk,*stat);
		}else{
			*pk += 8;	ret = FAIL;
		}
	}
	*pk = ret;
	return meter_resatonce(this->topuser, ackbuf,ackbuf[9]+10);
}

static void meter_recover_status(struct Meter_t *this)
{
	assert_param(this,;);
	struct status { uint32_t Addr; uint32_t ndo; uint32_t stat;uint32_t time;  } pstatus[10];
	bzero(pstatus,sizeof(pstatus));
	/* 获取DIDO设备地址和状态 */
	/* select * from (select * from db_dido_info order by add_time desc) group by addr; */
	this->sql->sql_select("select addr,ndo,do_stat,add_time from db_dido_info group by addr order by addr;",\
				(char*)pstatus,sizeof(pstatus[0]),sizeof(pstatus)/sizeof(pstatus[0]),0);
	/* 解析数据 */
	struct status *broadcast = NULL;
	/* 查询之前是否有过广播操作 */
	for(int i=0; i<sizeof(pstatus)/sizeof(pstatus[0]); ++i){
		if(pstatus[i].Addr == 0xfe){
			broadcast = &pstatus[i];
			break;
		}	//end of if(pstatus[i].addr == 0xfe)
	}	//end of for(int i=0; i<sizeof(pstatus)
	/* 有广播操作，与单播操作时间对比 */
	if(broadcast)
		for(int i=0; i < sizeof(pstatus)/sizeof(pstatus[0]); ++i){
			if(pstatus[i].Addr != 0xfe && pstatus[i].time < broadcast->time){
				pstatus[i].stat = broadcast->stat;
			}
		}	//end of for(int i=0;

	/* 恢复DIDO状态 */
	for(int i=0; pstatus[i].Addr != 0; ++i){
		debug(DEBUG_DIDO,"addr=0x%X,ndo=%d,stat=0x%X,add_time=%u\n",\
			pstatus[i].Addr,pstatus[i].ndo,pstatus[i].stat,pstatus[i].time);
		if(pstatus[i].Addr != 0xfe )
			meter_sendpackage(this,pstatus[i].Addr,pstatus[i].ndo,pstatus[i].stat);
	}
}

static void meter_release(struct Meter_t *this)
{
	assert_param(this,;);
	DELETE(this->sql,sql_release);
	DELETE(this->uart,uart_relese,CFG_COMDIDO);
	if(this->Point_flag)
		FREE(this);
}

Meter_t *meter_init(Meter_t *this,struct appitf_t *topuser)
{
	assert_param(topuser,NULL);
	Meter_t  *const pth = this;
	if(!pth){
		this = malloc(sizeof(Meter_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(Meter_t));
	this->Point_flag = (!pth)?1:0;

	this->topuser = topuser;
	this->sql = sql_Init(NULL);
	if(!this->sql) goto out;
	#ifdef Config_UART
	this->uart = uart_init(NULL,CFG_COMDIDO,"9600,8,1,N");
	if(!this->uart) goto out;
	#endif

	this->meter_open = meter_open;
	this->meter_close = meter_close;
	this->meter_readi = meter_readi;
	this->meter_reado = meter_reado;
	this->meter_flashopen = meter_flashopen;
	this->meter_release = meter_release;
	this->meter_querydi = meter_querydi;
	this->meter_querydo = meter_querydo;
	this->meter_query_dido = meter_query_dido;
	this->meter_recover_status = meter_recover_status;
	meter_recover_status(this);		//恢复重启前的状态
	return this;
out:
	if(!this->sql) DELETE(this->sql,sql_release);
	if(!this->uart) DELETE( this->uart,uart_relese,CFG_COMDIDO);
	if(!pth) FREE(this);
	return NULL;
}
