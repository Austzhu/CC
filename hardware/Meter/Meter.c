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


#if 0
#define MakeSlave(pf,_addr,_ctrl,_cmd)  do{\
	pf[0] = _addr; pf[1] = _ctrl;pf[4] = 0xff&((_cmd)>>8); \
	pf[5] = 0xff&(_cmd);\
}while(0)

#define MakeSlaveOpenAll(pf,_addr,oc) do{\
	pf[0] = _addr; pf[1] = 0x0f; pf[2] = 0; pf[3] = 0;\
	pf[4] = 0; pf[5] = 8;  pf[6] = 1;  pf[7] = 0xff&(oc);\
}while(0)


#define Task_merter(buf,ptr) do{\
	if(ptr->get_Quetype && ptr->Task_Append){\
		int type = ptr->get_Quetype(ptr,buf[0]);\
		ptr->Task_Append(ptr,type&0xff,(type>>8) &0xff,buf,buf[1]+2);\
	}\
}while(0)


static int meter_response(appitf_t *topuser,int subcmd,u8 slave_addr,u8 ndo,u8 res,int reslut)
{
	assert_param(topuser,FAIL);
	u8 pf[12] = { 0x52,0x08,0x80,0x06,0xe1,subcmd,slave_addr,ndo,res,reslut,0 };
	Task_merter(pf,topuser->Queue);
	return SUCCESS;
}


static int meter_res_atonce(appitf_t *topuser,int subcmd,u8 slave_addr,u8 ndo,u8 res,int reslut)
{
	assert_param(topuser,FAIL);
	u8 Ackbuf[24] = { 0x68,0,0,0,0,0,0,0x68,0x80,0x06,0x0e1,subcmd,slave_addr,ndo,res,reslut,0};
	memcpy(Ackbuf+1,topuser->param.CCUID,6);
	get_check_sum(Ackbuf,16);
	return topuser->opt_Itf->opt_send(topuser->opt_Itf,Ackbuf,18);
}

static int reado(Meter_t *this,u8 slave_addr)
{
	assert_param(this,FAIL);

	int res = -1;
	char slave[16] = {0};
	uint8_t *pslave = (uint8_t *)slave;
	appitf_t *topuser = this->topuser;

	MakeSlave(pslave,slave_addr,0x01,0x08);
	crc_low(pslave+6,pslave,6);
	topuser->Serial->serial_flush(topuser->Serial,CFG_COMDIDO);
	topuser->single->Display("meter reado  Send data:",slave,8);
	topuser->Serial->serial_send(topuser->Serial,CFG_COMDIDO,slave,8,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,CFG_COMDIDO,slave,6,2*SendTimeout);
	topuser->single->Display("meter reado  recv data:",&slave,6);
	res = crc_cmp_low(pslave+4,pslave,4);
	if(SUCCESS != res){
		debug(1,"Meter read do crc_check error!\n");
		return -1;
	}
	return pslave[3];
}
#endif

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
	return this->uart->uart_readall(this->uart,(char*)pf,100);
}

static int meter_update_sql(Meter_t *this,uint8_t addr,uint8_t num,int flag,uint32_t status)
{
	assert_param(this,FAIL);

	char buf[12] = {0};
	sprintf(buf,"addr=%d",addr);
	int32_t isexst = this->sql->sql_Isexist(CFG_tb_dido,buf);

	switch(flag){
		case UP_DI:
			if(TRUE == isexst)
				return this->sql->sql_update(CFG_tb_dido,\
					Asprintf("set di_stat=%d,ndi=%d where addr=%d",status,num,addr));
			else
				return this->sql->sql_insert(Asprintf("insert into "CFG_tb_dido\
					"(addr,ndi,di_stat) values(%d,%d,%d);",addr,num,status));
		case UP_DO:
			if(TRUE == isexst)
				return this->sql->sql_update(CFG_tb_dido,\
					Asprintf("set do_stat=%d,ndo=%d where addr=%d",status,num,addr));
			else
				return this->sql->sql_insert(Asprintf("insert into "CFG_tb_dido\
					"(addr,ndo,do_stat) values(%d,%d,%d);",addr,num,status));
		default:	return FAIL;
	}
}

static int meter_open(Meter_t *this,uint8_t addr, uint8_t num,uint32_t ndo)
{
	assert_param(this,FAIL);

	int res = SUCCESS;
	uint32_t status = this->meter_querydo(this, addr,num);
	if(status == ~0){
		debug(DEBUG_DIDO,"meter open query do error!\n");
		res = FAIL;
		goto out;
	}
	status |= ndo;
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,0x80,0x08,0xe1, 0x01, addr,
		0xff&(status>>24), 0xff&(status>>16),0xff&(status>>8),0xff&status,res,0,0,0
	};
	if(-1 == meter_sendpackage(this,addr,num,status) )
		*(ackbuf+17) = FAIL;
out:
	meter_resatonce(this->topuser, ackbuf,18);
	if(ackbuf[17] != FAIL){
		/* 更新DO状态 */
		meter_update_sql(this, addr, num,UP_DO, status);
		/* 更新DI状态 */
		sleep(1);	//继电器变化，di可能需要响应时间
		status = this->meter_querydi(this, addr,num);
		if(0xffffffff != status)
			meter_update_sql(this, addr, num,UP_DI, status);
	}
	return *(ackbuf+17);
}

