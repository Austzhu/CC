/********************************************************************
	> File Name:		SingleOpt.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com
	> Created Time:	2016年06月24日 星期五 10时08分15秒
 *******************************************************************/
#include "SingleOpt.h"
#include "main.h"
//static Pag_Single UpdatePackage;

#if 0
static pthread_t thread_checkSingle;

/**
 * cmd:确定查询的种类，广播开/组播开...
 * arg ：组播查询的时候提供组号
 * flags：0～7位，查询成功回复的命令字，0x42/0x43/0x47,
 * 	  8～15位，查询失败回复的命令字，0x42/0x43/0x47,
 * 	  16～23位，查询故障回复的命令字0x05
 * 	  24～31位，组播命令子或者广播命令字02/03
 */
static struct {int cmd;int arg;int flags;} CheckActionArgs;
static void *CheckSingleThread(void *args)
{
	int res =-1;
	debug(1,"\n*****Enter CheckSingleThread!*****\n");
	sleep(1);//等待父线程执行完一次循环，释放锁
	pthread_detach(pthread_self());
	pthread_mutex_lock(&mutex_task);	//获取task锁
	debug(1,"In CheckSingleThread get mutex_task\n");
	if(CheckActionArgs.arg  > 0){
		res = Check_Action(CheckActionArgs.cmd,CheckActionArgs.arg);
	}else{
		res = Check_Action(CheckActionArgs.cmd);
	}
	if(SUCCESS == res){
		SingleShortAckToServer(1,  0xff&(CheckActionArgs.flags>>24),  0xff&CheckActionArgs.flags);
	}else{
		SingleShortAckToServer(1,  0xff&(CheckActionArgs.flags>>24),  0xff&(CheckActionArgs.flags>>8));
	}
	pthread_mutex_unlock(&mutex_task);	//解task锁
	debug(1,"In CheckSingleThread release mutex_task\n");
	pthread_exit(NULL);
}
#endif

void Display_package(const char *Message,void *package,int len)
{
	int ii = 0;
	debug(1,"%s:",Message);
	while(ii < len){
		debug(1,"%02x ",((u8*)package)[ii++]);
	}debug(1,"\n");
}
/**
 * 接收单灯响应数据
 * @param  Pkg 接收数据的缓存区
 * @return     成功：SUCCESS    失败：FAIL
 */
s32 Recv_Package(Pag_Single *Pkg)
{
	int Timeout = 5;
	char *Psingle = NULL;
	while( Timeout--){//等待数据头
		Uart_Recv(Uart1_ttyO1_485, (s8*)&Pkg->Header, 1,  1000000);
		//if(Pkg->Header == 0xff){ break; }else{debug(DEBUG_single,"Header=%02x\n",Pkg->Header);}
		if(Pkg->Header == 0xff){ break; }else{debug(DEBUG_single,".");fflush(stdout);}
	}
	if(Timeout <=0){
		debug(DEBUG_single,"Wait Header Timeout!\n");
		return FAIL;
	}
	if( !DeviceRecv485_v2(Uart1_ttyO1_485,(char*)&Pkg->Ctrl,sizeof(Pag_Single)-1, 3000000) ){
		debug(DEBUG_single,">>>>>Recv data: ");
		Psingle = (char*)Pkg;
		while(Psingle - (char*)Pkg< sizeof(Pag_Single) ){
			debug(DEBUG_single,"%02x ",*Psingle++);
		}debug(DEBUG_single,"\n");
	}else{
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		/* 设置相关状态 */
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Wait Single Response Time Out!");	//写错误信息
		#endif
		return FAIL;
	}
	/* 校验接收到的数据 */
	if(CHK_Crc16(Pkg->Crc16,(u8*)Pkg,sizeof(Pag_Single)-2) ){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Package CRC Check Error!");	//写错误信息
		#endif
		return FAIL;
	}
	#ifdef Config_Log
		Write_log(Coordinate, Pkg);
	#endif
	return SUCCESS;
}

s32 Recv_Package_v2(Pag_Single *Pkg,int Timeout)
{
	int i = 0;
	while( Timeout--){//等待数据头
		Uart_Recv(Uart1_ttyO1_485, (s8*)&Pkg->Header, 1,  100000);	//100ms
		//if(Pkg->Header == 0xff){ break; }else{debug(DEBUG_single,"Header=%02x\n",Pkg->Header);}
		if(Pkg->Header == 0xff){
			break;
		}else{
			if(0 == i%10){
				debug(DEBUG_single,".");fflush(stdout);
			}
		}
	}
	if(Timeout <=0){
		debug(DEBUG_single," Wait Header Timeout!\n");
		return FAIL;
	}
	if( !DeviceRecv485_v2(Uart1_ttyO1_485,(char*)&Pkg->Ctrl,sizeof(Pag_Single)-1, 1000000) ){
		Display_package(">>>>>Recv data",Pkg,sizeof(Pag_Single));
	}else{
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Wait Single Response Time Out!");	//写错误信息
		#endif
		return FAIL;
	}
	/* 校验接收到的数据 */
	if(CHK_Crc16(Pkg->Crc16,(u8*)Pkg,sizeof(Pag_Single)-2) ){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Package CRC Check Error!");	//写错误信息
		#endif
		return FAIL;
	}
	#ifdef Config_Log
		Write_log(Coordinate, Pkg);
	#endif
	return SUCCESS;
}

/**
 *  发送查询指令
 *  Group 组播查询的时候的组号，<0则表示广播查询
 *  成功 SUCCESS，失败 FAIL
 */
