#include "Callback_function.h"
#include "process.h"
#include "Interface.h"

#define MakeShortResponse(pbuf,len,cmd,subcmd,result) do{\
	pbuf[0] = 0x52;pbuf[1] = len+2;pbuf[2] = 0x80;pbuf[3] = len;\
	pbuf[4] = cmd;pbuf[5] = subcmd;pbuf[6] = result;\
}while(0)

#define Append2Queue(pbuf,pthis) do{\
	if(pthis->get_Quetype && pthis->Task_Append){\
		int type = pthis->get_Quetype(pthis,pbuf[0]);\
		pthis->Task_Append(pthis,type&0xff,(type>>8) &0xff,pbuf,pbuf[1]+2);\
	}\
}while(0)

#define MakeSingleResponse(pbuf,subcmd,_Addr,result) do{\
	pbuf[0] = 0x52;pbuf[1] = 0x08;pbuf[2] = 0x80;pbuf[3] = 0x06;\
	pbuf[4] = 0x01;pbuf[5] = subcmd;pbuf[6] = 0xff&(_Addr>>16);\
	pbuf[7] = 0xff&(_Addr>>8);pbuf[8] = 0xff&_Addr;pbuf[9] = result;\
}while(0)

#define Make2Response(pbuf,cmd,subcmd) do{\
	pbuf[0] = 0x52;pbuf[1] = 0x03;pbuf[2] = cmd;\
	pbuf[3] = 0x01;pbuf[4] = (subcmd);\
}while(0)

#define MakeUpdateResponse(pf,_cmd,_addr,_len,_res)  do{\
	pf[0] = 0x52;  pf[1] = _len+2;  pf[2] = 0x80;  pf[3] = _len;  \
	pf[4] = 0xff&((_cmd)>>8);  pf[5] = 0xff&(_cmd);  pf[6] = 0xff&((_addr)>>16);\
	if(_len > 4){pf[7] = 0xff&((_addr)>>8);  pf[8] = 0xff&(_addr);pf[9] = _res;}\
	else pf[7] = _res;\
}while(0)



s32 CallBack_Response(Node_t *node,void *top)
{
	assert_param(node,FAIL);
	assert_param(top,FAIL);

	appitf_t *topuser = (appitf_t*)top;
	u8 *Sendbuf = malloc(node->package[1] + 12);
	if(!Sendbuf) return FAIL;

	Sendbuf[0] = Sendbuf[7] = 0x68;
	memcpy(Sendbuf+1,topuser->param.CCUID,6);
	memcpy(Sendbuf+8,node->package + 2,node->package[1]);
	/* 获取校验码 */
	get_check_sum(Sendbuf,node->package[1]+8);
	if(topuser->opt_Itf && topuser->opt_Itf->opt_send)	/* check point is initialized */
		topuser->opt_Itf->opt_send(topuser->opt_Itf,Sendbuf,node->package[1]+10);

	#ifdef CFG_showPackage
		printf("Response package:");
		for(int i=0,size=node->package[1]+10;i<size;++i)
			printf("%02X ",*(Sendbuf+i));
		printf("\n");
	#endif
	free(Sendbuf);
	return SUCCESS;
}

