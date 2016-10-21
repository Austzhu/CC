/********************************************************************
	> File Name:	Zt_Meter.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com
	> Created Time:	2016年07月04日 星期一 16时19分39秒
 *******************************************************************/
#include "Zt_Meter.h"

s32 DIDO_Respond(int cmd,  u8 Addr,  u8 Relay,  u8 Relay_Stat,  int Is_Success)
{
	u8 RespondBuf[128];
	memset(RespondBuf,0,sizeof(RespondBuf));
	RespondBuf[0] = 0x51;
	RespondBuf[2] = 0x80;
	RespondBuf[4] = 0xE1;

	switch(cmd){
		case Respond_Open:
			RespondBuf[1] = 0x08;
			RespondBuf[3] = 0x06;
			RespondBuf[5] = 0x01;	break;
		case Respond_Close:
			RespondBuf[1] = 0x08;
			RespondBuf[3] = 0x06;
			RespondBuf[5] = 0x02;	break;
		case Respond_CheckDO:
			RespondBuf[1] = 0x08;
			RespondBuf[3] = 0x06;
			RespondBuf[5] = 0x03;	break;
		case Respond_CheckDI:
			RespondBuf[1] = 0x08;
			RespondBuf[3] = 0x06;
			RespondBuf[5] = 0x04;	break;
		default:break;
	}
	RespondBuf[6] = Addr;		//设备模块地址
	RespondBuf[7] = Relay;		//哪几个继电器
	RespondBuf[8] = Relay_Stat;	//继电器状态
	RespondBuf[9] = Is_Success; 	//是否成功

	if(!TaskGenerateAndAppend(ETH_NET_TYPE_QUEUE,RespondBuf,NET_TASK,TASK_LEVEL_NET)){
		return SUCCESS;
	}else{
		err_Print(DEBUG_DIDO,"DIDO Respond err!\n");
		return FAIL;
	}
}

s32 DIDO_Open(struct task_node *node)
{
	int ii = 0;
	int nDO = node->pakect[4];
	struct DAM0808 Meter;
	memset(&Meter,0,sizeof(Meter));
	Meter.Addr 	= node->pakect[3];
	if(nDO == DIDO_All){//全开
		Meter.Cmd 	= 0x0F;
		Meter.Data[0] 	= 0X00;
		Meter.Data[1] 	= 0X00;
		Meter.Data[2] 	= 0X00;
		Meter.Data[3] 	= 0X08;
		Meter.Data[4] 	= 0X01;
		Meter.Data[5] 	= 0XFF;
		Meter.Data[6] 	= 0X39;
		Meter.Data[7] 	= 0XDB;
		Display_package("DIDO Send Data",&Meter,DIDO_nLenALL);
		UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
		Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenALL, 1000000);
		if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead, 2000000)){
			Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead);
		}else{
			debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
		}
		goto Chk_Stat;
	}
	Meter.Cmd 	= 0x05;
	Meter.Data[2] 	= 0xFF;
	Meter.Data[3] 	= 0x00;
	while(ii < DIDO_8DO){
		Meter.Data[1] = ii++;
		if(nDO&0x01){
			Crc16_HL( (u8*)(&Meter) + DIDO_nLenOC -2,(u8*)&Meter,DIDO_nLenOC -2);
			Display_package("DIDO Send Data",&Meter,DIDO_nLenOC);
			UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
			Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenOC, 1000000);
			if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead, 2000000)){
				Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead);
			}else{
				debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
			}
		}
		nDO >>= 1;
	}
