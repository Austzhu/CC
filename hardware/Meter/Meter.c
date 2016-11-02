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
	assert_param(topuser,NULL,FAIL);
	u8 pf[32] = {0};

	pf[0] = 0x52; pf[1] = 0x08;pf[2] = 0x80;
	pf[3] = 0x06;pf[4] = 0xE1;pf[5] = subcmd;
	pf[6] = slave_addr;pf[7] = ndo;pf[8] = res;
	pf[9] = reslut;
	Task_merter(pf,topuser->Queue);
	return SUCCESS;
}

static int meter_res_atonce(appitf_t *topuser,int subcmd,u8 slave_addr,u8 ndo,u8 res,int reslut)
{
	assert_param(topuser,NULL,FAIL);

	u8 Ackbuf[32] = {0};
	Ackbuf[0] = Ackbuf[7] = 0x68;
	memcpy(Ackbuf+1,topuser->param.CCUID,6);
	Ackbuf[8] = 0x80; Ackbuf[9] = 0x06;
	Ackbuf[10] = 0xe1; Ackbuf[11] = subcmd;
	Ackbuf[12] = slave_addr; Ackbuf[13] = ndo;
	Ackbuf[14] = res; Ackbuf[15] = reslut;
	topuser->ethernet->ether_packagecheck(Ackbuf,16);
	return topuser->ethernet->ether_send(topuser->ethernet,Ackbuf,18);
}

static int reado(Meter_t *this,u8 slave_addr)
{
	assert_param(this,NULL,FAIL);

	int res = -1;
	char slave[16] = {0};
	u8 *pslave = (u8*)slave;
	appitf_t *topuser = this->topuser;

	memset(slave,0,sizeof(slave));
	MakeSlave(pslave,slave_addr,0x01,0x08);
	topuser->Crc16_HL(crc_get,pslave+6,pslave,6);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter reado  Send data:",slave,8);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,slave,8,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,slave,6,2*SendTimeout);
	topuser->single->Display("meter reado  recv data:",&slave,6);
	res = topuser->Crc16_HL(crc_check,pslave+4,pslave,4);

	if(SUCCESS != res){
		debug(1,"Meter read do crc_check error!\n");
		return -1;
	}
	return pslave[3];
}


static int meter_sendpackage(Meter_t *this,u8 slave_addr,void *Message,u8 setval)
{
	assert_param(this,NULL,FAIL);
	assert_param(Message,NULL,FAIL);

	/**
	 *  package format:
	 *  slave addr(1byte),ctrl(1byte),start addr(2byte),number of relay(2byte),
	 *  size of next control(1byte),the value for relay the bit is 1 open else close,CRC16(2byte)
	 */
	u8 *pf = (u8*)Message;
	appitf_t *topuser = this->topuser;

	*pf = slave_addr;		//slave addr
	*(pf+1) = 0x0f;			//CTRL
	*(pf+2) = *(pf+3) = 0;	//start of relay addr
	*(pf+4) = 0; *(pf+5) = 8;	//the number f relay
	*(pf+6) = 1;				//size of next value
	*(pf+7) = setval;
	topuser->Crc16_HL(crc_get,pf+8,pf,8);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter open all Send data:",pf,10);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,(s8*)pf,10,SendTimeout);
	return topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,(s8*)pf,8,2*SendTimeout);

}

static int meter_open(Meter_t *this,u8 slave_addr, u8 ndo)
{
	assert_param(this,NULL,FAIL);

	char slave[16] ={0};
	int res = reado(this,slave_addr);
	if(-1 == res)  {
		meter_res_atonce(this->topuser,sub_open,slave_addr,ndo,0,FAIL);
		return FAIL;
	}
	meter_sendpackage(this,slave_addr,slave, res|ndo);

	return  this->meter_reado(this,slave_addr,ndo,sub_open);
}

static int meter_close(Meter_t *this,u8 slave_addr, u8 ndo)
{
	assert_param(this,NULL,FAIL);

	char slave[16] = {0};
	int res = reado(this,slave_addr);
	if(-1 == res)  {
		meter_res_atonce(this->topuser,sub_close,slave_addr,ndo,0,FAIL);
		return FAIL;
	}
	meter_sendpackage(this,slave_addr,slave, ~ndo&res);
	return this->meter_reado(this,slave_addr,ndo,sub_close);
}

static int meter_readi(Meter_t *this,u8 slave_addr,u8 ndo,subcmd_t subcmd)
{
	assert_param(this,NULL,FAIL);

	int res = -1;
	char slave[16] = {0};
	u8 *pslave = (u8*)slave;
	appitf_t *topuser = this->topuser;

	memset(slave,0,sizeof(slave));
	MakeSlave(pslave,slave_addr,0x02,0x08);
	topuser->Crc16_HL(crc_get,pslave+6,pslave,6);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter readi  Send data:",slave,8);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,slave,8,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,slave,6,2*SendTimeout);
	topuser->single->Display("meter readi  recv data:",slave,6);
	res = topuser->Crc16_HL(crc_check,pslave+4,pslave,4);

	if(SUCCESS != res){
		debug(1,"Meter read di crc_check error!\n");
		meter_response(topuser,subcmd,slave_addr,ndo,0,FAIL);
		return FAIL;
	}
	return meter_response(topuser,subcmd,slave_addr,ndo,pslave[3],SUCCESS);
}

static int meter_reado(Meter_t *this,u8 slave_addr,u8 ndo,subcmd_t subcmd)
{
	assert_param(this,NULL,FAIL);
	appitf_t *topuser = this->topuser;
	int res = reado(this,slave_addr);
	if(-1 == res)
		return meter_res_atonce(topuser,subcmd,slave_addr,ndo,res,FAIL);
	return meter_res_atonce(topuser,subcmd,slave_addr,ndo,res,SUCCESS);
}

static int meter_flashopen(struct Meter_t *this,u8 slave_addr,u8 ndo,int ms)
{
	assert_param(this,NULL,FAIL);

	char slave[16] ={0};
	appitf_t *topuser = this->topuser;
	int res = reado(this,slave_addr);
	if(-1 == res)  {
		meter_res_atonce(topuser,sub_flash,slave_addr,ndo,0,FAIL);
		return FAIL;
	}
	/*  open the relay */
	meter_sendpackage(this,slave_addr,slave, res|ndo);
	/* response to pc */
	meter_res_atonce(topuser,sub_flash,slave_addr,ndo,res|ndo,SUCCESS);
	topuser->msleep(ms*100);
	/* close the relay */
	return meter_sendpackage(this,slave_addr,slave, res&~ndo);
}

static void meter_release(struct Meter_t **this)
{
	memset(*this,0,sizeof(Meter_t));
	free(*this);
	*this = NULL;
}

Meter_t *meter_init(struct appitf_t *topuser)
{
	Meter_t *this = malloc(sizeof(Meter_t));
	if(!this) return NULL;
	memset(this,0,sizeof(Meter_t));

	this->topuser = topuser;
	this->meter_open = meter_open;
	this->meter_close = meter_close;
	this->meter_readi = meter_readi;
	this->meter_reado = meter_reado;
	this->meter_flashopen = meter_flashopen;
	this->meter_release = meter_release;


	if( !this->topuser || !this->meter_open || !this->meter_close ||\
		!this->meter_readi || !this->meter_reado || \
		!this->meter_flashopen || !this->meter_release ){
		free(this);
		return NULL;
	}
	return this;
}
