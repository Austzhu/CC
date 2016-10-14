/******************************************************************
** 文件名:
** Copyright (c) 1998-2099 *********公司技术开发部
** 创建人:
** 日　期:
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:
*******************************************************************/
#include "cmd_process.h"
#include "coordinate.h"
#include "log.h"
#include "database.h"

#define DATABASE_PATH "/mnt/nfs/gui_nfs_test/db.dat"
#define RTCHardWareDEV_PATH		"/dev/rtc0"

#define DIDO_OUTPUT_QUERY 	 	0x03
#define DIDO_INPUT_QUERY   	 		0x04
#define DIDO_PARAMCONFIG_QUERY  		0x05

#define DIDO_QUERY_INPUT	    		1
#define DIDO_QUERY_OUTPUT			2
//A3-Sub
#define CCGlobalParaSet 			1
#define CCGlobalParaQuery 			2
#define CCGlobalVersionQuery 			3
#define CCGlobalControlMethSet 		4

#define TimerTaskSet        	 		0x01
#define TimerTaskVerificat        		0x02
#define TimerTaskDelete    		  	0x03
#define TimerTaskQuery   	      		0x04
#define TimerTaskQuatityMax		  	5

extern struct task_queue 			taskQueue[QUEUE_TOTAL_QUANTITY];	//QUEUE_TOTAL_QUANTITY
extern struct TOPDeviceMeterStatStruct 	TOPDeviceMeterStat;
extern volatile SINT 				TOPCCRegistr2ServerState;
extern volatile SINT 				TOPHEARTBEATACK;
extern volatile SINT 				TOPGLOBALMEMALLOCTIMES;
extern volatile UCHAR 				TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];
extern volatile UCHAR 				TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];
extern volatile UCHAR 				TOP_LOCALREALISTIC_DIINFOR_STAT[DIDO_DEVICE_QUANTITY];
extern struct MetterElecInforStruc 		TOPMetterElecInformation;
extern GlobalCCparam 				CCparamGlobalInfor;
extern faalitf_t 					g_faalitf[4];
extern AtCMDs 					AtCMDs_ACT[];
TimerTaskToSaveFormatStruct 			TimerTaskList[TimerTaskQuatityMax];


SINT TOPGeneralShortAckToServer(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD,UCHAR Resault)
{
	Buf[0] = 0x51; Buf[1] = 0x05;Buf[2] = 0x80;Buf[3] = 0x03; Buf[4] = ActRequCMD;
	Buf[5] = ActRequSubCMD;Buf[6] = Resault;

	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,Buf,NET_TASK,TASK_LEVEL_NET)){
		return SUCCESS;
	}//start
	else{
		printf("TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
		return TASK_QUEUE_APPEND_ERR;
	}

}

SINT TOPLongAckToServer(UCHAR *ACKSendBufLong,UCHAR ActRequCMD,UCHAR ActRequSubCMD,UCHAR MeterAddr)
{
			ACKSendBufLong[0] = 0x52;ACKSendBufLong[1] = 0x97+0x01;ACKSendBufLong[2] = 0x80;
			ACKSendBufLong[3] = 0x97;ACKSendBufLong[4] = ActRequCMD;ACKSendBufLong[5] = ActRequSubCMD+1;
			ACKSendBufLong[6] = MeterAddr;

			memcpy(&ACKSendBufLong[7],&TOPMetterElecInformation,sizeof(struct MetterElecInforStruc));
			#if DEBUG_LOCAL_METTER
				printf("***********************\n");
				printf("sizeof(struct MetterElecInforStruc) is %x\n",sizeof(struct MetterElecInforStruc));
				printf("ACKSendBufLong is %x:%x:%x:%x\n\n\n",ACKSendBufLong[0],ACKSendBufLong[1],ACKSendBufLong[2],ACKSendBufLong[3]);
				printf("%x:%x:%x:\n\n\n",ACKSendBufLong[4],ACKSendBufLong[5],ACKSendBufLong[6]);
				printf("ua is %f,Ub IS %f,UC IS %f\n",TOPMetterElecInformation.Ua,TOPMetterElecInformation.Ub,TOPMetterElecInformation.Uc);
				printf("Ua in float is %x:%x:%x:%x\n\n\n",ACKSendBufLong[7],ACKSendBufLong[8],ACKSendBufLong[9],ACKSendBufLong[10]);
				printf("***********************\n");
			#endif

			if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET)){

				return SUCCESS;
			}//start
			else{

				printf("TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
				return TASK_QUEUE_APPEND_ERR;
			}

}


SINT TOPGeneralCCBusyAckToServer()
{
	return SUCCESS;
}

u32 CallBack_Open(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	u8 ACKSendBufLong[24];
	switch(node->pakect[2]){
		case 0x01://单灯，单控open
			memset(ACKSendBufLong,0,sizeof(ACKSendBufLong));
			ACKSendBufLong[0] = 0x51;ACKSendBufLong[1] = 0x08;ACKSendBufLong[2] = 0x80;
			ACKSendBufLong[3] = 0x06;ACKSendBufLong[4] = 0x01;ACKSendBufLong[5] = 0x42;
			ACKSendBufLong[6] = node->pakect[3];ACKSendBufLong[7] = node->pakect[4];
			ACKSendBufLong[8] = node->pakect[5];
			if( SUCCESS == SingleOpen(node)){
				ACKSendBufLong[9] = SUCCESS;	//回复服务器，开灯成功
				TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
			}else{
				ACKSendBufLong[9] = FAIL;		//回复服务器，开灯失败
				TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
				//return FAIL;
			}break;
		case 0x02://组播
			if( SUCCESS == GroupOpen(node)){
				//回复服务器，开灯成功
				debug(DEBUG_single,"Group Open Success!\n");
			}else{
				//回复服务器，开灯失败
				debug(DEBUG_single,"Group Open Fail!\n");
			}break;
		case 0x03: //广播
			BroadcastOpen(node); break;
		default:break;
	}
	return SUCCESS;
}

u32 CallBack_Close(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	u8 ACKSendBufLong[24];
	switch(node->pakect[2]){
		case 0x01://单灯，单控open
			memset(ACKSendBufLong,0,sizeof(ACKSendBufLong));
			ACKSendBufLong[0] = 0x51;ACKSendBufLong[1] = 0x08;ACKSendBufLong[2] = 0x80;
			ACKSendBufLong[3] = 0x06;ACKSendBufLong[4] = 0x01;ACKSendBufLong[5] = 0x43;
			ACKSendBufLong[6] = node->pakect[3];ACKSendBufLong[7] = node->pakect[4];
			ACKSendBufLong[8] = node->pakect[5];
			if( SUCCESS == SingleClose(node)){
				ACKSendBufLong[9] = SUCCESS;//回复服务器，开灯成功
				TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
			}else{
				ACKSendBufLong[9] = FAIL;//回复服务器，开灯失败
				TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
			}break;
		case 0x02:
			if( SUCCESS == GroupClose(node)){
				//回复服务器，开灯成功
				debug(DEBUG_single,"Group Close Success!\n");
			}else{
				//回复服务器，开灯失败
				debug(DEBUG_single,"Group Close Fail!\n");
			}
			break;
		case 0x03: BroadcastClose(node);break;
		default:
			break;
	}
	return SUCCESS;
}