Chk_Stat:
	/* 查询继电器的状态 */
	Meter.Addr 	= node->pakect[3];
	Meter.Cmd 	= 0x01;
	Meter.Data[0] 	= 0x00;
	Meter.Data[1] 	= 0x00;
	Meter.Data[2] 	= 0x00;
	Meter.Data[3] 	= 0x08;
	Crc16_HL( (u8*)(&Meter) + DIDO_nLenOC -2,(u8*)&Meter,DIDO_nLenOC -2);
	Display_package("DIDO Send Data",&Meter,DIDO_nLenOC);
	UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
	Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenOC, 1000000);
	/* 等待回复 */
	memset(&Meter,0,sizeof(Meter));
	if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead-2, 2000000)){
		Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead-2);
		/* Crc校验 */
		if( CHK_Crc16HL((u8*)(&Meter) + DIDO_nLenOC-4, (u8*)&Meter, DIDO_nLenOC -4)){
			debug(DEBUG_DIDO,"DIDO Recv Crc16 Check err!\n");
			goto err;
		}
	}else{
		debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
		goto err;
	}

	/* 回复上位机成功 */
	return DIDO_Respond(Respond_Open,Meter.Addr,node->pakect[4],Meter.Data[1],SUCCESS);
err:
	/* 回复上位机失败 */
	return DIDO_Respond(Respond_Open,Meter.Addr,node->pakect[4],Meter.Data[1],FAIL);
}

s32 DIDO_Close(struct task_node *node)
{
	int ii = 0;
	int nDO = node->pakect[4];
	struct DAM0808 Meter;
	memset(&Meter,0,sizeof(Meter));
	Meter.Addr 	= node->pakect[3];
	if(nDO == DIDO_All){//全开
		Meter.Cmd 	= 0x0F;
		Meter.Data[0] 	= 0X00;
		Meter.Data[1] 	= 0X00;
		Meter.Data[2] 	= 0X00;
		Meter.Data[3] 	= 0X08;
		Meter.Data[4] 	= 0X01;
		Meter.Data[5] 	= 0X00;
		Meter.Data[6] 	= 0X79;
		Meter.Data[7] 	= 0X9B;
		Display_package("DIDO Send Data",&Meter,DIDO_nLenALL);
		UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
		Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenALL, 1000000);
		if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead, 2000000)){
			Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead);
		}else{
			debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
		}
		goto Chk_Stat;
	}
	Meter.Cmd 	= 0x05;
	Meter.Data[2] 	= 0x00;
	Meter.Data[3] 	= 0x00;
	while(ii < DIDO_8DO){
		Meter.Data[1] = ii++;
		if(nDO&0x01){
			Crc16_HL( (u8*)(&Meter) + DIDO_nLenOC -2,(u8*)&Meter,DIDO_nLenOC -2);
			Display_package("DIDO Send Data",&Meter,DIDO_nLenOC);
			UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
			Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenOC, 1000000);
			if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead, 2000000)){
				Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead);
			}else{
				debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
			}
		}
		nDO >>= 1;
	}
Chk_Stat:
	/* 查询继电器的状态 */
	Meter.Addr 	= node->pakect[3];
	Meter.Cmd 	= 0x01;
	Meter.Data[0] 	= 0x00;
	Meter.Data[1] 	= 0x00;
	Meter.Data[2] 	= 0x00;
	Meter.Data[3] 	= 0x08;
	Crc16_HL( (u8*)(&Meter) + DIDO_nLenOC -2,(u8*)&Meter,DIDO_nLenOC -2);
	Display_package("DIDO Send Data",&Meter,DIDO_nLenOC);
	UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
	Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenOC, 1000000);
	/* 等待回复 */
	memset(&Meter,0,sizeof(Meter));
	if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead-2, 2000000)){
		Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead-2);
		/* Crc校验 */
		if( CHK_Crc16HL((u8*)(&Meter) + DIDO_nLenOC-4, (u8*)&Meter, DIDO_nLenOC -4)){
			debug(DEBUG_DIDO,"DIDO Recv Crc16 Check err!\n");
			goto err;
		}
	}else{
		debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
		goto err;
	}
	/* 回复上位机成功 */
	return DIDO_Respond(Respond_Close,Meter.Addr,node->pakect[4],Meter.Data[1],SUCCESS);
