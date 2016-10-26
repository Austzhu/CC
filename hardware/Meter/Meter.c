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
	appitf_t *topuser = this->parent;

	memset(&slave,0,sizeof(slave_t));
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

static int meter_open(Meter_t *this,u8 slave_addr, u8 ndo)
{
	assert_param(this,NULL,FAIL);

	char slave[16] ={0};
	char recvbuf[12] = {0};
	u8 *pslave = (u8*)slave;
	appitf_t *topuser = this->parent;

	int res = reado(this,slave_addr);
	if(-1 == res)  {
		meter_res_atonce(topuser,sub_open,slave_addr,ndo,0,FAIL);
		return FAIL;
	}

	MakeSlaveOpenAll(slave,slave_addr,(res|ndo));
	topuser->Crc16_HL(crc_get,pslave+8,pslave,8);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter open all Send data:",slave,10);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,slave,10,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,recvbuf,8,2*SendTimeout);

	return  this->meter_reado(this,slave_addr,ndo,sub_open);
}

static int meter_close(Meter_t *this,u8 slave_addr, u8 ndo)
{
	assert_param(this,NULL,FAIL);

	char slave[16] = {0};
	char recvbuf[12] = {0};
	u8 *pslave = (u8*)slave;
	appitf_t *topuser = this->parent;
	int res = reado(this,slave_addr);
	if(-1 == res)  {
		meter_res_atonce(topuser,sub_close,slave_addr,ndo,0,FAIL);
		return FAIL;
	}
	MakeSlaveOpenAll(slave,slave_addr,(~ndo&res));
	topuser->Crc16_HL(crc_get,pslave+8,pslave,8);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter close all Send data:",slave,10);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,slave,10,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,recvbuf,8,2*SendTimeout);

	return this->meter_reado(this,slave_addr,ndo,sub_close);
}

static int meter_readi(Meter_t *this,u8 slave_addr,u8 ndo,subcmd_t subcmd)
{
	assert_param(this,NULL,FAIL);

	int res = -1;
	char slave[16] = {0};
	u8 *pslave = (u8*)slave;
	appitf_t *topuser = this->parent;

	memset(slave,0,sizeof(slave_t));
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
	appitf_t *topuser = this->parent;
	int res = reado(this,slave_addr);
	if(-1 == res)
		return meter_res_atonce(topuser,subcmd,slave_addr,ndo,res,FAIL);
	return meter_res_atonce(topuser,subcmd,slave_addr,ndo,res,SUCCESS);
}

static int meter_flashopen(struct Meter_t *this,u8 slave_addr,u8 ndo,int ms)
{
	assert_param(this,NULL,FAIL);
	char slave[16] ={0};
	char recvbuf[12] = {0};
	u8 *pslave = (u8*)slave;
	appitf_t *topuser = this->parent;

	int res = reado(this,slave_addr);
	if(-1 == res)  {
		meter_res_atonce(topuser,sub_flash,slave_addr,ndo,0,FAIL);
		return FAIL;
	}
	/*  open the relay */
	MakeSlaveOpenAll(slave,slave_addr,(res|ndo));
	topuser->Crc16_HL(crc_get,pslave+8,pslave,8);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter open all Send data:",slave,10);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,slave,10,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,recvbuf,8,2*SendTimeout);
	/* response to pc */
	meter_res_atonce(topuser,sub_flash,slave_addr,ndo,res|ndo,FAIL);
	topuser->msleep(ms*100);
	/* close the relay */
	MakeSlaveOpenAll(slave,slave_addr,(res&~ndo));
	topuser->Crc16_HL(crc_get,pslave+8,pslave,8);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter open all Send data:",slave,10);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,slave,10,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,recvbuf,8,2*SendTimeout);
	return SUCCESS;
}

static int meter_init(Meter_t *this,void *parent)
{
	assert_param(this,NULL,FAIL);

	this->parent = parent;
	return SUCCESS;
}


Meter_t g_meter ={
	.meter_open = meter_open,
	.meter_close = meter_close,
	.meter_readi = meter_readi,
	.meter_reado = meter_reado,
	.meter_init = meter_init,
	.meter_flashopen = meter_flashopen,
};
