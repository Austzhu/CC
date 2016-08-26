#include "meter_cmdrdwr.h"
//struct MetterElecInforStruc 	TOPMetterElecInformation;

extern volatile UCHAR TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];	
extern volatile UCHAR TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];			


SINT DIDOOprate(UCHAR DeviceId,UCHAR chanel,UCHAR action)
{
	//CHAR result = 1;
	CHAR i;
	CHAR RetResul = 0x01;
	//CHAR OutPutstat;
	UCHAR Reciv[50];
	SINT RecivLen;
	CHAR crc[2];
	CHAR ActSendBuf[METTERCMD_LENMAX];
	MetterOpratePack DIDOOpenCmd = {0x01,0x05,0x27,0x10,0xff,0x00};
	MetterOpratePack DIDOCloseCmd = {0x01,0x05,0x27,0x10,0x00,0x00};

	memset(ActSendBuf,0,strlen(ActSendBuf));
	
	switch(action){
		case DIDO_OPEN:
			DIDOOpenCmd.address = DeviceId;
			if(UartBufClear(Uart2_ttyS3_485,Flush_Input|Flush_Output)){return UART_BUF_CLEAR_ERR;}
			//metter open oprate
			for(i = 0;i<DONumPerMeter ;i++){
				if((chanel>>i)& 0x01){		//chanl :0000 0011
					debug(DEBUG_LOCAL_METTER,"chnal is %d,i  is %d, and DIDOOpenCmd.start_addr_low is %x\n",chanel,i,DIDOOpenCmd.start_addr_low);
					memcpy(ActSendBuf,(CHAR *)&DIDOOpenCmd,6);
					debug(DEBUG_LOCAL_METTER,"ActSendBuf is %x:%x:%x:%x:%x:%x\n",ActSendBuf[0],ActSendBuf[1],ActSendBuf[2],ActSendBuf[3],ActSendBuf[4],ActSendBuf[5]);
					if(!Crc16((UCHAR *)crc, (UCHAR *)&DIDOOpenCmd, 6)){
						ActSendBuf[6+1] = crc[0];
						ActSendBuf[6] = crc[1];
					}
					if(Uart_Send(Uart2_ttyS3_485,(CHAR *)ActSendBuf,6+2,  0)){
						Uart_Send(Uart2_ttyS3_485,(CHAR *)ActSendBuf,6+2,0);
					}
					if(!Device485Reciv(Uart2_ttyS3_485,Reciv,&RecivLen)){
						debug(DEBUG_LOCAL_METTER,"act send in open is %x:%x:%x:%x:%x:%x:%x\n",ActSendBuf[0],ActSendBuf[1],ActSendBuf[2],ActSendBuf[3],ActSendBuf[4],
								ActSendBuf[5],ActSendBuf[6]);
						debug(DEBUG_LOCAL_METTER,"recive buf in open oprate is %x:%x:%x:%x:%x:%x:%x:\n",Reciv[0],Reciv[1],Reciv[2],Reciv[3],
								Reciv[4],Reciv[5],Reciv[6]);
						debug(DEBUG_LOCAL_METTER,"RecivLen in open oprate  is %d\n",RecivLen);
						if(!strncmp((CHAR *)ActSendBuf,(CHAR *)Reciv,8)){
							RetResul &= 0x1;
						}
						else{
							RetResul &= 0x00;
						}
						debug(DEBUG_LOCAL_METTER,"RetResul is RetResul[%d] %d\n",i,RetResul);								
					}else{	//can not reciv
						RetResul &= 0x00;
					}
					debug(DEBUG_LOCAL_METTER,"RetResul per time is %d\n",RetResul);
					sleep(1);
				}
				DIDOOpenCmd.start_addr_low++;
			}
			if(RetResul){
				printf("DIDO_OPEN success\n");
				return SUCCESS;
			}	
			break;
		case DIDO_CLOSE:	
			DIDOCloseCmd.address = DeviceId;
			if(UartBufClear(Uart2_ttyS3_485,Flush_Input|Flush_Output)){return UART_BUF_CLEAR_ERR;}
			for(i = 0;i<DONumPerMeter ;i++){
				if((chanel>>i)& 0x01){		//chanl :0000 0011
					debug(DEBUG_LOCAL_METTER,"chnal is %d,i  is %d, and DIDOCloseCmd.start_addr_low is %x\n",chanel,i,DIDOCloseCmd.start_addr_low);
					memcpy(ActSendBuf,(CHAR *)&DIDOCloseCmd,6);
					debug(DEBUG_LOCAL_METTER,"ActSendBuf is %x:%x:%x:%x:%x:%x\n",ActSendBuf[0],ActSendBuf[1],ActSendBuf[2],ActSendBuf[3],ActSendBuf[4],ActSendBuf[5]);
					if(!Crc16((UCHAR *)crc, (UCHAR *)&DIDOCloseCmd, 6)){
							ActSendBuf[6+1] = crc[0];
							ActSendBuf[6] = crc[1];
					}	
					Uart_Send(Uart2_ttyS3_485,(CHAR *)ActSendBuf,6+2,0);
					if(!Device485Reciv(Uart2_ttyS3_485,Reciv,&RecivLen)){
						debug(DEBUG_LOCAL_METTER,"act send in open is %x:%x:%x:%x:%x:%x:%x\n",ActSendBuf[0],ActSendBuf[1],ActSendBuf[2],ActSendBuf[3],ActSendBuf[4],
									ActSendBuf[5],ActSendBuf[6]);
						debug(DEBUG_LOCAL_METTER,"recive buf in open oprate is %x:%x:%x:%x:%x:%x:%x:\n",Reciv[0],Reciv[1],Reciv[2],Reciv[3],
									Reciv[4],Reciv[5],Reciv[6]);
						debug(DEBUG_LOCAL_METTER,"RecivLen in open oprate  is %d\n",RecivLen);
						if(!strncmp((CHAR *)ActSendBuf,(CHAR *)Reciv,8)){
							RetResul &= 0x1;
						}else{
							RetResul &= 0x00;
						}
					}else{
						RetResul &= 0x00;
					}
					debug(DEBUG_LOCAL_METTER,"RetResul is %d\n",RetResul);
					debug(DEBUG_LOCAL_METTER,"RetResul is RetResul[%d] %d\n",i,RetResul);
					sleep(1);
				}
				DIDOCloseCmd.start_addr_low++;
			}
			if(RetResul){
					printf("DIDO_OPEN success\n");
					return SUCCESS;
			}
			break;	
		default:
			break;
	
	}
	return FAIL;
}



