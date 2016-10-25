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
	*pf++ = _addr;*pf++ = 0x0f;*pf++ = 0;*pf++ = 0;\
	*pf++ = 0; *pf++ = 8;*pf++ = 1; *pf++ = oc;\
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

static int meter_open(Meter_t *this,u8 slavr_addr, u8 ndo)
{
	assert_param(this,NULL,FAIL);

	slave_t slave;
	u8 *pslave = (u8*)&slave;
	u8 recvbuf[12] = {0};
	appitf_t *topuser = this->parent;
	u8 _ndo = ndo;

	if(ndo == 0xff){		//open all
		MakeSlaveOpenAll(pslave,slavr_addr,0xff);
		topuser->Crc16_HL(crc_get,(u8*)&(slave.bak),&(slave.addr),8);
		topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
		topuser->single->Display("meter open all Send data:",&slave,sizeof(slave_t));
		topuser->Serial->serial_send(topuser->Serial,COM_DIDO,(s8*)&slave,sizeof(slave_t),SendTimeout);
		topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,(s8*)recvbuf,8,2*SendTimeout);
	}else{
		MakeSlave(pslave,slavr_addr,0x05,(0xff<<8));
		for(int i=0;i<8;++i){
			slave.Io_addr = i<<8;
			if(ndo&0x01){
				ndo >>= 1;
				topuser->Crc16_HL(crc_get,(u8*)&slave.crc,&slave.addr,6);
				topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
				topuser->single->Display("meter open all Send data:",&slave,8);
				topuser->Serial->serial_send(topuser->Serial,COM_DIDO,(s8*)&slave,8,SendTimeout);
				topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,(s8*)recvbuf,8,2*SendTimeout);
			}	//end of if(ndo&0x01)
		}	//end of for(int i=0;i<8;++i){
	}	//end of if(ndo == 0xff)
	return this->meter_reado(this,slavr_addr,_ndo);
}

static int meter_close(Meter_t *this,u8 slavr_addr, u8 ndo)
{
	assert_param(this,NULL,FAIL);

	slave_t slave;
	u8 *pslave = (u8*)&slave;
	s8 recvbuf[12] = {0};
	appitf_t *topuser = this->parent;
	u8 _ndo = ndo;
	if(ndo == 0xff){		//open all
		MakeSlaveOpenAll(pslave,slavr_addr,0);
		topuser->Crc16_HL(crc_get,(u8*)&slave.bak,&slave.addr,8);
		topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
		topuser->single->Display("meter close all Send data:",&slave,sizeof(slave_t));
		topuser->Serial->serial_send(topuser->Serial,COM_DIDO,(s8*)&slave,sizeof(slave_t),SendTimeout);
		topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,(s8*)recvbuf,8,2*SendTimeout);
	}else{
		MakeSlave(pslave,slavr_addr,0x05,0);
		for(int i=0;i<8;++i){
			slave.Io_addr = i<<8;
			if(ndo&0x01){
				ndo >>= 1;
				topuser->Crc16_HL(crc_get,(u8*)&slave.crc,&slave.addr,6);
				topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
				topuser->single->Display("meter close  Send data:",&slave,8);
				topuser->Serial->serial_send(topuser->Serial,COM_DIDO,(s8*)&slave,8,SendTimeout);
				topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,recvbuf,8,2*SendTimeout);
			}	//end of if(ndo&0x01)
		}	//end of for(int i=0;i<8;++i){
	}	//end of if(ndo == 0xff)
	return this->meter_reado(this,slavr_addr,_ndo);
}

static int meter_readi(Meter_t *this,u8 slavr_addr,u8 ndo)
{
	assert_param(this,NULL,FAIL);
	return SUCCESS;
}

static int meter_reado(Meter_t *this,u8 slavr_addr,u8 ndo)
{
	assert_param(this,NULL,FAIL);

	int res = -1;
	slave_t slave;
	u8 *pslave = (u8*)&slave;
	appitf_t *topuser = this->parent;
	memset(&slave,0,sizeof(slave_t));
	MakeSlave(pslave,slavr_addr,0x01,0x08);
	topuser->Crc16_HL(crc_get,(u8*)&slave.crc,&slave.addr,6);
	topuser->Serial->serial_flush(topuser->Serial,COM_DIDO);
	topuser->single->Display("meter reado  Send data:",&slave,8);
	topuser->Serial->serial_send(topuser->Serial,COM_DIDO,(s8*)&slave,8,SendTimeout);
	topuser->Serial->serial_recv(topuser->Serial,COM_DIDO,(s8*)&slave.addr,6,2*SendTimeout);
	topuser->single->Display("meter reado  recv data:",&slave,6);
	res = topuser->Crc16_HL(crc_check,(u8*)&slave.cmd,&slave.addr,4);

	if(SUCCESS != res){
		debug(1,"Meter read do crc_check error!\n");
		meter_response(topuser,sub_reado,slavr_addr,ndo,0,FAIL);
		return FAIL;
	}
	pslave = (u8*)&slave;
	return meter_response(topuser,sub_reado,slavr_addr,ndo,pslave[4],SUCCESS);
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
};