u32 CallBack_Light(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	u8 ACKSendBufLong[24];
	switch(node->pakect[2]){
		case 0x01://单灯，单控open
			memset(ACKSendBufLong,0,sizeof(ACKSendBufLong));
			ACKSendBufLong[0] = 0x51;ACKSendBufLong[1] = 0x08;ACKSendBufLong[2] = 0x80;
			ACKSendBufLong[3] = 0x06;ACKSendBufLong[4] = 0x01;ACKSendBufLong[5] = 0x47;
			ACKSendBufLong[6] = node->pakect[3];ACKSendBufLong[7] = node->pakect[4];
			ACKSendBufLong[8] = node->pakect[5];
			if( SUCCESS == SingleOpen(node)){
				ACKSendBufLong[9] = SUCCESS;	//回复服务器，开灯成功
				TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
			}else{
				ACKSendBufLong[9] = FAIL;		//回复服务器，开灯失败
				TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
			}
			break;
		case 0x02:
			if( SUCCESS == GroupLight(node)){
				//回复服务器，开灯成功
				debug(DEBUG_single,"Group Light Success!\n");
			}else{
				//回复服务器，开灯失败
				debug(DEBUG_single,"Group Light Fail!\n");
			}
			break;
		case 0x03:BroadcastLight(node);break;
		default:
			break;
	}
	return SUCCESS;
}

u32 CallBack_Demand(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	switch(node->pakect[2]){
		case 0x01://单灯，单播查询
			return SingleQuery(node);
		case 0x02://组播查询
			return GroupQuery(node);
		case 0x03: //广播查询
			return BroadcastQuery(node);
		default:break;
	}
	return SUCCESS;
}

u32 CallBack_electric(u8 ctrl,u8 itf,struct task_node *node)
{
	assert_param(node,NULL,FAIL);
	switch(node->pakect[2]){
		case 0x01://单灯，单播查询
			//return SingleQuery(node);
		case 0x02://组播查询
			//return GroupQuery(node);
		case 0x03: //广播查询
			return Broadcas_electric(node);
		default:break;
	}
	return SUCCESS;
}

u32 CallBack_RtData(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	printf("In %s test\n",__func__);
	return 0;
}

UINT CallBackGPRSTest1(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	printf("in gprs  for test 0x31\n");
	return SUCCESS;
}

UINT CallBackServerFeedback(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	PureCmdArry *CCACK2SERVERPkt = (PureCmdArry *)node->pakect;

	debug(DEBUG_server2cc,"in function [%s] for response:",__func__);
	debug(DEBUG_server2cc,"data[0]:%x,data[1]:%d,data[2]:%d\n",CCACK2SERVERPkt->data[0],CCACK2SERVERPkt->data[1],CCACK2SERVERPkt->data[2]);
	switch(CCACK2SERVERPkt->data[0]){
		case 0xA1:
		/* 登入回复 */
			if((CCACK2SERVERPkt->data[1] == 0x01)){	//logon  subcmd=0x01
				if((CCACK2SERVERPkt->data[2] == SUCCESS)){
					debug(1,"^^^logon ok^^^\n");
					TOPCCRegistr2ServerState = CC_REGISTER_OK;
					return SUCCESS;
				}else{
					debug(DEBUG_LOCAL_CC2SERVER_RIGISTER,"CC_SERVER_REFUSE_LOGON_ERR occer errcode :%d\n",CC_SERVER_REFUSE_LOGON_ERR);
					TOPCCRegistr2ServerState = CC_REGISTER_ERR;
					return CC_SERVER_REFUSE_LOGON_ERR;
				}
			}
		/* 心跳回复 */
			if((CCACK2SERVERPkt->data[1] == 0x02)){
				if((CCACK2SERVERPkt->data[2] == SUCCESS)){
					debug(1,"^^^HeartBeat ok!^^^\n");
					TOPHEARTBEATACK = SUCCESS;
					return SUCCESS;
				}else{
					TOPHEARTBEATACK = FAIL;
					debug(DEBUG_LOCAL_CC2SERVER_HEARTBEAT,"CC_SERVER_REFUSE_ACKHEARTBEAT_ERR occer errcode :%d\n",CC_SERVER_REFUSE_ACKHEARTBEAT_ERR);
					return CC_SERVER_REFUSE_ACKHEARTBEAT_ERR;
				}
			}break;
		default :
			//TOPCCRegistr2ServerState = CC_REGISTER_ERR;
			break;
		}
	return NO_SUCH_SUBCMD;
}


UINT CallBackLogon(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	printf("in function CallBackLogon for test 0xa1\n");
	return SUCCESS;

}

UINT CallBackReset(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	UCHAR ACKSendBuf[70];
	UCHAR COMPULSORY_Telephone[12];
	UCHAR LOCAL_Telephone_NUM[12];
	PureCmdArry *CCRESETRPkt = (PureCmdArry *)node->pakect;
	memset(ACKSendBuf,0,sizeof(ACKSendBuf));
	switch(CCRESETRPkt->data[0]){	//0xA2
		case 0x01:	//reset
			debug(DEBUG_reset,"^^^^^GET CMD RESET\n");
			if(!Reset2DoFunctions()){
				debug(DEBUG_reset,"^^^^^reset success.....\n");
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);
			}else{
				debug(DEBUG_reset,"^^^^^reset fail.....\n");
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], FAIL);
				return CMD_RESET_ACTION_ERR;
			}
			return SUCCESS;
		case 0x02:	//reboot
			debug(DEBUG_reboot,"^^^^^get cmd reboot\n");
			TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);
			sleep(3);
			system("reboot");
			return SUCCESS;
		case 0x03:	//校时任务
			debug(DEBUG_chktime,"^^^^^get cmd check time\n");
			if(!Timer_Correct(CCRESETRPkt)){
				TOPGeneralShortAckToServer(ACKSendBuf,ctrl,node->pakect[2],SUCCESS);/* 应答回复 */

			}else{
				TOPGeneralShortAckToServer(ACKSendBuf,ctrl,node->pakect[2],FAIL);
			}
			return SUCCESS;
		case 0x04://查询时间
			debug(DEBUG_inqTime,"^^^^^GET CMD inquire time\n");
			CCLocateTimeUpLoad(ACKSendBuf,ctrl,node->pakect[2]);
			return SUCCESS;
		case 0x05://查询版本号
			debug(DEBUG_inqVersion,"#####Get CMD inquire Version!\n");
			return CC_Inquire_Version(ACKSendBuf,ctrl,node->pakect[2]);
		case 0x20:
			return SMS_MODULE_INFOR_MakeAND_UpLoad(ACKSendBuf,ctrl,node->pakect[2]);
		case 0x21:
			memcpy(COMPULSORY_Telephone,&node->pakect[3],12);
			COMPULSORY_Telephone[11] = '\0';
			SMS_MODULE_DAIL_COMPULSORY(ACKSendBuf,ctrl,node->pakect[2],COMPULSORY_Telephone);
			break;
		case 0x23:
			memcpy(LOCAL_Telephone_NUM,&node->pakect[3],12);
			LOCAL_Telephone_NUM[11] = '\0';
			SMS_MODULE_INIT_SET_UpLoad(ACKSendBuf,ctrl,node->pakect[2],LOCAL_Telephone_NUM);
			break;
		case 0x70:{
			debug(DEBUG,"\nsystem will poweroff after 5 seconds......\n\n");
			sleep(1);
			TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);
			sleep(4);
			system("poweroff");
		}
		default:break;
	}//end of switch
	return SUCCESS;
}