err:
	/* 回复上位机失败 */
	return DIDO_Respond(Respond_Close,Meter.Addr,node->pakect[4],Meter.Data[1],FAIL);
}
s32 DIDO_ReadDO(struct task_node *node)
{
	struct DAM0808 Meter;
	memset(&Meter,0,sizeof(Meter));
	/* 查询继电器的状态 */
	Meter.Addr 	= node->pakect[3];
	Meter.Cmd 	= 0x01;
	Meter.Data[0] 	= 0x00;
	Meter.Data[1] 	= 0x00;
	Meter.Data[2] 	= 0x00;
	Meter.Data[3] 	= 0x08;
	Crc16_HL( (u8*)(&Meter) + DIDO_nLenOC -2,(u8*)&Meter,DIDO_nLenOC -2);
	Display_package("DIDO Send Data",&Meter,DIDO_nLenOC);
	UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
	Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenOC, 1000000);
	/* 等待回复 */
	memset(&Meter,0,sizeof(Meter));
	if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead-2, 2000000)){
		Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead-2);
		/* Crc校验 */
		if( CHK_Crc16HL((u8*)(&Meter) + DIDO_nLenOC-4, (u8*)&Meter, DIDO_nLenOC -4)){
			debug(DEBUG_DIDO,"DIDO Recv Crc16 Check err!\n");
			goto err;
		}
	}else{
		debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
		goto err;
	}
	/* 回复上位机成功 */
	return DIDO_Respond(Respond_CheckDO,Meter.Addr,node->pakect[4],Meter.Data[1],SUCCESS);
err:
	/* 回复上位机失败 */
	return DIDO_Respond(Respond_CheckDO,Meter.Addr,node->pakect[4],Meter.Data[1],FAIL);
}
s32 DIDO_ReadDI(struct task_node *node)
{
	struct DAM0808 Meter;
	memset(&Meter,0,sizeof(Meter));
	/* 查询继电器的状态 */
	Meter.Addr 	= node->pakect[3];
	Meter.Cmd 	= 0x02;
	Meter.Data[0] 	= 0x00;
	Meter.Data[1] 	= 0x00;
	Meter.Data[2] 	= 0x00;
	Meter.Data[3] 	= 0x08;
	Crc16_HL( (u8*)(&Meter) + DIDO_nLenOC -2,(u8*)&Meter,DIDO_nLenOC -2);
	Display_package("DIDO Send Data",&Meter,DIDO_nLenOC);
	UartBufClear(Uart3_tty03_DIDO, Flush_Input|Flush_Output);
	Uart_Send(Uart3_tty03_DIDO, (s8*)&Meter, DIDO_nLenOC, 1000000);
	/* 等待回复 */
	memset(&Meter,0,sizeof(Meter));
	if( !DeviceRecv485_v2(Uart3_tty03_DIDO, (char*)&Meter, DIDO_nLenRead-2, 2000000)){
		Display_package("#####Uart2_tty02_DIDO Recv data",&Meter,DIDO_nLenRead-2);
		/* Crc校验 */
		if( CHK_Crc16HL((u8*)(&Meter) + DIDO_nLenOC-4, (u8*)&Meter, DIDO_nLenOC -4)){
			debug(DEBUG_DIDO,"DIDO Recv Crc16 Check err!\n");
			goto err;
		}
	}else{
		debug(DEBUG_DIDO,"#####Can't Recv from Uart2_tty02_DIDO!\n");
		goto err;
	}
	/* 回复上位机成功 */
	return DIDO_Respond(Respond_CheckDI,Meter.Addr,node->pakect[4],Meter.Data[1],SUCCESS);
err:
	/* 回复上位机失败 */
	return DIDO_Respond(Respond_CheckDI,Meter.Addr,node->pakect[4],Meter.Data[1],FAIL);
}