static int meter_close(Meter_t *this,uint8_t addr, uint8_t num,uint32_t ndo)
{
	assert_param(this,FAIL);
	int32_t res = SUCCESS;
	uint32_t status = this->meter_querydo(this, addr,num);
	if( 0xffffffff == status ){
		debug(DEBUG_DIDO,"meter open query do error!\n");
		res = FAIL;
		goto out;
	}
	status &= ~ndo;
	uint8_t ackbuf[] = {
		0x68,0,0,0,0,0,0,0x68,0x80,0x08,0xe1, 0x02, addr,
		0xff&(status>>24), 0xff&(status>>16),0xff&(status>>8),0xff&status,res,0,0,0
	};
	if(-1 == meter_sendpackage(this,addr,num,status))
		*(ackbuf+17) = FAIL;
out:
	meter_resatonce(this->topuser, ackbuf,18);
	if(ackbuf[17] != FAIL){
		/* 更新DO状态 */
		meter_update_sql(this, addr, num,UP_DO, status);
		sleep(1);	//继电器变化，di可能需要响应时间
		/* 更新DI状态 */
		status = this->meter_querydi(this, addr,num);
		if(0xffffffff != status)
			meter_update_sql(this, addr, num,UP_DI, status);
	}
	return *(ackbuf+17);
}

static int meter_querydi(struct Meter_t *this,uint8_t addr,uint8_t ndi)
{
	assert_param(this,FAIL);
	/* 设备地址(addr), 指令(02), 起始地址(00,00), 查询数量(00,08) */
	char slave[16] = {addr,2,0,0,0,ndi,0};
	uint8_t *ps = (uint8_t*)slave;
	int chk = -1;
	crc_low(ps+6,ps,6);
	this->uart->uart_flush(this->uart);
	display_pack("meter readi  Send data",slave, 8);
	this->uart->uart_send(this->uart,slave,8,SendTimeout);
	int len = this->uart->uart_readall(this->uart, slave,100);
	display_pack("meter readi  recv data", slave, len);
	if(*(ps+1) != 0x02){
		debug(DEBUG_DIDO,"meter readi error!\n");
		return -1;
	}
	chk = crc_cmp_low(ps+len-2,ps,len-2);

	if(SUCCESS != chk){
		debug(DEBUG_DIDO,"Meter read di crc_check error!\n");
		return -1;
	}
	return *(ps+2) > 1 ? ps[3] | ps[4] << 8  : *(ps+3);
}

static int meter_querydo(struct Meter_t *this,uint8_t addr,uint8_t ndo)
{
	assert_param(this,FAIL);
	/* 设备地址, 指令, 起始地址(),查询数量(),CRC */
	char slave[16] = {addr,1,0,0,0,ndo,0};
	uint8_t *ps = (uint8_t *)slave;
	crc_low(ps+6,ps,6);
	this->uart->uart_flush(this->uart);
	display_pack("meter reado  Send data",slave, 8);
	this->uart->uart_send(this->uart,slave,8,SendTimeout);
	this->uart->uart_recv(this->uart, slave, 6,SendTimeout);
	display_pack("meter reado  recv data", slave, 6);
	if(*(ps+1) != 0x01){
		debug(DEBUG_DIDO,"meter reado error!\n");
		return -1;
	}
	int chk = crc_cmp_low(ps+4,ps,4);
	if(SUCCESS != chk){
		debug(1,"Meter read do crc_check error!\n");
		return -1;
	}
	return ps[3];
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
		meter_update_sql(this, addr, num,UP_DI, status);
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
		meter_update_sql(this, addr, num,UP_DO, status);
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

static int meter_query_dido(struct Meter_t*this, uint8_t cnt,uint8_t *pd)
{
	assert_param(this,FAIL);
	assert_param(pd,FAIL);
	debug(DEBUG_DIDO,"meter count is %d\n",cnt);
	uint8_t addr = 0, ret = SUCCESS;
	uint8_t ackbuf[128] = {0x68,0,0,0,0,0,0,0x68,0x80,cnt*9+4,0xe1,0x06,cnt,0};
	uint8_t *pk = ackbuf + 13;
	char condition[12] = {0};
	uint32_t stat[2] = {0};
	while(cnt--){
		*pk++ = addr = *pd++;
		sprintf(condition,"addr=%d",addr);
		if(this->sql->sql_Isexist(CFG_tb_dido,condition) ){
			this->sql->sql_select( Asprintf("select di_stat,do_stat from "\
				CFG_tb_dido" where addr=%d;",addr),(char*)stat,sizeof(stat),1,0);
			*pk++ = 0xff&(stat[0] >> 24); *pk++ = 0xff&(stat[0] >> 16);
			*pk++ = 0xff&(stat[0] >> 8);*pk++ = 0xff&(stat[0] >> 0);
			*pk++ = 0xff&(stat[1] >> 24); *pk++ = 0xff&(stat[1] >> 16);
			*pk++ = 0xff&(stat[1] >> 8);*pk++ = 0xff&(stat[1] >> 0);
		}else{
			*pk  += 8;
		}
	}
	*pk = ret;
	return meter_resatonce(this->topuser, ackbuf,ackbuf[9]+10);

}

static void meter_release(struct Meter_t **this)
{
	assert_param(this,;);
	assert_param(*this,;);
	DELETE((*this)->sql,sql_release);
	DELETE((*this)->uart,uart_relese);
	FREE(*this);
}

Meter_t *meter_init(Meter_t *this,struct appitf_t *topuser)
{
	assert_param(topuser,NULL);
	Meter_t*pth = this;
	if(!pth){
		this = malloc(sizeof(Meter_t));
		if(!this) return NULL;
	}
	bzero(this,sizeof(Meter_t));
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

	return this;
out:

	if(!this->sql) DELETE(this->sql,sql_release);
	if(!this->uart) DELETE( this->uart,uart_relese);
	if(!pth) FREE(this);
	return NULL;
}