UINT CallBackSMSModule_RLT(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	return SUCCESS;
}

USHORT ServerPortGenerate(UCHAR *ptr)
{
	return ((*ptr)*256 + *(ptr+1));
}

CHAR CCParaCheckAvailable(void)
{
	return SUCCESS;
}

static s32 CCGlobalparamInforUpdate(u8 *P_reciv)
{
	char temp[64] = {0};
	memset(temp,0,sizeof(temp));
	debug(DEBUG_CC_Config,"Config for CC Start!\n");

 #if 1
	/* 配置全局变量CCparamGlobalInfor */
	CCparamGlobalInfor.ItfWay = *P_reciv++;
	debug(DEBUG_CC_Config,"CCparamGlobalInfor.ItfWay = %d\n",CCparamGlobalInfor.ItfWay);
	/* IP */
	memcpy(temp,P_reciv,32); P_reciv += 32;
	#if 1
	in_addr_t tp_addr,seraddr;
	tp_addr = ntohl(inet_addr(temp));
	seraddr = ntohl(inet_addr(CCparamGlobalInfor.ServerIpaddr));
	debug(DEBUG_CC_Config,"tpIp=%s\nIP=%s\n",temp,CCparamGlobalInfor.ServerIpaddr);
	debug(DEBUG_CC_Config,"tp_addr=0x%x,seraddr=0x%x\n",tp_addr,seraddr);
	/* 把配置的服务器ip赋值给全局变量 */
	//memcpy(CCparamGlobalInfor.ServerIpaddr,temp,32);

	/* 检测和当前ip是否在同一网段，默认为c类网 */
	if( (tp_addr&(~0xff)) != (seraddr&(~0xff)) ){	//不在同一网段上，就要设置本地ip了
		debug(DEBUG_CC_Config,"Not in the same network segment\n");
		char dest[32]={0},src[32]={0};
		int i=0;
		int desti=0,srci=0;
		while(i<32){
			if(temp[i] == '.'){++desti;}
			if(CCparamGlobalInfor.ServerIpaddr[i] == '.'){++srci;}
			if(desti <=2 ){
				dest[i] = temp[i];
			}
			if(srci <= 2){
				src[i] = CCparamGlobalInfor.ServerIpaddr[i];
			}
			++i;
			if(srci >2 && desti >2) break;
		}

		memset(temp,0,sizeof(temp));
		sprintf(temp,"sed -i 's/address %s/address %s/' /etc/network/interfaces",src,dest);
		debug(DEBUG_CC_Config,"shell cmd:%s\n",temp);
		//system(temp);		//更改系统中的ip配置，改成同一网段上
		sprintf(temp,"sed -i 's/gateway %s/gateway %s/' /etc/network/interfaces",src,dest);
		debug(DEBUG_CC_Config,"shell cmd:%s\n",temp);
		//system(temp);		//更改系统中的网关配置，改成同一网段上
		/* 重启网卡 */
		//system("ifdown eth0");
		//sleep(5);
		//system("ifup eth0");
	}
	#endif
	debug(DEBUG_CC_Config,"CCparamGlobalInfor.ServerIpaddr %s\n",CCparamGlobalInfor.ServerIpaddr);
	/* Port */
	//CCparamGlobalInfor.ServerPort   = *P_reciv++;
	//CCparamGlobalInfor.ServerPort |= (*P_reciv++) << 8;
	u16 	tport 	  = *P_reciv++ << 8 ;
		tport 	*= *P_reciv++;
	debug(DEBUG_CC_Config,"CCparamGlobalInfor.ServerPort=0x%04x\n",tport);

	/* HeartBCycle */
	//CCparamGlobalInfor.HeartBCycle = *P_reciv++;
	debug(DEBUG_CC_Config,"CCparamGlobalInfor.HeartBCycle=%d\n",*P_reciv++);
	/* UID */
	memcpy(CCparamGlobalInfor.CCUID,P_reciv,6);
	Display_package("CCparamGlobalInfor.CCUID",CCparamGlobalInfor.CCUID,sizeof(CCparamGlobalInfor.CCUID));
	P_reciv += 6;
	CCparamGlobalInfor.KwhSaveDay 	= *P_reciv++;
	CCparamGlobalInfor.KwhReadInter 	= *P_reciv;
	/* 将现在的配置写入到配置文件/数据库中 */
	SaveParam();
 #endif
	/* 重新连接服务器 */
	debug(DEBUG_CC_Config,"Config for CC end!\n");
	return SUCCESS;
}

s32  Column_IsExist(int cmd ,int Condition)
{
	int SelectBuf = 0;
	switch(cmd){
		case Cmd_light:
			Select_Table_V2(Asprintf("select Base_Addr from db_light where Base_Addr=%d ;",Condition),(char*)&SelectBuf,sizeof(SelectBuf),1,0);
			break;
		case Cmd_coordi:
			Select_Table_V2(Asprintf("select Base_Addr from db_coordinator where Base_Addr=%d ;",Condition),(char*)&SelectBuf,sizeof(SelectBuf),1,0);
			break;
		default:break;
	}
	if(SelectBuf){
		debug(1,"In database was exist Addr 0x%x\n",SelectBuf);
		return 1;
	}else{
		debug(1,"In database does not exist Addr 0x%x\n",Condition);
		return 0;
	}
}

s32 Update_ExistColumn(int cmd , void *buf)
{
	char sql[200]={0};
	TableSingle_t *Single  = NULL;
	TableCoordi_t *Coordi= NULL;
	u8 strbuf[24];
	memset(sql,0,sizeof(sql));
	memset(strbuf,0,sizeof(strbuf));
	switch(cmd){
		case Cmd_light:
			Single  = buf;
			sprintf(sql,"set Wl_Addr=%d,lt_gid=%d,Coor_id=%d,Map_Addr=%d where Base_Addr=%d",
						Single->Wl_Addr,Single->lt_gid,Single->Coor_id,Single->Map_Addr,Single->Base_Addr);
			return Update_Table(Cmd_light,sql);
		case Cmd_coordi:
			Coordi= buf;
			HexToStr_v3(strbuf, (u8*)Coordi->CC_id ,6);
			sprintf(sql,"set Wl_Addr=%d,Coor_gid=%d,CC_id='%s',Map_Addr=%d where Base_Addr=%d",
						Coordi->Wl_Addr,Coordi->Coor_gid,strbuf,Coordi->Map_Addr,Coordi->Base_Addr);
			return Update_Table(Cmd_coordi,sql);
		default:break;
	}
	return SUCCESS;
}

