#include "Callback_function.h"
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


 s32 CallBack_Response(Node_t *node,void *parent)
{
	assert_param(node,NULL,FAIL);
	assert_param(parent,NULL,FAIL);

	appitf_t *_parent = (appitf_t*)parent;
	u8 *Sendbuf = malloc(node->package[1] + 12);
	if(!Sendbuf) return FAIL;

	Sendbuf[0] = Sendbuf[7] = 0x68;
	memcpy(Sendbuf+1,_parent->CCUID,6);
	memcpy(Sendbuf+8,node->package + 2,node->package[1]);
	if(_parent->ItfWay == ether_net /* 使用网络连接才能调用这接口 */ &&\
		_parent->ethernet 		/* 是否定义了网络连接的类 */ &&\
		_parent->ethernet->ether_packagecheck != NULL){
		_parent->ethernet->ether_packagecheck(Sendbuf,node->package[1]+8);
		_parent->ethernet->ether_send(_parent->ethernet,Sendbuf,node->package[1]+10);
	}
	#ifdef DisplayResPackage
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
	assert_param(node,NULL,FAIL);
	assert_param(parent,NULL,FAIL);

	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	int res = -1;
	switch(package->data[0]){
		case 0x01:	//reset
			res = Reset2DoFunctions();
			MakeShortResponse(AckBuffer,03,0xA2,0x01,res);
			Append2Queue(AckBuffer,_parent->Queue);
			if(res == SUCCESS)
				 debug(DEBUG_reset,"^^^^^reset success.....\n");
			else debug(DEBUG_reset,"^^^^^reset fail.....\n");
			break;
		case 0x02:		//reboot
			MakeShortResponse(AckBuffer,03,0xA2,0x02,SUCCESS);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^reboot success.....\n");
			system("reboot");
			break;
		case 0x03:	//time tick
			res = time_tick(&package->data[1]);
			MakeShortResponse(AckBuffer,03,0xA2,0x03,res);
			Append2Queue(AckBuffer,_parent->Queue);
			if(res == SUCCESS)
				 debug(DEBUG_reset,"Time tick success!\n");
			else debug(DEBUG_reset,"Time tick error!\n");
			break;
		case 0x04:	//Query time
			res = Query_time(AckBuffer,sizeof(AckBuffer));
			if(SUCCESS == res){
				Append2Queue(AckBuffer,_parent->Queue);
				debug(DEBUG_reset,"Query time success!\n");
			}else debug(DEBUG_reset,"Query time error!\n");
			break;
		case 0x05: 	//Query time
			res = CC_Inquire_Version(AckBuffer,sizeof(AckBuffer));
			if(SUCCESS == res){
				Append2Queue(AckBuffer,_parent->Queue);
				debug(DEBUG_reset,"CC Inquire Version success!\n");
			}else debug(DEBUG_reset,"CC Inquire Version error!\n");
			break;
		case 0x70:	//poweroff
			MakeShortResponse(AckBuffer,03,0xA2,0x70,SUCCESS);
			Append2Queue(AckBuffer,_parent->Queue);
			debug(DEBUG_reset,"^^^^^poweroff success.....\n");
			system("poweroff");
			break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_Config(Node_t *node,void *parent)
{
	assert_param(node,NULL,FAIL);
	assert_param(parent,NULL,FAIL);
	PureCmdArry_t *package = (PureCmdArry_t*)node->package;
	appitf_t *_parent = (appitf_t*)parent;
	u8 AckBuffer[64] = {0};
	int res = -1;

	switch(package->data[0]){
		case 0x01:break;
		case 0x05:
			res = SingleConfig(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x05,res);
			Append2Queue(AckBuffer,_parent->Queue);
			if(SUCCESS == res){
				debug(DEBUG_reset,"Single config success!\n");
			}else debug(DEBUG_reset,"Single config error!\n");
			break;
		case 0x06:
			res = CoordiConfig(package->data +1,_parent);
			MakeShortResponse(AckBuffer,03,0xA3,0x05,res);
			Append2Queue(AckBuffer,_parent->Queue);
			if(SUCCESS == res){
				debug(DEBUG_reset,"Coordinate config success!\n");
			}else debug(DEBUG_reset,"Coordinate config error!\n");
			break;
		case 0x07:break;
		case 0x08:break;
		case 0x09:break;
		default:break;
	}
	return SUCCESS;
}

s32 CallBack_answer(Node_t *node,void*parent)
{
	assert_param(node,NULL,FAIL);
	assert_param(parent,NULL,FAIL);

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
