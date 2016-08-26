/******************************************************************
** 文件名:	infor_out.c
** Copyright (c) 2012-2014 *********公司技术开发部
** 创建人:	wguilin
** 日　期:	2012.03
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/

#ifndef __ETHER_H__
#define __ETHER_H__
#include "include.h"

#include "link_method.h"
#include "infor_out.h"
#include "cc_param.h"
//#include "softwareinit.h"
#include "loadfile.h"
#include "gprs_ppp.h"

/*Global variables */
extern int sock_ether;

SINT Ether_Disconnect(void);				//ok-20120316	socket_comm_line_stat MODIFY OK
SINT Ether_Connect(void);				//ok-20120316  socket_comm_line_stat MODIFY OK
SINT Ether_Logon(void);				//OK-20120317	 cc_register_stat
SINT Ether_HeartBeat(void);				//OK-20120317	 cc_register_stat
SINT Ether_Linestat(void);				//socket_comm_line_stat MODIFY OK
SINT Ether_RawSend(UCHAR *buf, int len);		//ok-20120316 socket_comm_line_stat
SINT Ether_Getchar(UCHAR *buf);			//ok-20120316  socket_comm_line_stat
SINT Ether_Keepalive(void);
//int linestat_empty(void);



#define CLOSE_SOCKET(sock)   do{ if((sock) >= 0) { close(sock); sock = -1; }}while(0)

#define CONNECT_WAIT_SECOND 	5
#define SEND_BUF_EMPTY_TIMES 	20
#define SEND_WHEN_EMPTY_TIME 	10
#define SOCKET_SET_OPT_TIMEOUT 	30



#endif