static int Is_Group = -2;
SingleList_t SingleList[SingleNumber];
s32 Query_Action(int Group)
{
	char sql[128];
	Pag_Single SingleRes;
	struct {int CoordiAddr;int SingleAddr;}buf[SingleNumber];
	u8 *Single = (u8*)malloc(SingleNumber*2+sizeof(Pag_Single)+2);
	int repeat =0;
	if(NULL == Single){
		err_Print(DEBUG_single,"malloc single fail!\n");
		Is_Group = -2;
		return FAIL;
	}
	memset(Single,0,SingleNumber*2+sizeof(Pag_Single)+2);
	memset(buf,0,sizeof(buf));
	memset(sql,0,sizeof(sql));
	memset(SingleList,0,sizeof(SingleList));

	Pag_Single *P_single 	= (Pag_Single *)Single;
	P_single->Header 	= Single_Header;
	P_single->Ctrl 		= CoordiCtrl_Query;
	P_single->Cmd[0]	= 0x00;
	P_single->Cmd[1]	= 0x01;

	if(Group >= 0){//组播查询
		sprintf(sql,"select Coor_id,Base_Addr from db_light where lt_gid=%d order by Coor_id;",Group);
		Select_Table_V2(sql,(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
	}else{//广播查询
		//Select_Table_V2("select Coor_id,Base_Addr from db_light order by Coor_id desc;",(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
		Select_Table_V2("select Coor_id,Base_Addr from db_light order by Coor_id ;",(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
	}
	sleep(1);	//防止广播或组播操作后，立马发查询指令，前面几个灯的状态会判断有误

	int AddrLen = 0,i = 0;
	u16 *PAddr = (u16*)((char*)P_single + sizeof(Pag_Single));
	int Maxlen = 0,last_i = 0;
	while(buf[i].CoordiAddr != 0){
		SingleList[i].Coordi_Addr 	= 0xff&buf[i].CoordiAddr;
		SingleList[i].Single_Addr[0] 	= (buf[i].SingleAddr >> 8) & 0xff;
		SingleList[i].Single_Addr[1] 	= buf[i].SingleAddr & 0xff;
		if(buf[i].CoordiAddr == buf[i+1].CoordiAddr){
			*PAddr 		= (buf[i].SingleAddr &0xff)<<8;
			*PAddr 		|= buf[i].SingleAddr>>8&0xff;
			++PAddr;
			AddrLen 		+= sizeof(u16);
		}else{
			if((i -last_i) > Maxlen){
				Maxlen = i -last_i+1 ;
			}last_i = i;
			repeat = 0;
			*PAddr 		= (buf[i].SingleAddr &0xff)<<8;
			*PAddr 		|= buf[i].SingleAddr>>8&0xff;
			++PAddr;
			AddrLen 		+= sizeof(u16);

			P_single->Coordi_Addr= buf[i].CoordiAddr;
			P_single->Data[0]	= (AddrLen>>8) & 0xff;
			P_single->Data[1]	= AddrLen & 0xff;
			Crc16(P_single->Crc16,(u8*)P_single, sizeof(Pag_Single)-2);
			Crc16((u8*)PAddr, P_single->Crc16 + sizeof(P_single->Crc16), AddrLen);
			debug(DEBUG_single,"AddrLen=%04x,Coordi_Addr=%02x\n",(P_single->Data[0]<<8)|P_single->Data[1],P_single->Coordi_Addr);
			Display_package("Query Send data",Single,sizeof(Pag_Single)+AddrLen+2);
 sendagina:
			UartBufClear(Uart1_ttyO1_485, Flush_Input|Flush_Output);		//清楚串口输入输出缓存

			Uart_Send(Uart1_ttyO1_485, (s8*)P_single, sizeof(Pag_Single)+AddrLen+2, SingleSendTimeout);

			memset(&SingleRes,0,sizeof(Pag_Single));
			if( SUCCESS != Recv_Package_v2(&SingleRes,SingleRecvTimeout) ){	//等待单灯回复，超过2s则超时。
				err_Print(DEBUG_single,"Wait Single Response Timeout!\n");
				if(repeat < 5){
					++repeat ;
					goto sendagina;
				}
				goto ERR;
			}
			if((ResponseCtrl|CoordiCtrl_Query) != SingleRes.Ctrl || 0x00 != SingleRes.Data[0]){
				err_Print(DEBUG_single,"Query fail!\n");
				goto ERR;
			}
			repeat =0;
			AddrLen = 0;
			PAddr = (u16*)((char*)P_single + sizeof(Pag_Single));
		}
		++i;
	}

	Is_Group = Group;
	free(Single);
	debug(1,"Wait %d S\n",Maxlen*2/5);
	sleep(Maxlen*2/5);

	return SUCCESS;
 ERR:
 	Is_Group = -2;
	free(Single);
	return FAIL;
}

s32 Query_electric(const void *Addr,int Addr_size)
{
	assert_param(Addr,NULL,FAIL);
	typedef struct {u32 Addr;u32 Warn_flags;u32 Coor_Addr;u32 light_V;u32 light_E; } Sqlbuf_t;
	const Sqlbuf_t *pAddr = Addr;
	int Single_count = Addr_size/sizeof(Sqlbuf_t);
	u8* const Sendbuf = (u8*)malloc(Single_count*2 + 16);
	if(!Sendbuf) goto out;

	Sendbuf[3] = pAddr-> Coor_Addr;	//协调器地址
	int i =0,count = 0,single_count_max = 0,repeat = 3;
	u8 *pbuf = Sendbuf+12;
	while(i++ < Single_count){
		if(pAddr->Coor_Addr != Sendbuf[3]){
			Sendbuf[0] = Single_Header;
			Sendbuf[1] = CoordiCtrl_Qelec;
			Sendbuf[6] = 0x00;					//命令字
			Sendbuf[7] = 0x01;					//命令字
			Sendbuf[8] = 0xff&(count>>8);
			Sendbuf[9] = 0xff&count;
			Crc16(&Sendbuf[10],Sendbuf,10);
			Crc16(Sendbuf+12+count,Sendbuf+12,count);
			Display_package("^^^^^electric Query",Sendbuf,count+14);
			repeat = 3;
			while(repeat--){
				/* send data */
				UartBufClear(Uart1_ttyO1_485, Flush_Input);
				Uart_Send(Uart1_ttyO1_485, (s8*)Sendbuf, count+14, SingleSendTimeout);
				/* Recv response */
				if( SUCCESS !=Recv_Package_v2((Pag_Single*)Sendbuf,SingleRecvTimeout) )
					debug(DEBUG_single,"Wait Single Response Timeout!\n");
				else if(!Sendbuf[8] && !Sendbuf[9] ) break;
				else goto out;
				if(repeat <= 0) goto out;
			}
			/* update Coordinate */
			Sendbuf[3] = pAddr->Coor_Addr;
			pbuf = Sendbuf+12;
			count = 0;
		}
		*pbuf++ = 0xff&(pAddr->Addr >> 8);
		*pbuf++ = 0xff&pAddr->Addr;
		count += 2;  ++pAddr;
		if(single_count_max < count) single_count_max = count;

		if(i == Single_count){
			Sendbuf[0] = Single_Header;
			Sendbuf[1] = CoordiCtrl_Qelec;
			Sendbuf[6] = 0x00;					//命令字
			Sendbuf[7] = 0x01;					//命令字
			Sendbuf[8] = 0xff&(count>>8);
			Sendbuf[9] = 0xff&count;
			Crc16(&Sendbuf[10],Sendbuf,10);
			Crc16(Sendbuf+12+count,Sendbuf+12,count);
			Display_package("^^^^^electric Query",Sendbuf,count+14);
			repeat = 3;
			while(repeat--){
				/* send data */
				UartBufClear(Uart1_ttyO1_485, Flush_Input);
				Uart_Send(Uart1_ttyO1_485, (s8*)Sendbuf, count+14, SingleSendTimeout);
				/* Recv response */
				if( SUCCESS !=Recv_Package_v2((Pag_Single*)Sendbuf,SingleRecvTimeout) )
					debug(DEBUG_single,"Wait Single Response Timeout!\n");
				if(!Sendbuf[8] && !Sendbuf[9] ) break;
				if(repeat <= 0) goto out;
			}
		}
	}
	if(Sendbuf) free(Sendbuf);
	printf("Wait %d S\n",single_count_max/3);
	sleep(single_count_max/3);
	return SUCCESS;
out:
	if(Sendbuf) free(Sendbuf);
	return FAIL;
}


s32 SingleShortAckToServer(int cmd,u8 ctrl, u8 Resault,...)
{
	u8    AckShortbuf[32];
	va_list 	arg_ptr;
	va_start(arg_ptr, Resault);
	memset(AckShortbuf,0,sizeof(AckShortbuf));
	AckShortbuf[0] = 0x51;
	switch(cmd){
		case 0:AckShortbuf[1] = 0x04; AckShortbuf[2] = 0x80;AckShortbuf[3]=0x02;AckShortbuf[4]=ctrl;AckShortbuf[5]=Resault;break;
		case 1:AckShortbuf[1] = 0x03; AckShortbuf[2] = ctrl;AckShortbuf[3]=0x01;AckShortbuf[4]=Resault;break;
		case 2:AckShortbuf[1] = 0x04; AckShortbuf[2] = ctrl;AckShortbuf[3]=0x02;AckShortbuf[4]=Resault;AckShortbuf[5]=va_arg(arg_ptr, int);break;
		default:break;
	}va_end(arg_ptr);

	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
		return SUCCESS;
	}else{
		debug(DEBUG_single,"TASK_QUEUE_APPEND_ERR :%d\n",TASK_QUEUE_APPEND_ERR);
		return TASK_QUEUE_APPEND_ERR;
	}
}

/**
 *   立即回复到上位机
 */
s32 ResponsePromptly(u8 ctrl,u8 SubCMD ,u8 Resault,int Bufferlen)
{
	u8 *puc 	= NULL;
	u8 chk 		= 0;
	u32 len 	= 0, i = 0;
	faalpkt_t *pkt 	= malloc(Bufferlen);
	if(NULL == pkt ){
		err_Print(DEBUG_single,"malloc error!\n");
		return FAIL;
	}
	memset(pkt,0,Bufferlen);
	pkt->ctrl  	=  0x80;
	pkt->rtua[0]  	=  CCparamGlobalInfor.CCUID[0];
	pkt->rtua[1]  	=  CCparamGlobalInfor.CCUID[1];
	pkt->rtua[2]  	=  CCparamGlobalInfor.CCUID[2];
	pkt->rtua[3]  	=  CCparamGlobalInfor.CCUID[3];
	pkt->rtua[4]  	=  CCparamGlobalInfor.CCUID[4];
	pkt->rtua[5]  	=  CCparamGlobalInfor.CCUID[5];
	pkt->data[0]	=  ctrl;
 #ifdef ResponseSubCmd
	pkt->len 	=  0x03;
	pkt->head 	=  pkt->dep = FAAL_HEAD;
	pkt->data[1]	=  SubCMD;
	pkt->data[2]	=  Resault;
 #else
	pkt->len 	=  0x02;
	pkt->head 	=  pkt->dep = FAAL_HEAD;
	pkt->data[1]	=  Resault;
 #endif
	len 	= pkt->len + LEN_FAALHEAD;
	puc 	= &pkt->head;
	for(i=0; i<len; i++) chk += *puc++;
	*puc++ = chk;
	*puc 	= FAAL_TAIL;
	len 	+= LEN_FAALTAIL;
	Display_package("Response Promptly",pkt,len);
	i = Ether_RawSend((u8*)pkt, len);
	free(pkt);
	return i;
}

s32 SingleOpen(struct task_node *node)
{
	int Repeat = 3;
	Pag_Single Single;
	if(!node){ return FAIL; }		//判断指针是否可用
	while(Repeat--){
		memset(&Single,0,sizeof(Single));
		Single.Header 			= Single_Header;
		Single.Ctrl 				= SingleCtrl_Single;
		Single.Coordi_Addr 	= node->pakect[3];
		Single.Single_Addr[0]	= node->pakect[4];
		Single.Single_Addr[1]	= node->pakect[5];
		Single.Cmd[0]			= (signle_open>>8) & 0xff;
		Single.Cmd[1]			= signle_open 		& 0xff;

		#ifdef Config_PWM_
			Single.Data[0]	= (node->pakect[6] < PWMmax) ? PWMmax- node->pakect[6] :0;
		#else
			Single.Data[0]	= (node->pakect[6] > PWMmax) ? PWMmax:node->pakect[6];
		#endif
		Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);

		Display_package("Single Open Send data",&Single,sizeof(Pag_Single));

		Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

		#ifdef Config_Log
			Write_log(Coordinate, &Single);
		#endif

		UartBufClear(Uart1_ttyO1_485, Flush_Input);// 冲洗输入缓存
		memset(&Single,0,sizeof(Single));
		if( SUCCESS !=Recv_Package_v2(&Single,SingleRecvTimeout) ){//等待单灯回复，超过1s则超时。
			debug(DEBUG_single,"Wait Single Response Timeout!\n");
			continue;
			//goto ERR;
		}else{ break; }
	}	//end of while(Repeat--){
	if(Repeat <= 0){ return FAIL; }
	/* 是否要校验回复回来的单灯或协调器地址??? */
	if((ResponseCtrl|SingleCtrl_Single) != Single.Ctrl || 0x00 != Single.Data[0]){
		debug(DEBUG_single,"^^^^^single open fail!\n");
		goto ERR;
	}debug(DEBUG_single,"single open success!\n");
	Is_Group = -2;		//组播或广播需要重新查询
	return SUCCESS;
 ERR:
	return FAIL;
}

s32 GroupOpen(struct task_node *node)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));

	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Group;
	Single.Group_Addr 	= node->pakect[3];	//组号
	Single.Cmd[0]		= (signle_open>>8) 	& 0xff;
	Single.Cmd[1]		= signle_open 		& 0xff;
	#ifdef Config_PWM_
		Single.Data[0]	= (node->pakect[4] < PWMmax) ? PWMmax- node->pakect[4] :0;
	#else
		Single.Data[0]	= (node->pakect[4] > PWMmax) ? PWMmax:node->pakect[4];
	#endif
	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Group Open Send data",&Single,sizeof(Pag_Single));
	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

	ResponsePromptly(0x02,0x42 ,0,32);

	if(SUCCESS == Query_Action(Single.Group_Addr)){
		SingleShortAckToServer(1,  0x02,  0x42);
		return SUCCESS;
	}else{
		SingleShortAckToServer(1,  0x02,  0x43);
		return FAIL;
	}
}

