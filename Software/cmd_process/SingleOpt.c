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

#if 0
/**
 * 查询组播/广播动作是否都执行了
 */
s32 Check_Action(int cmd,...)
{
	va_list 	arg_ptr;
	int 	group = 0;
	char 	sql[128];
	u8 	Single[2048];
	int 	i = 0;

	task_node 	node;
	Pag_Single 	*P_single = (Pag_Single*)Single;
	struct {int CoordiAddr;int SingleAddr;}buf[1000];
	struct {int CoordiAddr;int SingleAddr;}  errSingle[500], *P_errSingle;
	P_errSingle = errSingle;

	int j=0;
	u16 *Paddrlist = NULL;

	memset(sql,0,sizeof(sql));
	memset(Single,0,sizeof(Single));
	memset(buf,0,sizeof(buf));
	memset(errSingle,0,sizeof(errSingle));
	memset(&node,0,sizeof(node));
	va_start(arg_ptr, cmd);
	//LightLevel = va_arg(arg_ptr, int);
	switch(cmd){
		case Action_GroupOpen:
			P_single->Cmd[0] = (signle_open>>8) 	& 0xff;
			P_single->Cmd[1] = signle_open 	& 0xff;
			group = va_arg(arg_ptr, int);
			sprintf(sql,"select Coor_id,Base_Addr from db_light where lt_gid=%d order by Coor_id;",group);
			Select_Table_V2(sql,(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
			break;
		case Action_GroupClose:
			P_single->Cmd[0] = (signle_close>>8) 	& 0xff;
			P_single->Cmd[1] = signle_close 	& 0xff;
			group = va_arg(arg_ptr, int);

			sprintf(sql,"select Coor_id,Base_Addr from db_light where lt_gid=%d order by Coor_id;",group);
			Select_Table_V2(sql,(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
			break;
		case Action_BroadcastOpen:
			P_single->Cmd[0] = (signle_open>>8) 	& 0xff;
			P_single->Cmd[1] = signle_open 	& 0xff;
			Select_Table_V2("select Coor_id,Base_Addr from db_light order by Coor_id;",(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
			break;
		case Action_BroadcastClose:
			P_single->Cmd[0] = (signle_close>>8) 	& 0xff;
			P_single->Cmd[1] = signle_close 	& 0xff;
			Select_Table_V2("select Coor_id,Base_Addr from db_light order by Coor_id;",(char*)buf,sizeof(buf[0]),sizeof(buf)/sizeof(buf[0]),0);
			break;
		default:break;
	}va_end(arg_ptr);


	int AddrLen = 0;
	u16 *PAddr = (u16*)((char*)P_single + sizeof(Pag_Single));
	i=0;while(buf[i].CoordiAddr != 0){
		if(buf[i].CoordiAddr == buf[i+1].CoordiAddr){
			*PAddr++ = buf[i].SingleAddr;
			AddrLen += sizeof(u16);
		}else{
			AddrLen += sizeof(u16);
			*PAddr++ = buf[i].SingleAddr;
			/* 加上数据包头和功能码 */
			P_single->Header 	= 0xFF;
			P_single->Ctrl		= CtrlCode_Check;
			/* 对应的协调器 */
			P_single->Coordi_Addr 	= buf[i].CoordiAddr;
			/* 单灯地址表的长度，按字节计算 */
			P_single->Data[0]	= (AddrLen>>8) & 0xff;
			P_single->Data[1]	= AddrLen 	& 0xff;
			/* 计算基本数据包的校验码和计算单灯地址表的校验码 */
			Crc16(P_single->Crc16,(u8*)P_single, sizeof(Pag_Single)-2);
			Crc16((u8*)PAddr, P_single->Crc16 + sizeof(P_single->Crc16), AddrLen);
			/* 打印发送的数据 */
			debug(DEBUG_single,"\n##########Send data##########\n");
			debug(DEBUG_single,"AddrLen=%04x,Coordi_Addr=%02x\n",(P_single->Data[0]<<8)|P_single->Data[1],P_single->Coordi_Addr);
			j=0;while(j < sizeof(Pag_Single)){
				debug(DEBUG_single,"%02x ",Single[j++]);
			}debug(DEBUG_single,"\n");
			debug(DEBUG_single,"Single Addr List:");
			Paddrlist = (u16*)&Single[j];
			while(*Paddrlist != 0){
				debug(DEBUG_single,"%04x ",*Paddrlist++);
			}debug(DEBUG_single,"\n");

			UartBufClear(Uart1_ttyO1_485, Flush_Input|Flush_Output);
			/* 发送数据 */
			Uart_Send(Uart1_ttyO1_485, (s8*)P_single, sizeof(Pag_Single)+AddrLen+2, 2000000);

			//sleep(10);	//等待5秒协调器处理

			memset(Single,0,sizeof(Single));
			/* 接受数据 */
			if(SUCCESS == Recv_Package((Pag_Single*)Single)){
				AddrLen = ((Pag_Single*)Single)->Data[0] <<8 | ((Pag_Single*)Single)->Data[1];
				debug(DEBUG_single,"Addrlen:%d\n",AddrLen);
				if(AddrLen){//收到有执行失败的单灯
					if( SUCCESS == DeviceRecv485_v2(Uart1_ttyO1_485,  (s8*)&Single[sizeof(Pag_Single)] , AddrLen + 2, 3000000) ){
						/* CRC校验收到的操作失败的单灯地址表 */
						if(CHK_Crc16((u8*)&Single[sizeof(Pag_Single)] + AddrLen,(u8*)&Single[sizeof(Pag_Single)] ,AddrLen) ){//单灯地址列表校验失败
							debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
							/* 返回失败？ */
						}

						debug(DEBUG_single,"***Addr List:");
						Paddrlist = (u16*)&Single[sizeof(Pag_Single)];
						while((char*)Paddrlist  < (char*)&Single[sizeof(Pag_Single)]  + AddrLen){
							debug(DEBUG_single,"%04x ",*Paddrlist);
							if((char*)P_errSingle < (char*)errSingle + sizeof(errSingle) ){
								P_errSingle->CoordiAddr = ((Pag_Single*)Single)->Coordi_Addr;
								P_errSingle->SingleAddr  = *Paddrlist++;
								++P_errSingle;
							}else{
								debug(DEBUG_single,"Error Single Addr list buffer was full!\n");
								goto deal_errSingle;
							}
						}debug(DEBUG_single,"\n\n");

					}else{//没有收到单灯地址表的数据
						debug(DEBUG_single,"Recv Single Addr list Fail\n");
						/* 返回失败？ */
					}
				}//if(!AddrLen)
			}else{//没有收到协调器的回应
				debug(DEBUG_single,"Can not Recv Single Response\n");
				return FAIL;
			}
			AddrLen = 0;
			PAddr = (u16*)((char*)P_single + sizeof(Pag_Single));
			sleep(1);
		}
		++i;
	}

 deal_errSingle://处理那些操作失败的单灯,操作失败的单灯进行单播操作
	P_errSingle = errSingle;
	/* 把错误单灯写入数据库中 */
	while(P_errSingle->CoordiAddr != 0){
		debug(DEBUG_single,"CoordiAddr:%04X,SingleAddr:%04X\n",P_errSingle->CoordiAddr,P_errSingle->SingleAddr);
		++P_errSingle;
	}
	return SUCCESS;
}
#endif

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
		Single.Header 		= Single_Header;
		Single.Ctrl 		= SingleCtrl_Single;
		Single.Coordi_Addr 	= node->pakect[3];
		Single.Single_Addr[0]	= node->pakect[4];
		Single.Single_Addr[1]	= node->pakect[5];
		Single.Cmd[0]		= (signle_open>>8) 	& 0xff;
		Single.Cmd[1]		= signle_open 		& 0xff;

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
			Display_package("Single ConfigMapAddr Send data",&Single,sizeof(Pag_Single));
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
			Display_package("Single ConfigGroup Send data",&Single,sizeof(Pag_Single));
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
			Display_package("Coordinate ConfigMapAddr Send data",&Single,sizeof(Pag_Single));
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

s32 GroupQuery(struct task_node *node)
{
	Pag_Single Single;
	struct{int Singlelen; u8 CoorAddr;} CoordiAddr[20];
	u8 ResSingle[300];
	int i = 0;
	int Addrlen = 0;
	//int WaitTime = 0;
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
		//WaitTime = CalcWaitTime();
	}
	if(SUCCESS != Res ){
		goto ERR;
	}
	//debug(1,"wait %d S\n",WaitTime/2);

	GetCoordiAddrList(CoordiAddr,sizeof(CoordiAddr));

	Single.Header 		= Single_Header;
	Single.Ctrl 		= CoordiCtrl_GetData;
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
			if(CHK_Crc16((u8*)&ResSingle[sizeof(Pag_Single)] + Addrlen,(u8*)&ResSingle[sizeof(Pag_Single)] ,Addrlen) ){//单灯地址列表校验失败
				debug(DEBUG_single,"Single Addr List Crc16 check err!\n");
				goto ERR;
			}
		}
		debug(DEBUG_single,"Addrlen=%d\n",Addrlen);
		Display_package("Recv Single State",&ResSingle[sizeof(Pag_Single)] ,Addrlen);
		InsertState2Table(&ResSingle[sizeof(Pag_Single)],Addrlen);	//把查询到的灯的状态拷贝到全局变量中
		if(Addrlen/4 >= CoordiAddr[i].Singlelen){//判断一个协调器下的数据是否全部取完
			++i;
		}else{    CoordiAddr[i].Singlelen -= Addrlen/4 ;    }
	}
	/* 回复给上位机 */
	return QueryRes(0x02,SUCCESS);
 ERR:
 	return QueryRes(0x02,FAIL);
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
	if(Is_Group != -1){//判断之前查询的是否为现在要获取的组
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
	Single.Ctrl 		= CoordiCtrl_GetData;
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
			if(CHK_Crc16((u8*)&ResSingle[sizeof(Pag_Single)] + Addrlen,(u8*)&ResSingle[sizeof(Pag_Single)] ,Addrlen) ){//单灯地址列表校验失败
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
			++i;						//获取下一个协调器的单灯状态
		}else{    CoordiAddr[i].Singlelen -=  Addrlen/4;    }
	}
	/* 回复给上位机 */
	return QueryRes(0x03,SUCCESS);
 ERR:
 	return QueryRes(0x03,FAIL);
}