static s32 CC_Single_group(u8 *Pdata)
{
	assert_param(Pdata,NULL,FAIL);

	TableSingle_t   single;
	single.Wl_Addr 	= *Pdata++;
	single.lt_gid		= *Pdata++;
	single.Coor_id 		= *Pdata++;
	single.Base_Addr 	= *Pdata++ << 8;
	single.Base_Addr 	+= *Pdata++;
	single.Map_Addr 	= *Pdata++ <<8;
	single.Map_Addr 	+= *Pdata;
	debug(DEBUG_CC_Config,"Wl_Addr=0x%02x,lt_gid=0x%02x,Coor_id=0x%02x,Base_Addr=0x%04x,Map_Addr=0x%04x\n",
							single.Wl_Addr,single.lt_gid,single.Coor_id,single.Base_Addr,single.Map_Addr);

	/* 发送配置信息到单灯 */
	if(SUCCESS != Single_Config(Single_ConfigMapAddr,&single)){
		debug(DEBUG_CC_Config,"Single Config MapAddr Fail!\n");
		return FAIL;
	}
	if(SUCCESS != Single_Config(Single_ConfigGroup,&single)){
		debug(DEBUG_CC_Config,"Single Config Group Fail!\n");
		return FAIL;
	}
	int res = -1;
	if(Column_IsExist(Cmd_light,single.Base_Addr)){
		res = Update_Table(Cmd_light,Asprintf("set Wl_Addr=%d,lt_gid=%d,Coor_id=%d,Map_Addr=%d where Base_Addr=%d",\
					single.Wl_Addr,single.lt_gid,single.Coor_id,single.Map_Addr,single.Base_Addr));
	}else{
		res = Insert_Table_v2(Asprintf("insert into db_light(Wl_Addr,Base_Addr,lt_gid,Coor_id,Map_Addr)  values(%d,0x%04X,%d,%d,%d);",\
			single.Wl_Addr,single.Base_Addr,single.lt_gid,single.Coor_id,single.Map_Addr));
		if(SUCCESS == res){
			res = Insert_Table_v2(Asprintf("insert into db_info_light(Base_Addr,Warn_flags) values(0x%04X,0);",single.Base_Addr));
			if(SUCCESS != res) Delete_Table(Cmd_light,Asprintf("where Base_Addr=%d",single.Base_Addr));
		}
	}
	return res;
}
static s32 CC_Coordi_group(u8 *Pdata)
{
	assert_param(Pdata,NULL,FAIL);

	//int res = -1;
	TableCoordi_t coordi;

	coordi.Wl_Addr 	= *Pdata++;
	coordi.Coor_gid 	= *Pdata++;
	HexToStr_v3((u8*)coordi.CC_id, (u8*)Pdata, 6); Pdata += 6 ;
	coordi.Base_Addr 	= *Pdata++;
	coordi.Map_Addr 	= *Pdata;
	debug(DEBUG_CC_Config,"CCUID: %s,Wl_Addr=0x%02x,Coor_gid=0x%02x,Base_Addr=0x%02x,Map_Addr=0x%02x\n",\
						coordi.CC_id,coordi.Wl_Addr,coordi.Coor_gid,coordi.Base_Addr,coordi.Map_Addr);

	/* 发送配置信息到单灯 */
	if(SUCCESS != Single_Config(Coordi_ConfigMapAddr,&coordi)){
		debug(DEBUG_CC_Config,"Coordinate Config Group Fail!\n");
		return FAIL;
	}

	if(Column_IsExist(Cmd_coordi,coordi.Base_Addr)){		//表中已经存在该单灯，则更新表中的数据
		return Update_Table(Cmd_coordi,Asprintf("set Wl_Addr=%d,Coor_gid=%d,CC_id='%s',Map_Addr=%d where Base_Addr=%d",\
			coordi.Wl_Addr,coordi.Coor_gid,coordi.CC_id,coordi.Map_Addr,coordi.Base_Addr));
	}else{
		return Insert_Table_v2( Asprintf("insert into db_coordinator(Wl_Addr,Base_Addr,Coor_gid,CC_id,Map_Addr) values(%d,%d,%d,'%s',%d);",\
			coordi.Wl_Addr,coordi.Base_Addr,coordi.Coor_gid,coordi.CC_id,coordi.Map_Addr));
	}
	return SUCCESS;
}


static s32 CC_Tasklist_Config(u8 *Pdata)
{
	if(!Pdata){return FAIL;}
	s32 ii = 0;
	TableTasklist_t tasklist;
	memset(&tasklist,0,sizeof(tasklist));

	tasklist.Tk_id = *Pdata++;
	tasklist.Rank = *Pdata++;
	/* 等待时间 */
	ii = 3; while(ii >= 0){ tasklist.Wait_time 	|= *Pdata++ << (8*ii--);}
	/* 把16禁止的命令转化成字符串 */
	HexToStr_v3(tasklist.Cmd, Pdata+1, *Pdata);
	debug(DEBUG_CC_Config,"tkid=0x%02x,rank=0x%02x,waittime=%08x,Cmd=%s\n",tasklist.Tk_id,tasklist.Rank,(u32)tasklist.Wait_time,tasklist.Cmd);
	return Insert_Table(Cmd_tasklist, &tasklist);
}

