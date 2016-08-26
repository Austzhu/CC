#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "include.h"


#define NEWMSG
#define emfuture

#ifdef NEWMSG						//yuan dingyi
	#define TOTAL_UARTS_NUM		6
	#define UART0_TTYS1_DIS		1		//ttyS1	PB4:TXD0,PB5:RXD0
	#define UART1_TTYS2_GPRS		2		//ttyS2:PB6:TXD1,PB7:RXD1
	#define UART2_TTYS3_485		3		//ttyS3 PB8:TXD2,PB9:RXD2 20120612
	#define UART3_TTYS4_GPS		4		//ttyS4 PB10:TXD3,PB11:RXD3 20120612
	#define UART4_TTYS5_BACK		5		//ttyS5 PA31:TXD4,PA30:RXD4 20120612
	#define UART5_TTYS6_PLC		6		//ttyS6 PB12:RXD5,PB13:TXD5
	#define UART_RECIVE_SIZE		512
#else
	#define TOTAL_UARTS_NUM		3
	#define UART1_TTYS2_GPRS		1
	#define UART2_TTYS3_485		3		//ttyAT3    20120612
	#define UART5_TTYS6_PLC		2
	#define UART_RECIVE_SIZE		512
#endif

struct UartReciveStruct{
	UCHAR buf[UART_RECIVE_SIZE];
	SINT len;
	SINT head;
};

 /*  串口波特率枚举 */
typedef enum{
	Bd300 		= 300,
	Bd600 		= 600,
	Bd1200 	= 1200,
	Bd2400 	= 2400,
	Bd4800 	= 4800,
	Bd9600 	= 9600,
	Bd19200 	= 19200,
	Bd38400 	= 38400,
	Bd57600 	= 57600,
	Bd115200 	= 115200

} UartSpeed;
/*  串口端口枚举 */
typedef enum{
	Uart0_ttyS1_DIS ,		//0
	Uart1_ttyS2_GPRS, 		//1
	Uart2_ttyS3_485,
	Uart3_ttyS4_GPS,
	Uart4_ttyS5_BACK,
	Uart5_ttyS6_PLC
} UartPort;

typedef enum{
	Flush_Input 	= 0x01,
	Flush_Output 	= 0x02
}Flush_IO;

extern s32 Uart_Open(u32 port);
extern s32 Uart_Close(u32 port);
extern s32 Uart_Config(u32 port, u32 speed, s32 bits, s32 stop,	 s8 parity);
extern s32 Uart_Recv(u32  port,  s8* buf,   u32 len,  s32  block);	//block为是否阻塞，<=0为阻塞，>0为等待的时间，单位为us
extern s32 Uart_Send(u32  port,  s8* buf,   u32 len,  s32  block);
extern s32 Uart_GetFd(u32  port);

extern s32 UartForGprsInit(void);
extern s32 UartForDISInit(void);
extern s32 UartForPlcInit(void);
extern s32 UartFor485Init(void);
extern s32 UartForCoordi(void);
extern s32 UartBufClear(u32 port, Flush_IO IO);

#endif
