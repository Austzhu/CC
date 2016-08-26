/********************************************************************
	> File Name:	coordinate.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com 
	> Created Time:	2016年04月11日 星期一 09时57分35秒
 *******************************************************************/
#include "coordinate.h"
#include "serial.h"
#define  BUFSIZE 200

static s8 Sendbuf_485[BUFSIZE];
static s8 Recvbuf_485[BUFSIZE];
/**
 * 获取发送或接受buf的首地址
 * sr： 要获取的是发生还是接收的地址
 * 成功返回buf的首地址，失败返回NULL
 */
s8* Getbuf(SR_t sr)
{
	switch(sr){
		case send_addr: return Sendbuf_485;
		case recv_addr:  return Recvbuf_485;
		default:
			debug(DEBUG_coordinate,"unknow address sr:%d\n",sr); 
		return NULL;
	}
}

s32 Device_recv_485(UartPort  port,  s8** buf,   u32 len,  s32  block)
{
	memset(Recvbuf_485,0,sizeof(Recvbuf_485));
	*buf = Recvbuf_485;
	return Uart_Recv(port,  Recvbuf_485, len,  block);
}

s32 DeviceRecv485_v2(UartPort  port,  s8 *buf,   u32 len,  s32  block)
{
	return Uart_Recv(port,buf,len,block);
#if 0
	int i = 0;
	int res=0;
	while(len > 0){
		if(len > 40){
			res |= Uart_Recv(port, buf+i, 40,  block);
			len -= 40;
			i += 40;
		}else{
			res |= Uart_Recv(port, buf+i, len,  block);
			len = 0;
		}
	}
	return res;
#endif
}