s32 Del_Sqlite(u8 *Pdata)
{
	char Sql[128];
	time_t start = 0,end = 0;
	struct tm tim;
	memset(Sql,0,sizeof(Sql));
	memset(&tim,0,sizeof(tim));
	switch(*Pdata++){
		case 0x00://协调器记录表
			if(*Pdata++ == 0x00){
				sprintf(Sql,"where Base_Addr=%d ",*Pdata);
				return Delete_Table(Cmd_coordi,Sql);
			}else{
				debug(DEBUG_DelSql,"db_coordinator have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}break;
		case 0x01://单灯记录表
			if(*Pdata++ == 0x00){
				sprintf(Sql,"where Base_Addr=%d ",*Pdata<<8 | *(Pdata+1));
				return Delete_Table(Cmd_light,Sql);
			}else{
				debug(DEBUG_DelSql,"db_light have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}break;
		case 0x02://任务表
			if(*Pdata++ == 0x01){//名称
				sprintf(Sql,"where Name='%s' ",Pdata);
				return Delete_Table(Cmd_task,Sql);
			}else if(*(Pdata-1) == 0x02){//时间范围
				tim.tm_year	= 100+*Pdata++;
				tim.tm_mon	= *Pdata++ - 1;
				tim.tm_mday	= *Pdata++;
				tim.tm_hour	= *Pdata++;
				tim.tm_min	= *Pdata++;
				tim.tm_sec	= *Pdata++;
				start = mktime(&tim);
				tim.tm_year	= 100+*Pdata++;
				tim.tm_mon	= *Pdata++ - 1;
				tim.tm_mday	= *Pdata++;
				tim.tm_hour	= *Pdata++;
				tim.tm_min	= *Pdata++;
				tim.tm_sec	= *Pdata;
				end = mktime(&tim);
				sprintf(Sql,"where Start_Date < %ld AND Start_Date > %ld",end,start);
				return Delete_Table(Cmd_task,Sql);
			}else{
				debug(DEBUG_DelSql,"db_task have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}break;
		case 0x03://报警日志记录表
			if(*Pdata++ == 0x02){//名称
				tim.tm_year	= 100+*Pdata++;
				tim.tm_mon	= *Pdata++ - 1;
				tim.tm_mday	= *Pdata++;
				tim.tm_hour	= *Pdata++;
				tim.tm_min	= *Pdata++;
				tim.tm_sec	= *Pdata++;
				start = mktime(&tim);
				tim.tm_year	= 100+*Pdata++;
				tim.tm_mon	= *Pdata++ - 1;
				tim.tm_mday	= *Pdata++;
				tim.tm_hour	= *Pdata++;
				tim.tm_min	= *Pdata++;
				tim.tm_sec	= *Pdata;
				end = mktime(&tim);
				sprintf(Sql,"where Start_Date < %ld AND Start_Date > %ld",end,start);
				return Delete_Table(Cmd_warn,Sql);
			}else{
				debug(DEBUG_DelSql,"db_warn have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}break;
		default:break;
	}return SUCCESS;
}

static s32 CC_Task_Config(u8 *Pdata)
{
	if(!Pdata){return FAIL;}
	volatile s32 ii = 0;
	TableTask_t task;
	memset(&task,0,sizeof(task));
	task.Priority = *Pdata++;
	strcpy((char*)task.Name,(char*)Pdata);
	Pdata += 32;
	ii = 3; while(ii >= 0){ task.Start_Date 	|= *Pdata++ << (8*ii--);}
	ii = 3; while(ii >= 0){ task.End_Date 	|= *Pdata++ << (8*ii--);}
	ii = 3; while(ii >= 0){ task.Run_Time 	|= *Pdata++ << (8*ii--);}
	ii = 3; while(ii >= 0){ task.Inter_Time 	|= *Pdata++ << (8*ii--);}
	task.Type 	= *Pdata++;
	task.State 	= *Pdata;
	debug(DEBUG_CC_Config,"name:%s,priority=%02x,start_t=%08x,end_t=%08x,run_t=%08x,inter_t=%08x,type=%02x,stat=%02x\n",task.Name,
		task.Priority,(u32)task.Start_Date,(u32)task.End_Date,(u32)task.Run_Time,(u32)task.Inter_Time,task.Type,task.State);
	return Insert_Table(Cmd_task, &task);
}

UINT CallBackCCGlobalParaSetGet(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	PureCmdArry *TimeTaskPkt = (PureCmdArry *)node->pakect;
	UCHAR * P_reciv;
	UCHAR AckShortbuf[20];
	P_reciv = (UCHAR *)&TimeTaskPkt->data[1];
	switch(TimeTaskPkt->data[0]){
		case 0x01:/* 集中器参数设置下行命令 */
			if( SUCCESS == CCGlobalparamInforUpdate(P_reciv)){
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],SUCCESS);
			}else{
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],FAIL);
			}return SUCCESS;
		case 0x05:		// 集中器参数单灯分组配置下行命令
			if(SUCCESS == CC_Single_group(P_reciv)){
				debug(DEBUG_CC_Config,"Config Single group success!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],SUCCESS);
			}else{
				debug(DEBUG_CC_Config,"Config Single group fail!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],FAIL);
			}return SUCCESS;
		/* 集中器参数协调器分组配置下行命令 */
		case 0x06:
			if(SUCCESS == CC_Coordi_group(P_reciv)){
				debug(DEBUG_CC_Config,"Config Coordinate group success!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],SUCCESS);
			}else{
				debug(DEBUG_CC_Config,"Config Coordinate group fail!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],FAIL);
			}return SUCCESS;
		/* 集中器参数配置任务下行命令 */
		case 0x07:
			if(SUCCESS == CC_Task_Config(P_reciv)){
				debug(DEBUG_CC_Config,"Config Task group success!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],SUCCESS);
			}else{
				debug(DEBUG_CC_Config,"Config Task group fail!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],FAIL);
			}return SUCCESS;
		/* 集中器配置任务明细下行命令 */
		case 0x08:
			if(SUCCESS == CC_Tasklist_Config(P_reciv)){
				debug(DEBUG_CC_Config,"Config Tasklist group success!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],SUCCESS);
			}else{
				debug(DEBUG_CC_Config,"Config Tasklist group fail!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],FAIL);
			}return SUCCESS;
		case 0x09://删除数据库中的内容
			if(SUCCESS ==Del_Sqlite(P_reciv)){
				debug(DEBUG_CC_Config,"delete sql table success!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],SUCCESS);
			}else{
				debug(DEBUG_CC_Config,"delete sql table fail!\n");
				TOPGeneralShortAckToServer(AckShortbuf, ctrl, node->pakect[2],FAIL);
			}return SUCCESS;
		default :	break;
	}

	return FAIL;
}

UINT CallBackETHShotAck(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	u8 SendBuf[80];
	int PacketLen;
	faalpkt_t *Packet=(faalpkt_t *)SendBuf;
	memcpy(&(Packet->ctrl),&(node->pakect[2]),node->pakect[1]);
	PacketLen = basic_makepkt(itf, Packet);
	#ifdef Config_Log
		Write_log(CC, SendBuf );
	#endif
	if((*g_faalitf[itf].rawsend)((UCHAR *)Packet,PacketLen)) {
		return FAIL;
	}
	return SUCCESS;
}

UINT CallBackETHLongContexBack(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	u8 SendBuf[400];
	int PacketLen;
	faalpkt_t *Packet=(faalpkt_t *)SendBuf;
	memcpy(&(Packet->ctrl),&(node->pakect[2]),node->pakect[1]);
	PacketLen = basic_makepkt(itf, Packet);

	#ifdef Config_Log
		//Write_log(CC, SendBuf );
	#endif

	if((*g_faalitf[itf].rawsend)((UCHAR *)Packet, PacketLen)) {
		return FAIL;
	}
	return SUCCESS;
}

UINT CallBackApplicUpDate(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	UCHAR ACKSendBuf[60];
	memset(ACKSendBuf,0,sizeof(ACKSendBuf));
	switch(node->pakect[2]){
		case 0x01:break;
		case 0x05://协调器升级
			if(SUCCESS == Coordinator_Update(node)){
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);
			}else{
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], FAIL);
			}break;
		case 0x06://单灯升级Single_Update
			if(SUCCESS == Single_Update(node)){
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);
			}else{
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], FAIL);
			}break;
			default:break;
	}
	return SUCCESS;
}

UINT CallBackTimerControlTaskRLT(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	UCHAR ACKSendBuf[60];
	memset(ACKSendBuf,0,sizeof(ACKSendBuf));
	PureCmdArry *TimeTaskPkt = (PureCmdArry *)node->pakect;
	//解析收到的数据包
	switch(TimeTaskPkt->data[0]){
		/* 下发定时任务下行命令 */
		case TimerTaskSet:
			if( SUCCESS == TimetaskInsertSQL((u8*)&TimeTaskPkt->data[1], TimeTaskPkt->len)){
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);
			}else{
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], FAIL);
			}break;
		default:		break;
	}
	return SUCCESS;
}

UINT CallBackMetterInfoColletRD(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	struct MetterElecInforStruc *RealTimeElecInfor;
	UCHAR ACKSendBuf[60];
	UCHAR ACKSendBufLong[300];
	memset(ACKSendBuf,0,sizeof(ACKSendBuf));
	memset(ACKSendBufLong,0,sizeof(ACKSendBufLong));

	switch(node->pakect[2]){
		case 1:	//real time information get
			// struct MetterElecInforStruc *RealTimeElecInfor;
			//RealTimeElecInfor = (struct MetterElecInforStruc *)malloc(sizeof(struct MetterElecInforStruc));
			RealTimeElecInfor = &TOPMetterElecInformation;

			if(!MetterRealTimeRDOprate(node->pakect[3],node->pakect[4],RealTimeElecInfor)){

				TOPLongAckToServer(ACKSendBufLong, ctrl, node->pakect[2], node->pakect[3]);//return SUCCESS;


				//TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);//start
				//small block append
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], SUCCESS);


			}//end of if(!MetterRealTimeRDOprate(node->pakect[3],
			else{
				TOPGeneralShortAckToServer(ACKSendBuf, ctrl, node->pakect[2], FAIL);

				printf("METER_REALTIME_READ_ERR : %d\n",METER_REALTIME_READ_ERR);
				return METER_REALTIME_READ_ERR;
			};
			//send back short


			//date infor ##########memcpy();
			break;
		case 3:		//record information get
			//MetterRecordRDOprate(/*DATABASE_PATH*/);
			break;
		default:
			break;
	}

	debug(DEBUG_LOCAL_METTER,"in function CallBackMetterInfoColletRD\n");


	return SUCCESS;
}

UINT CallBackMetterInfoColletSet(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	debug(DEBUG_LOCAL_METTER,"in function CallBackMetterInfoColletSet\n");
	return SUCCESS;
}

u32 CallBackMetterInfoBroadcast(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	u8  cl = node->pakect[0];
	node->pakect[0] = node->pakect[2];
	node->pakect[2] = cl;
	if( TaskGenerateAndAppend(Coordinate_Queue  ,  (u8*)node->pakect,   Coordinate_Task,  Task_Level_Coor) ){
		debug(DEBUG_TaskAppend,"In %s Task Append fail !\n",__func__);
	}
	return SUCCESS;
}

UINT CallBackMetterInfoColletExtrSet(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	debug(DEBUG_LOCAL_METTER,"in function CallBackMetterInfoColletSet\n");
	return SUCCESS;
}

UINT CallBackMetterInfoColletExtrRD(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	debug(DEBUG_LOCAL_METTER,"in function CallBackMetterInfoColletSet\n");
	return SUCCESS;
}

SINT TOPDIDOShortCallBackAck(UCHAR *buf,CHAR AppendType,CHAR AppendLen,CHAR AckCMD,CHAR AckLen,CHAR AckRequCMD,
						CHAR AckRequSubCMD,CHAR DeviceID,CHAR ChanleID,CHAR Stat,CHAR Resault)
{
		buf[0] = AppendType;buf[1] = AppendLen;buf[2] = AckCMD;
		buf[3] = AckLen;buf[4] = AckRequCMD;buf[5] = AckRequSubCMD;
		buf[6] = DeviceID;buf[7] = ChanleID;buf[8] = Stat;
		buf[9] = Resault;
		if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,buf,NET_TASK,TASK_LEVEL_NET)){
			return SUCCESS;

		}else {
			return DIDO_OPENINF_SBACK_ERR;
		}
}