s32 BroadcastOpen(struct task_node *node)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));
	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Broadcast;
	Single.Cmd[0]		= (signle_open>>8) 	& 0xff;
	Single.Cmd[1]		= signle_open 		& 0xff;
	#ifdef Config_PWM_
		Single.Data[0]	= (node->pakect[3] < PWMmax) ? PWMmax- node->pakect[3] :0;
	#else
		Single.Data[0]	= (node->pakect[3] > PWMmax) ? PWMmax : node->pakect[3];
	#endif

	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Broadcast Open Send data",&Single,sizeof(Pag_Single));

	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
	/* 回复广播命令发送成功 */
	ResponsePromptly(0x03,0x42 ,0,32);

	if(SUCCESS == Query_Action(-1)){
		SingleShortAckToServer(1,  0x03,  0x42);
		return SUCCESS;
	}else{
		SingleShortAckToServer(1,  0x03,  0x43);
		return FAIL;
	}
}

s32 GroupLight(struct task_node *node)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));

	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Group;
	Single.Group_Addr 	= node->pakect[3];	//组号
	Single.Cmd[0]		= (signle_open>>8) 	& 0xff;
	Single.Cmd[1]		= signle_open 		& 0xff;
	#ifdef Config_PWM_
		Single.Data[0]	= (node->pakect[4] < PWMmax) ? PWMmax- node->pakect[4] :0;
	#else
		Single.Data[0]	= (node->pakect[4] > PWMmax) ? PWMmax:node->pakect[4];
	#endif
	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Group Open Send data",&Single,sizeof(Pag_Single));
	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

	ResponsePromptly(0x02,0x47 ,0,32);

	if(SUCCESS == Query_Action(Single.Group_Addr)){
		SingleShortAckToServer(2,  0x02,  0x47,node->pakect[4]);
		return SUCCESS;
	}else{
		SingleShortAckToServer(2,  0x02,  0x47,0);
		return FAIL;
	}
}

s32 BroadcastLight(struct task_node *node)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));
	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Broadcast;
	Single.Cmd[0]		= (signle_open>>8) 	& 0xff;
	Single.Cmd[1]		= signle_open 		& 0xff;
	#ifdef Config_PWM_
		Single.Data[0]	= (node->pakect[3] < PWMmax) ? PWMmax- node->pakect[3] :0;
	#else
		Single.Data[0]	= (node->pakect[3] > PWMmax) ? PWMmax : node->pakect[3];
	#endif

	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Broadcast Open Send data",&Single,sizeof(Pag_Single));

	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
	/* 回复广播命令发送成功 */
	ResponsePromptly(0x03,0x47 ,0,32);

	if(SUCCESS == Query_Action(-1)){
		SingleShortAckToServer(2,  0x03,  0x47,node->pakect[3]);
		return SUCCESS;
	}else{
		SingleShortAckToServer(2,  0x03,  0x47,0);
		return FAIL;
	}
}