SINT DIDOOutInputStatQuery(UCHAR DeviceId,UCHAR chanel,UCHAR type,UCHAR *stat)
{
	UCHAR Reciv[20];
	CHAR sendbuf[50];
	SINT RecivLen;
	CHAR crc[2];
	//CHAR QueSend[10]={0x01,0x03,0x9f,0x30,0x00,0x02};
	MetterOpratePack QueSend = {0x01,0x03,0x4e,0x22,0x00,0x01};		//
	QueSend.address = DeviceId;
	
	if(type == DIDO_QUERY_OUTPUT){
		QueSend.start_addr_low = 0x22;		//4E22   query output
	}
	
	if(type == DIDO_QUERY_INPUT){
		QueSend.start_addr_low = 0x20;		//4E20   query input
	}
	
	memset(sendbuf,0,strlen(sendbuf));

	/*crc check*/
	Crc16((UCHAR *)crc, (UCHAR *)&QueSend, 6);
	QueSend.crc_high = crc[0];
	QueSend.crc_low = crc[1];

	memcpy(sendbuf,(CHAR *)&QueSend,8);

	UartBufClear(Uart2_ttyS3_485,Flush_Input|Flush_Output);
	
	debug(DEBUG_LOCAL_METTER,"sendbuf is %x:%x:%x:%x:%x:%x:%x:%x\n",sendbuf[0],sendbuf[1],sendbuf[2],sendbuf[3],
		sendbuf[4],sendbuf[5],sendbuf[6],sendbuf[7]);
	
	//send
	Uart_Send(Uart2_ttyS3_485,(CHAR *)sendbuf,6+2,0);

	//recive
	if(!Device485Reciv(Uart2_ttyS3_485,Reciv,&RecivLen)){
		debug(DEBUG_LOCAL_METTER,"recive buf out is %x:%x:%x:%x:%x:%x:%x:%x\n",Reciv[0],Reciv[1],Reciv[2],Reciv[3],
			Reciv[4],Reciv[5],Reciv[6],Reciv[7]);
		debug(DEBUG_LOCAL_METTER,"RecivLen out  is %d\n",RecivLen);
		
	

		//
		*stat =(Reciv[4]);

		//fix the global stat
		TOP_LOCALREALISTIC_DIDOCIR_STAT[(DeviceId-6)] = (Reciv[4]);

		return SUCCESS;
	};
	debug(DEBUG_LOCAL_METTER,"DIDOOutInputStatQuery DIDO_QUERY_ERR %d\n",DIDO_QUERY_ERR);

	return DIDO_QUERY_ERR;

}