UINT CallBackMetterDIDO(UCHAR ctrl,UCHAR itf,struct task_node *node)
{
	switch(node->pakect[2]){
		case 0x01:  return DIDO_Open(node);
		case 0x02:  return DIDO_Close(node);
		case 0x03:  return DIDO_ReadDO(node);
		case 0x04:  return DIDO_ReadDI(node);
		default:break;
	}
	return SUCCESS;
}

UINT MetterRealTimeRDOprate(CHAR DeviceId,CHAR contex, void *MetterElecInfor1)
{
	//declear var
	struct MetterElecInforStruc *MetterElecInfor = (struct MetterElecInforStruc*)MetterElecInfor1;
	UCHAR Reciv[250];
	SINT RecivLen,i,j;
	UCHAR crc[2];
	UCHAR ActSendBufUart[METTERCMD_LENMAX];
	UCHAR *ptr;
	//9c40 == 40000
	//struct MetterOpratePack MetterRealTimeRDCmd = {0x01,0x03,0x9c,0x40,0x00,0x4a};
	//test del struct MetterOpratePack MetterRealTimeRDCmd = {0x01,0x03,0x9c,0x40,0x00,0x04};
	struct MetterOpratePack MetterRealTimeRDCmd = {0x01,0x03,0x9c,0x40,0x00,0x68};	//new

	debug(DEBUG_LOCAL_METTER,"in function MetterRealTimeRDOprate\n");


	memset(ActSendBufUart,0,sizeof(ActSendBufUart));

	MetterRealTimeRDCmd.address = DeviceId;

	if(UartBufClear(Uart2_ttyS3_485,Flush_Input|Flush_Output)){return UART_BUF_CLEAR_ERR;}
	memcpy(ActSendBufUart,&MetterRealTimeRDCmd,6);

	//crc
	if(!Crc16((UCHAR *)crc, (UCHAR *)&MetterRealTimeRDCmd, 6)){
		ActSendBufUart[6+1] = crc[0];
		ActSendBufUart[6] = crc[1];
	}

	debug(DEBUG_LOCAL_METTER,"actsend and crc is %x:%x:%x:%x:%x:%x:%x:%x:\n",
		ActSendBufUart[0],ActSendBufUart[1],ActSendBufUart[2],ActSendBufUart[3],
			ActSendBufUart[4],ActSendBufUart[5],ActSendBufUart[6],ActSendBufUart[7]);



	//send
	if(Uart_Send(Uart2_ttyS3_485,(CHAR *)ActSendBufUart,  6+2,  0)){
		Uart_Send(Uart2_ttyS3_485,(CHAR *)ActSendBufUart,  6+2,  0);
	}

	//recive
	if(!Device485Reciv(UART2_TTYS3_485,Reciv,&RecivLen)){

		if(DEBUG_LOCAL_METTER){
			printf("act send in open is %x:%x:%x:%x:%x:%x:%x:%x\n",ActSendBufUart[0],ActSendBufUart[1],ActSendBufUart[2],ActSendBufUart[3],ActSendBufUart[4],
			ActSendBufUart[5],ActSendBufUart[6],ActSendBufUart[6]);
			printf("recive buf in open oprate is %x:%x:%x:%x:%x:%x:%x:\n",Reciv[0],Reciv[1],Reciv[2],Reciv[3],
			Reciv[4],Reciv[5],Reciv[6]);
			printf("RecivLen in open oprate  is %d\n",RecivLen);
		}
	}

	//format change
	ptr = &Reciv[3];
	debug(DEBUG_LOCAL_METTER,"size of sturct is %d\n",sizeof(struct MetterElecInforStruc));
	debug(DEBUG_LOCAL_METTER,"size of Reciv-5 is %d\n",RecivLen-5);
	if(RecivLen-5!=(sizeof(struct MetterElecInforStruc))){
		return FAIL;
	}else{
		for(i=1,j=0;i<=(sizeof(struct MetterElecInforStruc))/4;i++){
			((char *)MetterElecInfor)[j++]=ptr[i*4-1];
			((char *)MetterElecInfor)[j++]=ptr[i*4-2];
			((char *)MetterElecInfor)[j++]=ptr[i*4-3];
			((char *)MetterElecInfor)[j++]=ptr[i*4-4];
		}
	}

	#if DEBUG_LOCAL_METTER
		//TopElecInforPrintf(MetterElecInfor);
		TopElecInforPrintf((struct MetterElecInforStruc *)&TOPMetterElecInformation);
	#endif

	return SUCCESS;
}