s32 CallBack_Reset(Node_t *node,void *parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);

	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	int res = -1;
	switch(package->data[0]){
		case 0x01:	//reset
			res = opt_reset();
			MakeShortResponse(AckBuffer,03,0xA2,0x01,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^reset %s^^^^^\n",res == SUCCESS ? "success":"error");
			break;
		case 0x02:	//reboot
			MakeShortResponse(AckBuffer,03,0xA2,0x02,SUCCESS);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^reboot success.....\n");
			sync();
			IoctlBuzzer(100,0);
			reboot(RB_AUTOBOOT);
			break;
		case 0x03:	//time tick
			res = opt_tmtick(&package->data[1]);
			MakeShortResponse(AckBuffer,03,0xA2,0x03,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^Time tick %s^^^^^\n",res == SUCCESS ? "success":"error");
			break;
		case 0x04:	//Query time
			res = opt_tmquery(AckBuffer,sizeof(AckBuffer));
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^Query time %s^^^^^\n",res == SUCCESS ? "success":"error");
			break;
		case 0x05: 	//Query time
			res = opt_version(AckBuffer,sizeof(AckBuffer));
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^Inquire Version %s^^^^^\n",res == SUCCESS ? "success":"error");
			break;
		case 0x70:	//poweroff
			MakeShortResponse(AckBuffer,03,0xA2,0x70,SUCCESS);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^poweroff success.....\n");
			sync();
			IoctlBuzzer(100,0);
			sleep(2);
			reboot(RB_POWER_OFF);
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_Config(Node_t *node,void *parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);
	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	int res = -1;

	switch(package->data[0]){
		case 0x01:break;
		case 0x02:
			res = Config_tunnel(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x02,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Tunnel config %s\n",res==SUCCESS? "success!":"error!");
			break;
		case 0x04:	//模式切换
			res = _parent->TopUser_setMode(_parent,package->data[1]);
			MakeShortResponse(AckBuffer,03,0xA3,0x04,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_config,"config control method %d %s\n",\
				_parent->param.ControlMethod, res==SUCCESS? "success!":"error!");
			break;
		case 0x05:	//单灯配置
			res = Config_Single(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x05,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Single config %s\n",res==SUCCESS? "success!":"error!");
			break;
		case 0x06:	//协调器配置
			res = Config_Coordi(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x06,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Coordinate config %s\n",res==SUCCESS? "success!":"error!");
			break;
		case 0x07:	//时序任务配置
			res = Config_task(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x07,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"config task %s\n",res==SUCCESS? "success!":"error!");
			break;
		case 0x08:	//时序任务操作明细配置
			res = Config_tasklist(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x08,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"config task list %s\n",res==SUCCESS? "success!":"error!");
			break;
		case 0x09:
			res = Config_delete(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x09,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"delete sql table %s\n",res==SUCCESS? "success!":"error!");
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_answer(Node_t *node,void*parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);

	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	switch(package->data[0]){
		case 0xA1:
			if(package->data[1] == 0x01){
				if(package->data[2] == 0){
					debug(1,"^^^logon ok^^^\n");
					_parent->Connect_status = Connect_ok;
				}else{
					_parent->Connect_status = Connect_error;
				}
			}else if(package->data[1] == 0x02){
				if(package->data[2] == 0){
					debug(1,"^^^HeartBeat ok!^^^\n");
					_parent->HeartBeat_status = HeartBeat_ok;
				}else{
					_parent->HeartBeat_status = HeartBeat_error;
				}
			}
			return SUCCESS;
		default:break;
	}
	printf("node data:");
	for(int i=0,size=package->len + 2;i< size;++i){
		printf("%02x ",node->package[i]);
	}printf("\n");
	return SUCCESS;
}

s32 CallBack_single(Node_t *node,void*parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);

	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	u32 Addr = (node->package[3] <<16) | (node->package[4] << 8) | node->package[5];
	u32 light = 0; int res = -1;

	switch(package->data[0]){
		case 0x42:
			light = node->package[6];
			#ifdef CFG_PWM_N
				light = (light<CFG_PWMAX ? CFG_PWMAX-light : 0)<< 8;
			#else
				light = (light>CFG_PWMAX ? CFG_PWMAX : light) << 8;
			#endif
			res = _parent->single->sin_open(_parent->single,cmd_single,Addr,light);
			MakeSingleResponse(AckBuffer,0x42,Addr,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"single open %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x43:
			res = _parent->single->sin_close(_parent->single,cmd_single,Addr);
			MakeSingleResponse(AckBuffer,0x43,Addr,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"single close %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x45:
			res = _parent->single->sin_Querystatus(_parent->single,cmd_single,Addr);
			debug(DEBUG_reset,"single Query status %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x46:
			res = _parent->single->sin_Queryelectric(_parent->single,cmd_single,Addr);
			debug(DEBUG_reset,"single Query electric %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x47:
			light = node->package[6];
			#ifdef CFG_PWM_N
				light = (light<CFG_PWMAX ? CFG_PWMAX- light : 0)<<8;
			#else
				light = (light>CFG_PWMAX ? CFG_PWMAX : light)<<8;
			#endif
			res = _parent->single->sin_open(_parent->single,cmd_single,Addr,light);
			MakeSingleResponse(AckBuffer,0x47,Addr,res);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"single light %s!\n",SUCCESS == res ? "ok":"error");
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_group(Node_t *node,void*parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);
	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	u32 Addr = node->package[3]<<24;;
	u32 light = 0; int res = -1;

	switch(package->data[0]){
		case 0x42:
			light = node->package[4];
			#ifdef CFG_PWM_N
				light = (light<CFG_PWMAX ? CFG_PWMAX-light : 0)<<8;
			#else
				light = (light>CFG_PWMAX ? CFG_PWMAX : light)<<8;
			#endif
			res = _parent->single->sin_open(_parent->single,cmd_group,Addr,light);
			/* The second response to PC */
			Make2Response(AckBuffer,02,SUCCESS == res ? 0X42:0X43);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Group open %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x43:
			res = _parent->single->sin_close(_parent->single,cmd_group,Addr);
			/* The second response to PC */
			Make2Response(AckBuffer,02,SUCCESS == res ? 0X43:0X42);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Group close %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x45:
			res = _parent->single->sin_Querystatus(_parent->single,cmd_group,Addr);
			debug(DEBUG_reset,"Group Query status %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x46:
			res = _parent->single->sin_Queryelectric(_parent->single,cmd_group,Addr);
			debug(DEBUG_reset,"Group Query electric %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x47:
			light = node->package[4];
			#ifdef CFG_PWM_N
				light = (light<CFG_PWMAX ? CFG_PWMAX-light : 0)<<8;
			#else
				light = (light>CFG_PWMAX ? CFG_PWMAX : light)<<8;
			#endif
			res = _parent->single->sin_open(_parent->single,cmd_grouplight,Addr,light);
			/* The second response to PC */
			Make2Response(AckBuffer,02,SUCCESS == res ? 0X47:0X43);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Group light %s!\n",SUCCESS == res ? "ok":"error");
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_broadcast(Node_t *node,void*parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);
	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	u32 light;
	int res = -1;

	switch(package->data[0]){
		case 0x42:
			light = node->package[3];
			#ifdef CFG_PWM_N
				light = (light<CFG_PWMAX ? CFG_PWMAX-light : 0)<<8;
			#else
				light = (light>CFG_PWMAX ? CFG_PWMAX : light)<<8;
			#endif
			res = _parent->single->sin_open(_parent->single,cmd_broadcast,0,light);
			/* The second response to PC */
			Make2Response(AckBuffer,03,SUCCESS == res ? 0X42:0X43);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Broadcast open %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x43:
			res = _parent->single->sin_close(_parent->single,cmd_broadcast,0);
			/* The second response to PC */
			Make2Response(AckBuffer,03,SUCCESS == res ? 0X43:0X42);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Broadcast close %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x45:
			res = _parent->single->sin_Querystatus(_parent->single,cmd_broadcast,0);
			debug(DEBUG_reset,"Broadcast Query status %s!\n",SUCCESS == res ? "ok":"error");
			_parent->warn->warn_verdict(_parent->warn);
			break;
		case 0x46:
			res = _parent->single->sin_Queryelectric(_parent->single,cmd_broadcast,0);
			debug(DEBUG_reset,"Broadcast Query electric %s!\n",SUCCESS == res ? "ok":"error");
			_parent->warn->warn_verdict(_parent->warn);
			break;
		case 0x47:
			light = node->package[3];
			#ifdef CFG_PWM_N
				light = (light<CFG_PWMAX ? CFG_PWMAX-light : 0)<<8;
			#else
				light = (light>CFG_PWMAX ? CFG_PWMAX : light)<<8;
			#endif
			res = _parent->single->sin_open(_parent->single,cmd_broadlight,0,light);
			/* The second response to PC */
			Make2Response(AckBuffer,03,SUCCESS == res ? 0X47:0X43);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"Broadcast light %s!\n",SUCCESS == res ? "ok":"error");
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_meter(Node_t *node,void*parent)
{
	assert_param(node,FAIL);
	assert_param(parent,FAIL);
	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *top = (appitf_t*)parent;
	uint8_t *pd = package->data;
	int32_t res = -1;
	switch(*pd){
		case 0x01:
			res = top->meter->meter_open(top->meter,*(pd+1),*(pd+2),PARAM_INT(pd+3));
			debug(DEBUG_reset,"meter open %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x02:
			res = top->meter->meter_close(top->meter,*(pd+1),*(pd+2),PARAM_INT(pd+3));
			debug(DEBUG_reset,"meter close %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x03:
			res = top->meter->meter_reado(top->meter,*(pd+1),*(pd+2));
			debug(DEBUG_reset,"meter read do %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x04:
			res = top->meter->meter_readi(top->meter,*(pd+1),*(pd+2));
			debug(DEBUG_reset,"meter read di %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x05:
			res = top->meter->meter_flashopen(top->meter,*(pd+1),*(pd+2),PARAM_INT(pd+3),*(pd+7));
			debug(DEBUG_reset,"meter flash open %s!\n",SUCCESS == res ? "ok":"error");
			break;
		case 0x06:
			res = top->meter->meter_query_dido(top->meter,*(pd+1),pd+2);
			debug(DEBUG_reset,"meter query dido %s!\n",SUCCESS == res ? "ok":"error");
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_Update(Node_t *node,void *top)
{
	assert_param(node,FAIL);
	assert_param(top,FAIL);
	u8 AckBuffer[64] = {0};
	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *topuser = (appitf_t*)top;
	int addr = package->data[1] << 16,  res = -1;
	 switch(package->data[0]){
	 	case 0x06:
	 		addr |= (package->data[2]<<8) | (package->data[3]);
	 		res = topuser->single->sin_update(topuser->single,addr);
	 		MakeUpdateResponse(AckBuffer,((0xA4<<8)|0x06),addr,6,res);
	 		Append2Queue(AckBuffer,topuser->Queue);
	 		debug(DEBUG_reset,"update %04x %s!\n",0xffff&addr,SUCCESS == res ? "ok":"error");
	 		break;
	 	case 0x05:
	 		res = topuser->single->sin_update(topuser->single,addr);
	 		MakeUpdateResponse(AckBuffer,((0xA4<<8)|0x05),addr,4,res);
	 		Append2Queue(AckBuffer,topuser->Queue);
	 		debug(DEBUG_reset,"update %02x %s!\n",0xff&(addr>>16),SUCCESS == res ? "ok":"error");
	 		break;
	 	default:break;
	 }
	return SUCCESS;
}