s32 SingleClose(struct task_node *node)
{
	int Repeat = 3;
	Pag_Single Single;
	if(!node){ return FAIL; }
	memset(&Single,0,sizeof(Single));
	while(Repeat--){
		Single.Header 		= Single_Header;
		Single.Ctrl 		= SingleCtrl_Single;
		Single.Coordi_Addr 	= node->pakect[3];
		Single.Single_Addr[0]	= node->pakect[4];
		Single.Single_Addr[1]	= node->pakect[5];
		Single.Cmd[0]		= (signle_close>>8) 	& 0xff;
		Single.Cmd[1]		= signle_close 		& 0xff;
		Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);

		Display_package("Single Close Send data",&Single,sizeof(Pag_Single));

		Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

		#ifdef Config_Log
			Write_log(Coordinate, &Single);
		#endif
		/* 冲洗输入缓存 */
		UartBufClear(Uart1_ttyO1_485, Flush_Input);
		memset(&Single,0,sizeof(Single));
		if( SUCCESS !=Recv_Package_v2(&Single,SingleRecvTimeout) ){//等待单灯回复，超过5s则超时。
			debug(DEBUG_single,"Wait Single Response Timeout!\n");
			//goto ERR;
			continue;
		}else{break;}
	}	//end of while(Reapeat--)
	if(Repeat <= 0){ return FAIL; }
	/* 是否要校验回复回来的单灯或协调器地址??? */
	if((ResponseCtrl|SingleCtrl_Single) != Single.Ctrl || 0x00 != Single.Data[0]){
		debug(DEBUG_single,"^^^^^single Close fail!\n");
		goto ERR;
	}debug(DEBUG_single,"single Close success!\n");
	Is_Group = -2;		//组播或广播需要重新查询
	return SUCCESS;
 ERR:
	return FAIL;
}

s32 GroupClose(struct task_node *node)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));

	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Group;
	Single.Group_Addr 	= node->pakect[3];	//组号
	Single.Cmd[0]		= (signle_close>>8) 	& 0xff;
	Single.Cmd[1]		= signle_close 		& 0xff;
	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Group Close Send data",&Single,sizeof(Pag_Single));
	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
	ResponsePromptly(0x02,0x43 ,0,32);

	if(SUCCESS == Query_Action(Single.Group_Addr)){
		SingleShortAckToServer(1,  0x02,  0x43);
		return SUCCESS;
	}else{
		SingleShortAckToServer(1,  0x02,  0x42);
		return FAIL;
	}
}
s32 BroadcastClose(struct task_node *node)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));

	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Broadcast;
	Single.Cmd[0]		= (signle_close>>8) 	& 0xff;
	Single.Cmd[1]		= signle_close 		& 0xff;
	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Group Close Send data",&Single,sizeof(Pag_Single));
	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
	/* 回复广播命令发送成功 */
	ResponsePromptly(0x03,0x43 ,0,32);

	if(SUCCESS == Query_Action(-1)){
		SingleShortAckToServer(1,  0x03,  0x43);
		return SUCCESS;
	}else{
		SingleShortAckToServer(1,  0x03,  0x42);
		return FAIL;
	}
}

s32 Single_Config(int cmd,void *PSingle)
{
	Pag_Single Single;
	memset(&Single,0,sizeof(Pag_Single));
	switch(cmd){
		case Single_ConfigMapAddr:
			Single.Header 		= Single_Header;
			Single.Ctrl		= SingleCtrl_Single;
			Single.Coordi_Addr 	= ((TableSingle_t*)PSingle)->Coor_id;
			Single.Single_Addr[0] 	= ((TableSingle_t*)PSingle)->Base_Addr >>8 	&0xff;
			Single.Single_Addr[1] 	= ((TableSingle_t*)PSingle)->Base_Addr 	&0xff;
			Single.Cmd[0] 		= (single_MapAddr >>8) 			&0xff;
			Single.Cmd[1] 		= single_MapAddr 				&0xff;
			Single.Data[0]		= ((TableSingle_t*)PSingle)->Map_Addr;
			Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
			Display_package("Single Config MapAddr Send data",&Single,sizeof(Pag_Single));
			UartBufClear(Uart1_ttyO1_485, Flush_Input|Flush_Output);
			Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
			break;
		case Single_ConfigGroup:
			Single.Header 		= Single_Header;
			Single.Ctrl		= SingleCtrl_Single;
			Single.Coordi_Addr 	= ((TableSingle_t*)PSingle)->Coor_id;
			Single.Single_Addr[0] 	= ((TableSingle_t*)PSingle)->Base_Addr >>8 	&0xff;
			Single.Single_Addr[1] 	= ((TableSingle_t*)PSingle)->Base_Addr 	&0xff;
			Single.Cmd[0] 		= (single_Group >>8) 				&0xff;
			Single.Cmd[1] 		= single_Group					&0xff;
			Single.Data[0]		= ((TableSingle_t*)PSingle)->lt_gid;
			Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
			Display_package("Single Config Group Send data",&Single,sizeof(Pag_Single));
			UartBufClear(Uart1_ttyO1_485, Flush_Input|Flush_Output);
			Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
			break;
		case Coordi_ConfigMapAddr:
			Single.Header 		= Single_Header;
			Single.Ctrl		= CoordiCtrl_Config;
			Single.Coordi_Addr 	= ((TableCoordi_t*)PSingle)->Base_Addr;
			Single.Cmd[0] 		= (single_MapAddr >>8) 	&0xff;
			Single.Cmd[1] 		= single_MapAddr		&0xff;
			Single.Data[0]		= ((TableCoordi_t*)PSingle)->Map_Addr;
			Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
			Display_package("Coordinate Config MapAddr Send data",&Single,sizeof(Pag_Single));
			UartBufClear(Uart1_ttyO1_485, Flush_Input|Flush_Output);
			Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
			break;
		case Coordi_ConfigGroup:
		default:break;
	}
	/* 写日志报告 */
	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif

	memset(&Single,0,sizeof(Single));
	if( SUCCESS != Recv_Package_v2(&Single,50)){
		debug(DEBUG_single,"Wait Single Response Timeout!\n");
		goto ERR;
	}
	if( 0x00 != Single.Data[0] ){
		debug(DEBUG_single,"^^^^^Config MapAddr Or Group fail!\n");
		goto ERR;
	}
	return SUCCESS;
 ERR:
 	return FAIL;
}

s32 Coordinator_Update(struct task_node *node)
{
	//pthread_t thread;
 #ifdef Config_NewProtocol
	Pag_Single sig;
	memset(&sig,0,sizeof(sig));
	sig.Header 			= 0xff;
	sig.Coordi_Addr 	= node->pakect[3];
	sig.Ctrl 			= CtrlCode_Coordi;
	sig.Cmd[0] 			= (0x80>>8)&0xff;
	sig.Cmd[1] 			= 0x80&0xff;
	sig.Data[0]			=sig.Data[1] = !0;
	Crc16(sig.Crc16,(u8*)&sig, sizeof(sig)-2);
	Display_package("#####Send data",&sig,sizeof(sig));
	Uart_Send(Uart1_ttyO1_485, (s8*)&sig, sizeof(sig), SingleSendTimeout);
	#ifdef Config_Log
		Write_log(Coordinate, &sig);
	#endif
	UartBufClear(Uart1_ttyO1_485, Flush_Input);
	memset(&sig,0,sizeof(sig));
	if( !DeviceRecv485_v2(Uart1_ttyO1_485,(s8*)&sig,sizeof(sig),3000000) ){
		Display_package("#####Recv data",&sig,sizeof(sig));
	}else{
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		/* 设置相关状态 */
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Wait Single Response Time Out!");	//写错误信息
		#endif
		goto err;
	}
	/* 校验接收到的数据 */
	if( CHK_Crc16(sig.Crc16,(u8*)&sig,sizeof(sig)-2)){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Package CRC Check Error!");	//写错误信息
		#endif
	}else{
		#ifdef Config_Log
			Write_log(Coordinate, &sig);
		#endif
		//return pthread_create(&thread, NULL,Updatethread, &sig) == 0?sleep(1),SUCCESS:FAIL;
	}
 #else
	Frame_485 sig;
	memset(&sig,0,sizeof(sig));
	sig.slave_addr 	= node->pakect[3];
	sig.ctrl 			= CtrlCode_Single;
	sig.cmd_h 		= (0x80>>8)&0xff;
	sig.cmd_l 		= 0x80&0xff;
	sig.light_level	= !0;
	Crc16(&sig.crc16_l,(u8*)&sig, sizeof(sig)-2);
	Display_package("#####Send data",&sig,sizeof(sig));
	Uart_Send(Uart1_ttyO1_485, (s8*)&sig, sizeof(Frame_485), SingleSendTimeout);
	#ifdef Config_Log
		Write_log(Coordinate, &sig);
	#endif
	UartBufClear(Uart1_ttyO1_485, Flush_Input);
	memset(&sig,0,sizeof(sig));
	if( !DeviceRecv485_v2(Uart1_ttyO1_485,(s8*)&sig,10,3000000) ){
		Display_package("##### Uart1 Recv data",&sig,sizeof(sig));
	}else{
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		/* 设置相关状态 */
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Wait Single Response Time Out!");	//写错误信息
		#endif
		goto err;
	}
	/* 校验接收到的数据 */
	if( CHK_Crc16(&sig.crc16_l,(u8*)&sig,sizeof(Frame_485)-2)){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Package CRC Check Error!");	//写错误信息
		#endif
	}else{
		#ifdef Config_Log
			Write_log(Coordinate, &sig);
		#endif
		//return pthread_create(&thread, NULL,Updatethread, &sig) == 0?sleep(1),SUCCESS:FAIL;
	}
 #endif
	return UpdateThread(&sig);
 err:
	return FAIL;
}