SINT TopElecInforPrintf(struct MetterElecInforStruc *MetterElecInfor)
{
	printf("U Infor is Ua : %f;UB : %f;UC : %f;Up : %f\n",MetterElecInfor->Ua,MetterElecInfor->Ub,MetterElecInfor->Uc,MetterElecInfor->Up);
	printf("Uxx Infor is UaB : %f;UBC : %f;UCA : %f;UL : %f\n",MetterElecInfor->Uab,MetterElecInfor->Ubc,MetterElecInfor->Uca,MetterElecInfor->Ul);
	printf("I Infor is Ia : %f;IB : %f;IC : %f;I : %f\n",MetterElecInfor->Ia,MetterElecInfor->Ib,MetterElecInfor->Ic,MetterElecInfor->I);
	printf("F is %f\n",MetterElecInfor->F);
	printf("P Infor is Pa : %f;PB : %f;PC : %f;P : %f\n",MetterElecInfor->Pa,MetterElecInfor->Pb,MetterElecInfor->Pc,MetterElecInfor->P);
	printf("Q Infor is Qa : %f;QB : %f;QC : %f;Q : %f\n",MetterElecInfor->Qa,MetterElecInfor->Qb,MetterElecInfor->Qc,MetterElecInfor->Q);
	printf("S Infor is Sa : %f;SB : %f;SC : %f;S : %f\n",MetterElecInfor->Sa,MetterElecInfor->Sb,MetterElecInfor->Sc,MetterElecInfor->S);
	printf("PF Infor is PFa : %f;PFB : %f;PFC : %f;PF : %f\n",MetterElecInfor->PFa,MetterElecInfor->PFb,MetterElecInfor->PFc,MetterElecInfor->PF);
	printf("WAT Infor is PFa : %f;kvarh : %f;kVAh : %f\n",MetterElecInfor->kWh,MetterElecInfor->kvarh,MetterElecInfor->kVAh);
	return SUCCESS;
}

UINT Reset2DoFunctions(void)
{
	//loadParam();
	/* 清空日志文件 */
	return SUCCESS;
}

SINT SMS_MODULE_DAIL_COMPULSORY(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD,UCHAR *Telphone)
{
	CHAR Infor_Bak_Bufs[20];
	u8 SendBuf[80];
	//CHAR *p;
	CHAR *p_no_dialtone = "DIALTONE";
	CHAR *p_busy = "BUSY";
	CHAR *p_no_carrier = "CARRIER";
	CHAR *p_connect = "CONNECT";
	CHAR *p_ok = "OK";
	if(!GPRS_GSM_Call((char*)Telphone,Infor_Bak_Bufs)){
		//start of SendBuf
		SendBuf[0] = 0x51; SendBuf[1] = 0x07;SendBuf[2] = 0x80;SendBuf[3] = 0X05; SendBuf[4] = ActRequCMD;
		SendBuf[5] = ActRequSubCMD;
		//0x01--NO DIALTONE   ox02--BUSY  0x03 -- NO CARRIER 0X04--CONNECT 0x05--OK(after release)
		if (strstr(Infor_Bak_Bufs,p_no_dialtone)){
			SendBuf[6] = 0x01;
		}
		else if(strstr(Infor_Bak_Bufs,p_busy)){
			SendBuf[6] = 0x02;
		}
		else if(strstr(Infor_Bak_Bufs,p_no_carrier)){
			SendBuf[6] = 0x03;
		}
		else if(strstr(Infor_Bak_Bufs,p_connect)){
			SendBuf[6] = 0x04;

		}
		else if(strstr(Infor_Bak_Bufs,p_ok)){
			SendBuf[6] = 0x05;
		}
		else{
			SendBuf[6] = 0xFF;
		}
		SendBuf[7] = 0xFF;
		SendBuf[8] = SUCCESS;

		if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,SendBuf,NET_TASK,TASK_LEVEL_NET)){
			return SUCCESS;
		}//start
		else{
			printf("TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
			return TASK_QUEUE_APPEND_ERR;
		}//end of else

		//return SUCCESS;
	}
	return FAIL;
}

SINT SMS_MODULE_INIT_SET_UpLoad(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD,UCHAR *Telphone)
{
	CHAR ATLID[30];
	int i = 0;
	sprintf(ATLID,"%s\"%s\"\r",AtCMDs_ACT[ATFLID_WR].Cmd,Telphone);
	//strcat(ATLID,Telphone);

	printf(" in SMS_MODULE_INIT_SET_UpLoad and cmd test\n");
	printf("ATLID %s\n",ATLID);

	//gprs oprations
	//INFOR SET原来最后一个参数类型不匹配，添加强转消除警告  Austzhu 2016.3.23
	GPRS_ATCMD_INTER(ATLID, AtCMDs_ACT[ATFLID_WR].ComplateInforReceive,
							AtCMDs_ACT[ATFLID_WR].P_Splited_Infor,(char**)SPLITE_LINE_RULES);
	//INFOR GET
	GPRS_ATCMD_INTER(AtCMDs_ACT[ATFLID_RD].Cmd, AtCMDs_ACT[ATFLID_RD].ComplateInforReceive,
							AtCMDs_ACT[ATFLID_RD].P_Splited_Infor,(char**)SPLITE_LINE_RULES);

	//TEST
	while(AtCMDs_ACT[ATFLID_RD].P_Splited_Infor[i] != NULL)	{
		printf("AtCMDs_ACT[ATFLID_RD].P_Splited_Infor[%d] is %s\n",i,AtCMDs_ACT[ATFLID_RD].P_Splited_Infor[i]);
		i++;
		printf("out of GPRS_ATCMD_INTER && in SMS_MODULE_INFOR_MakeAND_UpLoad");

	}
	//UPLOAD
	Buf[0] = 0x51; Buf[1] = 0x23;Buf[2] = 0x80;Buf[3] = 0X21; Buf[4] = ActRequCMD;
	Buf[5] = ActRequSubCMD;

	//memcpy(Buf[6],cc_para_term.company,strlen(cc_para_term.company));

	memcpy(&Buf[6],AtCMDs_ACT[ATFLID_RD].P_Splited_Infor[0],strlen(AtCMDs_ACT[ATFLID_RD].P_Splited_Infor[0]));
	//len = 15 ,contex 0x00 for back
	memset(&Buf[21],0x00,15);

	Buf[36] = SUCCESS;	//success or fail


	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,Buf,NET_TASK,TASK_LEVEL_NET)){
		return SUCCESS;
	}//start
	else{
		printf("TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
		return TASK_QUEUE_APPEND_ERR;
	}//end of else

	printf("after fill date\n");
}

