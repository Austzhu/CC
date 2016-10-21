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

#define SendTimeout 1000000		//1s
#define RecvTimeout  20

#define MakeSinglePack(ptr,ctrl,cmd,Addr,_data) do{\
	(ptr)->Header = 0xFF;(ptr)->Ctrl = 0xff&(ctrl); (ptr)->Group_Addr = 0xff&((Addr)>>24);\
	(ptr)->Coordi_Addr = 0xff&((Addr)>>16);(ptr)->Single_Addr[0] = 0xff&((Addr)>>8);\
	(ptr)->Single_Addr[1] = 0xff&(Addr);(ptr)->Cmd[0] = 0xff&((cmd)>>8);\
	(ptr)->Cmd[1] = 0xff&(cmd);(ptr)->Data[0] = 0xff&((_data)>>8);(ptr)->Data[1] = 0xff&(_data);\
}while(0)







static int sin_Config(Single_t *this,sin_cfg_t cmd,void *package)
{
	assert_param(this,NULL,FAIL);
	assert_param(package,NULL,FAIL);

	light_t single;
	TableSingle_t   *sp = package;
	TableCoordi_t *cp = package;
	appitf_t *topuser = this->parent;
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
	appitf_t *topuser = this->parent;
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

static int sin_Init(Single_t *this,void *parent)
{
	assert_param(this,NULL,FAIL);
	assert_param(parent,NULL,FAIL);

	this->parent = parent;
	this->sin_config = sin_Config;
	if(this->sin_config)
		return SUCCESS;
	return FAIL;
}

Single_t g_single = {
	.sin_init = sin_Init,
	.Display = Display_package,
	.sin_RecvPackage = sin_RecvPackage,
};