s32 Single_Update(struct task_node *node)
{
	//pthread_t thread;
 #ifdef Config_NewProtocol
	Pag_Single sig;
	memset(&sig,0,sizeof(sig));
	sig.Header 		= 0xff;
	sig.Coordi_Addr 	= node->pakect[3];
	sig.Single_Addr[0] 	= node->pakect[4];
	sig.Single_Addr[1] 	= node->pakect[5];
	sig.Ctrl 		= CtrlCode_Single;
	sig.Cmd[0] 		= (0x80>>8)&0xff;
	sig.Cmd[1] 		= 0x80&0xff;
	sig.Data[0]=sig.Data[1] 	= 0;
	Crc16(sig.Crc16,(u8*)&sig, sizeof(sig)-2);
	Display_package("#####Send data",&sig,sizeof(sig));
	UartBufClear(Uart1_ttyO1_485, Flush_Input);
	Uart_Send(Uart1_ttyO1_485, (s8*)&sig, sizeof(sig), SingleSendTimeout);
	#ifdef Config_Log
		Write_log(Coordinate, &sig);
	#endif
	memset(&sig,0,sizeof(sig));
	if( !DeviceRecv485_v2(Uart1_ttyO1_485,(s8*)&sig,sizeof(sig),3000000) ){
		Display_package("#####Recv data",&sig,sizeof(sig));
	}else{
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		/* 设置相关状态 */
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Wait Single Response Time Out!");	//写错误信息
		#endif
		goto err;
	}
	/* 校验接收到的数据 */
	if( CHK_Crc16(sig.Crc16,(u8*)&sig,sizeof(sig)-2)){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Package CRC Check Error!");	//写错误信息
		#endif
	}else{
		#ifdef Config_Log
			Write_log(Coordinate, &sig);
		#endif
		//return pthread_create(&thread, NULL,Updatethread, &sig) == 0?sleep(1),SUCCESS:FAIL;
		//return SUCCESS;
	}
 #else
	Frame_485 sig;
	memset(&sig,0,sizeof(sig));
	sig.slave_addr 	     = node->pakect[3];
	sig.single_addrH    = node->pakect[4];
	sig.single_addrL     = node->pakect[5];
	sig.ctrl                      = CtrlCode_Single;
	sig.cmd_h                = (0x80>>8)&0xff;
	sig.cmd_l                 = 0x80&0xff;
	sig.light_level          = 0;
	Crc16(&sig.crc16_l,(u8*)&sig, sizeof(sig)-2);
	Display_package("#####Uart1 Send data",&sig,sizeof(sig));
	Uart_Send(Uart1_ttyO1_485, (s8*)&sig, sizeof(Frame_485), SingleSendTimeout);
	#ifdef Config_Log
		Write_log(Coordinate, &sig);
	#endif
	UartBufClear(Uart1_ttyO1_485, Flush_Input);
	memset(&sig,0,sizeof(sig));
	if( !DeviceRecv485_v2(Uart1_ttyO1_485, (char*)&sig,  sizeof(sig), 6000000)){
		Display_package("#####Uart1 Recv data",&sig,sizeof(sig));
	}else{
		debug(DEBUG_single,">>>>>Can not recv from Uart1\n");
		/* 设置相关状态 */
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Wait Single Response Time Out!");	//写错误信息
		#endif
		goto err;
	}
	/* 校验接收到的数据 */
	if( CHK_Crc16(&sig.crc16_l,(u8*)&sig,sizeof(Frame_485)-2)){
		debug(DEBUG_single,">>>>>Crc16 check err!\n");
		#ifdef Config_Log
			Write_log(Coordinate|Log_err, "Package CRC Check Error!");	//写错误信息
		#endif
	}else{
		#ifdef Config_Log
			Write_log(Coordinate, &sig);
		#endif

		//return pthread_create(&thread, NULL,Updatethread, &sig ) == 0?sleep(1),SUCCESS:FAIL;
	}
 #endif
	return UpdateThread(&sig);
 err:
	return FAIL;
}

s32 SingleQuery(struct task_node *node)
{
	u8 ACKSendBufLong[24];
	Pag_Single Single;
	memset(&Single,0,sizeof(Single));
	Single.Header 		= Single_Header;
	Single.Ctrl 		= SingleCtrl_Single;
	Single.Coordi_Addr 	= node->pakect[3];
	Single.Single_Addr[0]	= node->pakect[4];
	Single.Single_Addr[1]	= node->pakect[5];
	Single.Cmd[0]		= (Single_Query>>8) 	& 0xff;
	Single.Cmd[1]		= Single_Query 	& 0xff;

	Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
	Display_package("Single Query Send data",&Single,sizeof(Pag_Single));
	UartBufClear(Uart1_ttyO1_485, Flush_Input);// 冲洗输入缓存
	Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

	#ifdef Config_Log
		Write_log(Coordinate, &Single);
	#endif

	memset(&Single,0,sizeof(Single));
	memset(ACKSendBufLong,0,sizeof(ACKSendBufLong));
	ACKSendBufLong[11] = SUCCESS;

	if( SUCCESS !=Recv_Package_v2(&Single,SingleRecvTimeout) ){//等待单灯回复，超过5s则超时。
		debug(DEBUG_single,"Wait Single Response Timeout!\n");
		ACKSendBufLong[11] = FAIL;
	}
	if((ResponseCtrl|SingleCtrl_Single) != Single.Ctrl ){
		debug(DEBUG_single,"single Query fail!\n");
		ACKSendBufLong[11] = FAIL;
	}else{
		debug(DEBUG_single,"single Query Success!\n");
	}
	/* 回复服务器 */
	ACKSendBufLong[0] = 0x51; ACKSendBufLong[1] = 0x0A; ACKSendBufLong[2] = 0x80;
	ACKSendBufLong[3] = 0x08; ACKSendBufLong[4] = 0x01; ACKSendBufLong[5] = 0x45;
	ACKSendBufLong[6] = node->pakect[3]; ACKSendBufLong[7] = node->pakect[4];
	ACKSendBufLong[8] = node->pakect[5]; ACKSendBufLong[9] = Single.Data[0];

	#ifdef Config_PWM_
		ACKSendBufLong[10] = (Single.Data[1]<PWMmax)?PWMmax-Single.Data[1]:0;
	#else
		ACKSendBufLong[10] = (Single.Data[1]>PWMmax)?PWMmax:Single.Data[1];
	#endif
	return TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,ACKSendBufLong,NET_TASK,TASK_LEVEL_NET);
}