SINT MetterParamQuery(UCHAR DeviceId,UCHAR chanel,UCHAR action)
{
return SUCCESS;
}

SINT DIDOParamQuery(UCHAR DeviceId,UCHAR chanel,UCHAR action)
{
return SUCCESS;
}


void UART1_TTYS2_PLC_change485test()
{
	MetterOpratePack sendopen = {0x01,0x03,0x9c,0x40, 0x00,0x3a};
	//CHAR *send="123456";
	CHAR result;
//	SINT recivlen;
	CHAR sendbuf[50];
//	CHAR recivbuf[100];
	CHAR crc[2];

	char test1[]="send success\n";
	char test2[]="send fail\n";

	memset(sendbuf,0,strlen(sendbuf));
	memcpy(sendbuf,(CHAR *)&sendopen,6);

	UartBufClear(Uart2_ttyS3_485,Flush_Input|Flush_Output);


	Crc16((UCHAR *)crc, (UCHAR *)&sendopen, 6);
			
	sendbuf[6+1] = crc[0];
	sendbuf[6] = crc[1];
	
	puts("send");
	//puts(sendbuf);
	printf("%x : %x :%x: %x: %x: %x: %x:%x",sendbuf[0],sendbuf[1],sendbuf[2],sendbuf[3],sendbuf[4],sendbuf[5],sendbuf[6],sendbuf[7]);
	result = Uart_Send(Uart2_ttyS3_485,(CHAR *)sendbuf,6+2, 0);
	//printf("result is %s %s\n",test1,test2);

	printf("result is %s\n",result == 0 ? test1:test2);
//	sleep(1);

}

SINT Device485Reciv(UCHAR UARTPORT,UCHAR *Reciv,SINT *RecivLen)
{

	//SINT recivlen;
	//UCHAR recivbuf[100];
	s32 times = 0;
	SINT WaitTimes = 0;

	while(WaitTimes < METERD_WAIT_TIME_CONTROL){
		*RecivLen = Uart_Recv(UARTPORT,(s8*)Reciv, 300,100000);
		WaitTimes++;

		if (WaitTimes > METERD_WAIT_TIME_CONTROL-2){
			debug(DEBUG_LOCAL_METTER,"Device485Reciv time out(-2),and WaitTimes is %d\n",WaitTimes);
		}

		if(*RecivLen < 1){
			continue;
		}
		debug(DEBUG_LOCAL_METTER,"recivlen in fun is %d\n",*RecivLen);
		debug(DEBUG_LOCAL_METTER,"contex in  is\n");
		for(times = 0;times<(*RecivLen);times++){
			debug(DEBUG_LOCAL_METTER,"%x:",Reciv[times]);
		}debug(DEBUG_LOCAL_METTER,"\n");

		return SUCCESS;
	}	//end of new

	return FAIL;
}