SINT CCLocateTimeUpLoad(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD)
{

	time_t SystemTimeGet;
	struct tm TimeStru;
	struct tm TimeStruRTC;
	time(&SystemTimeGet);
	TimeStru = *localtime(&SystemTimeGet);

	Buf[0] = 0x51; Buf[1] = 0x12;Buf[2] = 0x80;Buf[3] = 0x10; Buf[4] = ActRequCMD;
	Buf[5] = ActRequSubCMD;

	Buf[6] = (TimeStru.tm_year%100);
	Buf[7] = (TimeStru.tm_mon+1);
	Buf[8] = (TimeStru.tm_mday);

	Buf[9] = TimeStru.tm_wday;
	Buf[10] = (TimeStru.tm_hour);
	Buf[11] = (TimeStru.tm_min);
	Buf[12] = (TimeStru.tm_sec);

	memset(&TimeStruRTC,0,sizeof(TimeStruRTC));

	if(!RTCHardWareTimeGet(RTCHardWareDEV_PATH,&TimeStruRTC)){


		Buf[13] = (TimeStruRTC.tm_year%100);
		Buf[14] = (TimeStruRTC.tm_mon+1);
		Buf[15] = (TimeStruRTC.tm_mday);

		Buf[16] = TimeStruRTC.tm_wday;

		Buf[17] = (TimeStruRTC.tm_hour);
		Buf[18] = (TimeStruRTC.tm_min);
		Buf[19] = (TimeStruRTC.tm_sec);

	}

	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,Buf,NET_TASK,TASK_LEVEL_NET)){
		debug(DEBUG_inqTime,"^^^^^inquire time success\n");
		return SUCCESS;
	}//start
	else{
		printf("TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
		return TASK_QUEUE_APPEND_ERR;
	}//end of else
}

s32 CC_Inquire_Version(u8 *AckBuf,u8 Ctrl,u8 Cmd)
{
	AckBuf[0] = 0x51; AckBuf[1] = 0x24;AckBuf[2] = 0x80;AckBuf[3] = 0x22; AckBuf[4] = Ctrl;
	AckBuf[5] = Cmd;
	sprintf((char*)&AckBuf[6],"%s",VERSION_NUMBER);

	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckBuf,NET_TASK,TASK_LEVEL_NET)){
		debug(DEBUG_inqVersion,"^^^^^inquire Version success\n");
		return SUCCESS;
	}else{
		debug(DEBUG_inqVersion,"^^^^^inquire Version fail!\n");
		return FAIL;
	}
}

SINT RTCHardWareTimeGet(CHAR *FilePath,struct tm *TimeStruRTCPt)
{
	SINT Filept;
	SINT retval;
	struct tm TimeRTC;
	//struct tm TimeRTCHard;
	Filept = open(FilePath,O_RDONLY,0);
	if(Filept == -1){
		printf("Error when open %s,ErrCode is %d\n",FilePath,RTC_DEV_OPEN_ERR);
		return RTC_DEV_OPEN_ERR;
	}
	//ioctl
	retval = ioctl(Filept,RTC_RD_TIME,&TimeRTC);

	//if(ioctl(Filept, RTC_RD_TIME,&TimeRTC)){
	if(retval){
		printf("Error when RTC iotcl :err_code %d\n",RTC_DEV_IOCTL_ERR);
		close(Filept);
		return RTC_DEV_IOCTL_ERR;
	}

	close(Filept);
	*TimeStruRTCPt = TimeRTC;
	return SUCCESS;
}

SINT SMS_MODULE_INFOR_MakeAND_UpLoad(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD)
{
	char compn[15] = "MC52I";
	char test[15] = "fand";
	int i = 0;

	printf("SMS_MODULE_INFOR_MakeAND_UpLoad\n");

	//gprs oprations
	//原来最后一个参数类型不匹配，添加强转消除警告  Austzhu 2016.3.23
	GPRS_ATCMD_INTER(AtCMDs_ACT[ATCNUM].Cmd, AtCMDs_ACT[ATCNUM].ComplateInforReceive,
						AtCMDs_ACT[ATCNUM].P_Splited_Infor,(char**)SPLITE_LINE_RULES);

	//TEST
	while(AtCMDs_ACT[ATCNUM].P_Splited_Infor[i] != NULL)	{
		printf("AtCMDs_ACT[ATCNUM].P_Splited_Infor[%d] is %s\n",i,AtCMDs_ACT[ATCNUM].P_Splited_Infor[i]);
		i++;
		printf("out of GPRS_ATCMD_INTER && in SMS_MODULE_INFOR_MakeAND_UpLoad");

	}

	Buf[0] = 0x51; Buf[1] = 0x38;Buf[2] = 0x80;Buf[3] = 0X36; Buf[4] = ActRequCMD;
	Buf[5] = ActRequSubCMD;

	//memcpy(Buf[6],cc_para_term.company,strlen(cc_para_term.company));

	memcpy(&Buf[6],test,strlen(test));

	memcpy(&Buf[21],compn,strlen(compn));

	Buf[31] = 0xFF;


	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,Buf,NET_TASK,TASK_LEVEL_NET)){
		return SUCCESS;
	}//start
	else{
		printf("TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
		return TASK_QUEUE_APPEND_ERR;
	}//end of else

	printf("after fill date\n");
}

#if 0
SINT RTCHardWareTimeGet(char *dev,struct tm *t)
{
	int fd, retval;
        struct tm rtc_tm;
        fd = open(dev, O_RDONLY);
        if (fd == -1) {
                printf("open %s err\n",dev);
		  return -1;
        }
        /* Read the RTC time/date */
        retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
        if (retval == -1) {
                 printf("ioctl %s err\n",dev);
		  return -1;
        }
        close(fd);
	*t=rtc_tm;
}
#endif