int SingleList_Index(int SingleAddr)
{
	int i = 0;
	int GsingleAddr = 0;
	while(SingleList[i].Coordi_Addr != 0){
		GsingleAddr = SingleList[i].Single_Addr[0]<<8 | SingleList[i].Single_Addr[1];
		if(GsingleAddr == SingleAddr ){
			return i;
		}
		++i;
	}
	return -1;
}

static void InsertState2Table(u8 *Plist,int nLen)
{
	u8 *PPlist = Plist;
	int SingleAddr = 0;
	int index = 0;
	while((PPlist - Plist) < nLen){
		SingleAddr = PPlist[0]<<8 | PPlist[1];
		index = SingleList_Index(SingleAddr);
		if(index >= 0){
			SingleList[index].Single_State = PPlist[2];
			#ifdef Config_PWM_
				SingleList[index].Single_Light = (PPlist[3]<PWMmax)?PWMmax-PPlist[3]:0;
				if(PPlist[3] == 0xfe){
					SingleList[index].Single_Light = PPlist[3];
				}
			#else
				SingleList[index].Single_Light = (PPlist[3]>PWMmax)?PWMmax:PPlist[3];
				if(PPlist[3] == 0xfe){
					SingleList[index].Single_Light = PPlist[3];
				}
			#endif
		}
		PPlist += 4;
	}
}

static s32 QueryRes(u8 Ctrl,u8 Resault)
{
	int num = 0,i = 0;
	u8    AckShortbuf[300];
	u8 *PAck = NULL;
	memset(AckShortbuf,0,sizeof(AckShortbuf));
	AckShortbuf[0] = 0x52;
	AckShortbuf[2] = Ctrl;
	AckShortbuf[4] = 0x45;
	if(Resault != SUCCESS){
		AckShortbuf[0] = 0x51;
		AckShortbuf[1] = 0x04;
		AckShortbuf[3] = 0x02;
		AckShortbuf[5] = Resault;
		if(TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
			err_Print(DEBUG_single,"Append to queue fail!\n");
			return FAIL;
		}
		return SUCCESS;
	}

	PAck = &AckShortbuf[5];
	while(SingleList[i].Coordi_Addr != 0 && i< SingleNumber ){
		*PAck++ = SingleList[i].Single_Light;
		*PAck++ = SingleList[i].Single_State;
		*PAck++ = SingleList[i].Single_Addr[0];
		*PAck++ = SingleList[i].Single_Addr[1];
		++i;
		if(++num >= 50){
			*PAck = Resault;
			AckShortbuf[3] = num*4 + 2;//数据的长度
			AckShortbuf[1] = num*4 + 4;//数据的长度
			num = 0;
			if(TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
				err_Print(DEBUG_single,"Append to queue fail!\n");
			}
			PAck = &AckShortbuf[5];
		}
		//sleep(1);
	}
	if(num >0){
		*PAck = Resault;
		AckShortbuf[3] = num*4 + 2;//数据的长度
		AckShortbuf[1] = num*4 + 4;//数据的长度
		if(TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
			err_Print(DEBUG_single,"Append to queue fail!\n");
			return FAIL;
		}
	}
	return SUCCESS;
}

void Display_Singlelist(void)
{
	int i = 0;
	u8 CoordiAddr = SingleList[i].Coordi_Addr;
	if(CoordiAddr){
		debug(DEBUG_single,"CoordiAddr:0x%02x\n\t",CoordiAddr);
	}
	while(SingleList[i].Coordi_Addr != 0){
		if(SingleList[i].Coordi_Addr != CoordiAddr ){
			CoordiAddr = SingleList[i].Coordi_Addr;
			debug(DEBUG_single,"\nCoordiAddr:0x%02x\n\t",CoordiAddr);
		}
		debug(DEBUG_single,"%02x%02x  ",SingleList[i].Single_Addr[0],SingleList[i].Single_Addr[1]);
		++i;
	}
	debug(DEBUG_single,"\n");
}

void GetCoordiAddrList(void *Buffer,int len)
{
	//u8 *CoordiAddr = Buffer;
	typedef  struct{int Singlelen; u8 CoorAddr;} CoordiAddr_t;
	CoordiAddr_t *CoordiAddr  = Buffer;
	 int i = 0;
	while(SingleList[i].Coordi_Addr != 0){
		CoordiAddr->CoorAddr = SingleList[i++].Coordi_Addr;
		CoordiAddr->Singlelen = i;
		if(CoordiAddr->CoorAddr != SingleList[i].Coordi_Addr){
			++CoordiAddr;
			if(CoordiAddr - ( CoordiAddr_t*)Buffer >= len){
				break;
			}
		}
	}
	while(CoordiAddr != Buffer){
		CoordiAddr->Singlelen -= (CoordiAddr-1)->Singlelen;
		--CoordiAddr;
	}



	CoordiAddr = Buffer;
	 debug(DEBUG_single,"CoordiAddr List:");
	 while(CoordiAddr->CoorAddr != 0){
		debug(DEBUG_single,"Addr:0x%02x,len:%d\t",CoordiAddr->CoorAddr,CoordiAddr->Singlelen);
		CoordiAddr++;
	 }debug(DEBUG_single,"\n");
}

s32 CalcWaitTime(void)
{
	int SingleCnt = 0;
	int i = 0,j=0;
	while(SingleList[i++].Coordi_Addr != 0){
		if(SingleList[i].Coordi_Addr != SingleList[i-1].Coordi_Addr ){
			SingleCnt = (i-j >SingleCnt)?(i-j):SingleCnt;
			j = i;
		}
	}
	debug(DEBUG_single,"Max SingleCnt=%d\n",SingleCnt);
	return SingleCnt;
}

s32 Instert_electric(void *dest,int dest_size,void *src,int src_size)
{
	assert_param(dest,NULL,FAIL);
	assert_param(src,NULL,FAIL);
	typedef struct {u32 Addr;u32 Warn_flags;u32 Coor_Addr;u32 light_V;u32 light_E; } Sqlbuf_t;
	typedef struct {u16 Addr;u16 light_V;u16 light_E; } Recvbuf_t;

	Recvbuf_t *const psrc = src;
	Sqlbuf_t *pdest_start = dest;
	Sqlbuf_t *pdest_end  = pdest_start + dest_size/sizeof(Sqlbuf_t) - 1;
	int i = 0, src_count = src_size/sizeof(Recvbuf_t);
	while(i < src_count){
		u32 comp = bigend2littlend_2(psrc[i].Addr);
		pdest_start = dest;
		pdest_end  = pdest_start + dest_size/sizeof(Sqlbuf_t) - 1;
		while(pdest_start <= pdest_end){
			if(pdest_start->Addr == comp){
				pdest_start->light_V = bigend2littlend_2(psrc[i].light_V);
				pdest_start->light_E = bigend2littlend_2(psrc[i].light_E);
				//debug(DEBUG_single,"SingleAddr:0x%04x,Voltage:%04x, electric:%04x\n",
				//								comp,pdest_start->light_V,pdest_start->light_E);
				/* set flags */
				break;
			}else if(pdest_end->Addr == comp){
				pdest_end->light_V = bigend2littlend_2(psrc[i].light_V);
				pdest_end->light_E = bigend2littlend_2(psrc[i].light_E);
				//debug(DEBUG_single,"SingleAddr:0x%04x,Voltage:%04x, electric:%04x\n",
				//								comp,pdest_end->light_V,pdest_end->light_E);
				/* set flags */
				break;
			}
			pdest_start++;
			pdest_end--;
		}
		++i;
	}
	return SUCCESS;
}



s32 electric_response(void *dest,int dest_size,int Ctrl,int Is_Success)
{
	if(NULL == dest) Is_Success =FAIL;

	typedef struct {u32 Addr;u32 Warn_flags;u32 Coor_Addr;u32 light_V;u32 light_E; } Sqlbuf_t;

	u8 AckShortbuf[300];
	memset(AckShortbuf,0,sizeof(AckShortbuf));
	AckShortbuf[0] = 0x52;
	AckShortbuf[2] = Ctrl;
	AckShortbuf[4] = 0x46;
	if(Is_Success != SUCCESS ){
		AckShortbuf[0] = 0x51;
		AckShortbuf[1] = 0x04;
		AckShortbuf[3] = 0x02;
		AckShortbuf[5] = Is_Success;
		if(TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
			err_Print(DEBUG_single,"Append to queue fail!\n");
			return FAIL;
		}
		return SUCCESS;
	}
	u8 *pAckbuf = AckShortbuf + 5;
	int i = 0,  single_count = dest_size/sizeof(Sqlbuf_t),num = 0;
	Sqlbuf_t *psingle_info = dest;
	while( i++ < single_count){
		//printf("addr:%04x,vol:%04x,elec:%04x\n",psingle_info->Addr,psingle_info->light_V,psingle_info->light_E);
		*pAckbuf++ = 0xff &(psingle_info->light_V>>8);
		*pAckbuf++ = 0xff &psingle_info->light_V;
		*pAckbuf++ = 0xff &(psingle_info->light_E>>8);
		*pAckbuf++ = 0xff &psingle_info->light_E;
		*pAckbuf++ = 0xff &(psingle_info->Addr>>8);
		*pAckbuf++ = 0xff &psingle_info->Addr;
		++psingle_info;
		if(++num >= 50){
			*pAckbuf = Is_Success;
			AckShortbuf[3] = num*6 + 2;//数据的长度
			AckShortbuf[1] = num*6 + 4;//数据的长度
			num = 0;
			if(TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
				err_Print(DEBUG_single,"Append to queue fail!\n");
			}
			pAckbuf = AckShortbuf + 5;
		}	//end of if(++num >= 50){
	}	//end of while( i++ < single_count){
	if(num > 0){
		*pAckbuf = Is_Success;
		AckShortbuf[3] = num*6 + 2;//数据的长度
		AckShortbuf[1] = num*6 + 4;//数据的长度
		if(TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,AckShortbuf,NET_TASK,TASK_LEVEL_NET)){
			err_Print(DEBUG_single,"Append to queue fail!\n");
			return FAIL;
		}
	}
	/* insert data to sqlite */
	psingle_info = dest;
	single_count = dest_size/sizeof(Sqlbuf_t);
	time_t tm = 0;
	time(&tm);		//get current time
	u32 power = 0;
	for(i=0;i<single_count;++i){
		power = psingle_info[i].light_V * psingle_info[i].light_E/10000;
		Update_Table_v2("db_info_light",Asprintf("set Warn_flags=%d,light_V=%d,light_E=%d,light_P=%u,rtime=%ld where Base_Addr"\
				"=0x%04x",psingle_info[i].Warn_flags,psingle_info[i].light_V,psingle_info[i].light_E,power,tm,psingle_info[i].Addr));
	}
	return SUCCESS;
}

s32 GroupQuery(struct task_node *node)
{
	assert_param(node,NULL,FAIL);

	Pag_Single Single;
	struct{int Singlelen; u8 CoorAddr;} CoordiAddr[20];
	u8 ResSingle[300];
	int i = 0;
	int Addrlen = 0;
	int Res = SUCCESS;
	int repeat = 0;
	Pag_Single *PRes = (Pag_Single*)ResSingle;

	memset(ResSingle,0,sizeof(ResSingle));
	memset(CoordiAddr,0,sizeof(CoordiAddr));
	memset(&Single,0,sizeof(Single));

	ResponsePromptly(0x02,0x45 ,Res,32);
	debug(DEBUG_single,"Group=%d\n",node->pakect[3]);
	if(Is_Group != node->pakect[3]){//判断之前查询的是否为现在要获取的组
		Res = Query_Action(node->pakect[3]);
		Display_Singlelist();
	}	if(SUCCESS != Res ) goto ERR;

	GetCoordiAddrList(CoordiAddr,sizeof(CoordiAddr));

	Single.Header 		= Single_Header;
	Single.Ctrl 			= CoordiCtrl_GetData;
	Single.Cmd[0]		= 0x00;
	Single.Cmd[1]		= 0x01;
	Single.Data[0]		= GetSingleCunt;
	i = 0;
	while(CoordiAddr[i].CoorAddr != 0 && i < sizeof(CoordiAddr)/sizeof(CoordiAddr[0]) ){
		Single.Coordi_Addr 	= CoordiAddr[i].CoorAddr;
		Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
		Display_package("Group Query Send data",&Single,sizeof(Pag_Single));
 Sendagina:
		UartBufClear(Uart1_ttyO1_485, Flush_Input);// 冲洗输入缓存
		Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

		if( SUCCESS !=Recv_Package_v2(PRes,SingleRecvTimeout) ){//等待单灯回复，超过3s则超时。
			debug(DEBUG_single,"Wait Single Response Timeout!\n");
			if(repeat++ < 5){    goto Sendagina;    }
			goto ERR;
		}else{    repeat = 0;    }

		Addrlen = PRes->Data[0] << 8 | PRes->Data[1];
		if( SUCCESS == DeviceRecv485_v2(Uart1_ttyO1_485,  (s8*)&ResSingle[sizeof(Pag_Single)] , Addrlen + 2, 5000000) ){
			if(CHK_Crc16((u8*)&ResSingle[sizeof(Pag_Single)] + Addrlen,(u8*)&ResSingle[sizeof(Pag_Single)] ,Addrlen) ){
				debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
				goto ERR;
			}
		}
		debug(DEBUG_single,"Addrlen=%d\n",Addrlen);
		Display_package("Recv Single State",&ResSingle[sizeof(Pag_Single)] ,Addrlen);
		//InsertState2Sql();		//把查询到的灯的状态写入数据库中去
		InsertState2Table(&ResSingle[sizeof(Pag_Single)],Addrlen);	//把查询到的灯的状态拷贝到全局变量中
		if(Addrlen/4 >= CoordiAddr[i].Singlelen){		//判断一个协调器下的数据是否全部取完
			++i;
		}else{    CoordiAddr[i].Singlelen -= Addrlen/4 ;    }
	}
	/* 回复给上位机 */
	return QueryRes(0x02,SUCCESS);
 ERR:
 	return QueryRes(0x02,FAIL);
}

s32 Broadcas_electric(u32 flags)
{
	typedef struct {u32 Addr;u32 Warn_flags;u32 Coor_Addr;u32 light_V;u32 light_E; } Sqlbuf_t;
	typedef struct {u16 Addr;u16 light_V;u16 light_E; } Recvbuf_t;
	typedef struct {u32 Coor_Addr;u32 single_Count;} CoordiAddr_t;

	Pag_Single Single;
	memset(&Single,0,sizeof(Pag_Single));

	Sqlbuf_t *light_info = NULL;
	Recvbuf_t *Recvbuf = NULL;
	CoordiAddr_t *CoordiAddr = NULL;
	int single_count_max = 0;
	int lightinfo_size = Get_light_info_count("Base_Addr");
	int Coordinate_count = Get_CountByColumn("db_light","distinct Coor_id");
	if(lightinfo_size <= 0 || Coordinate_count <= 0) goto out;
	/* malloc for Coordinator list */
	CoordiAddr = malloc(Coordinate_count*sizeof(CoordiAddr_t));
	if(!CoordiAddr) goto out;
	/* get coordinator list */
	if(SUCCESS != Select_Table_V2( Asprintf("select distinct Coor_id from db_light;"),
							(char*)CoordiAddr,sizeof(CoordiAddr_t),Coordinate_count,0)){
		debug(DEBUG_single,"get coordinator Addr list fail!\n");
		goto out;
	}
	/* get single count in the same coordinator */
	int i = 0;
	while(i < Coordinate_count){
		char str[32];	memset(str,0,sizeof(str));
		sprintf(str,"where Coor_id=%d",CoordiAddr[i].Coor_Addr);
		CoordiAddr[i].single_Count = Get_CountByCondition("db_light","Base_Addr",str);
		if(single_count_max < CoordiAddr[i].single_Count) single_count_max = CoordiAddr[i].single_Count;
		++i;
	}
	light_info = malloc(lightinfo_size*sizeof(Sqlbuf_t));
	Recvbuf = malloc(single_count_max*sizeof(Recvbuf_t) + 2);
	if(!light_info || !Recvbuf ) goto out;

	/* get data from db_info_light */
	if( SUCCESS != Select_Table_V2( Asprintf("select db_info_light.Base_Addr,db_info_light.Warn_flags,"\
							"db_light.Coor_id from db_info_light,db_light where db_info_light.Base_Addr = "\
										"db_light.Base_Addr;"),(char*)light_info,sizeof(Sqlbuf_t),lightinfo_size,0)){
		debug(DEBUG_single,"get light info fail!\n");
		goto out;
	}
	/* first response */
	ResponsePromptly(0x03,0x46 ,SUCCESS,32);
	if( SUCCESS != Query_electric(light_info,lightinfo_size*sizeof(Sqlbuf_t))){
		debug(DEBUG_single,"Query_electric error!\n");
		goto out;
	}
	/* get the electric from single */
	for(i=0;i<Coordinate_count;){
		Single.Header 		= Single_Header;
		Single.Ctrl 			= CoordiCtrl_electric;
		Single.Coordi_Addr= CoordiAddr[i].Coor_Addr;
		Single.Cmd[0]		= 0x00;
		Single.Cmd[1]		= 0x01;
		Single.Data[0]		= GetSingleCunt;
		Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
		Display_package("Get current send data",&Single,sizeof(Pag_Single));
		for(int repeat=3;repeat-- > 0;){
			/* send data */
			UartBufClear(Uart1_ttyO1_485, Flush_Input);
			Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);
			/* Recv data */
			if( SUCCESS !=Recv_Package_v2(&Single,SingleRecvTimeout) ){
				debug(DEBUG_single,"Wait Single Response Timeout!\n");
			}else break;
			if(repeat <= 0) goto out;
		}
		int Responselen = Single.Data[0]<<8|Single.Data[1];
		if( Responselen > 0 && SUCCESS == DeviceRecv485_v2(Uart1_ttyO1_485,  (s8*)Recvbuf , Responselen + 2, 3000000) ){
			if(CHK_Crc16((u8*)Recvbuf+Responselen,(u8*)Recvbuf ,Responselen) ){
				debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
				goto out;
			}Display_package("Recv state",Recvbuf,Responselen+2);
			/* instert status to sql */
			Instert_electric(light_info,lightinfo_size*sizeof(Sqlbuf_t),Recvbuf,Responselen);
		}else debug(DEBUG_single,"Responselen <=0 or DeviceRecv485_v2 fail!\n");
		/* is take out of coordinator */
		if(Responselen/6 >= CoordiAddr[i].single_Count){
			++i;
		}else  CoordiAddr[i].single_Count -=  Responselen/6;
	}
	/* Response Wincc */
	electric_response(light_info,lightinfo_size*sizeof(Sqlbuf_t),0x03,SUCCESS);
	if(CoordiAddr) free(CoordiAddr);
	if(light_info) free(light_info);
	if(Recvbuf) free(Recvbuf);
	return SUCCESS;
out:
	electric_response(NULL,0,0x03,FAIL);
	if(CoordiAddr) free(CoordiAddr);
	if(light_info) free(light_info);
	if(Recvbuf) free(Recvbuf);
	return FAIL;
}

s32 BroadcastQuery(struct task_node *node)
{
	Pag_Single Single;
	struct{int Singlelen; u8 CoorAddr;} CoordiAddr[20];
	u8 ResSingle[300];
	int i = 0;
	int Addrlen = 0;
	//int WaitTime = 0;
	int Res = SUCCESS;
	Pag_Single *PRes = (Pag_Single*)ResSingle;
	int repeat =0;
	memset(ResSingle,0,sizeof(ResSingle));
	memset(CoordiAddr,0,sizeof(CoordiAddr));
	memset(&Single,0,sizeof(Single));

	ResponsePromptly(0x03,0x45 ,Res,32);
	if(Is_Group != -1){		//判断之前查询的是否为现在要获取的组
		debug(1,"Is_Group = %d\n",Is_Group);
		Res = Query_Action(-1);
		Display_Singlelist();
		//WaitTime = CalcWaitTime();
	}
	if(SUCCESS != Res ){ goto ERR;}
	//debug(1,"wait %d S\n",WaitTime/2);
	//sleep(WaitTime/2);
	GetCoordiAddrList(CoordiAddr,sizeof(CoordiAddr));

	Single.Header 		= Single_Header;
	Single.Ctrl 			= CoordiCtrl_GetData;
	Single.Cmd[0]		= 0x00;
	Single.Cmd[1]		= 0x01;
	Single.Data[0]		= GetSingleCunt;
	i = 0;
	while(CoordiAddr[i].CoorAddr != 0 && i < sizeof(CoordiAddr)/sizeof(CoordiAddr[0]) ){
		Single.Coordi_Addr 	= CoordiAddr[i].CoorAddr;
		Crc16(Single.Crc16,(u8*)&Single, sizeof(Single)-2);
		Display_package("Broadcast Query Send data",&Single,sizeof(Pag_Single));
 Sendagina:
		UartBufClear(Uart1_ttyO1_485, Flush_Input);// 冲洗输入缓存
		Uart_Send(Uart1_ttyO1_485, (s8*)&Single, sizeof(Single), SingleSendTimeout);

		if( SUCCESS !=Recv_Package_v2(PRes,SingleRecvTimeout) ){//等待单灯回复，超过10s则超时。
			debug(DEBUG_single,"Wait Single Response Timeout!\n");
			if(repeat++ < 5){  goto Sendagina; }
			goto ERR;
		}else{ repeat = 0; }

		Addrlen = PRes->Data[0] << 8 | PRes->Data[1];
		if( SUCCESS == DeviceRecv485_v2(Uart1_ttyO1_485,  (s8*)&ResSingle[sizeof(Pag_Single)] , Addrlen + 2, 3000000) ){
			if(CHK_Crc16((u8*)&ResSingle[sizeof(Pag_Single)] + Addrlen,(u8*)&ResSingle[sizeof(Pag_Single)] ,Addrlen) ){
				debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
				Display_package("Recv state",&ResSingle[sizeof(Pag_Single)],Addrlen+2);
				goto ERR;
			}
		}
		debug(DEBUG_single,"Addrlen=%d\n",Addrlen);
		Display_package("Recv Single State",&ResSingle[sizeof(Pag_Single)] ,Addrlen);
		/* 把灯的状态拷贝到全局去 */
		InsertState2Table(&ResSingle[sizeof(Pag_Single)],Addrlen);
		if(Addrlen/4 >= CoordiAddr[i].Singlelen){		//判断一个协调器下的数据是否全部取完
			++i;											//获取下一个协调器的单灯状态
		}else{    CoordiAddr[i].Singlelen -=  Addrlen/4;    }
	}
	/* 回复给上位机 */
	return QueryRes(0x03,SUCCESS);
 ERR:
 	return QueryRes(0x03,FAIL);
}
