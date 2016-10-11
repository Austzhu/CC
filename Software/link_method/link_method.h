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
#ifndef __LINK_METHOD_H__
#define __LINK_METHOD_H__
#include "include.h"
#include "ether.h"

/**
 *  通信操作接口
 */
typedef struct faalitf_t{
	UCHAR *rcvbuf;  				//接收缓存
	UCHAR *sndbuf;    				//发送缓存
	SINT (*connect)(void);				//链接建立
	SINT (*disconnect)(void);			//断开链接
	SINT (*logon)(void);				//登陆
	SINT (*linetest)(void);				//链路测试
	SINT (*linestat)(void);				//链路状态
	SINT (*rawsend)(UCHAR *buf, int len);   	//发送函数
	int (*getchar)(UCHAR *buf);    			//接收函数
	SINT (*keepalive)(void);				//心跳

	SINT(*bak_1)(void);				//函数功能扩展1
	SINT(*bak_2)(void);				//函数功能扩展2
	SINT(*bak_3)(void);				//函数功能扩展3
	SINT(*bak_4)(void);				//函数功能扩展4
	SINT timeout;   					//接收超时
	SINT rcvmax;    					//接收数据区最大长度
	SINT sndmax;    					//发送数据区最大长度
	SINT sndnor;     					//发送数据一般长度上限
	UCHAR attr;  					//通道属性
} faalitf_t;

typedef struct {
	UINT cnt;
	UINT max;
	UINT flag;
} faaltimer_t;

typedef struct {
	UCHAR *pbuf;
	USHORT cnt;
	USHORT maxlen;
	UCHAR stat;
} faalfsm_t;


#define SOCKET_LINESTAT_OK 	0
#define SOCKET_LINESTAT_ERR 	1
#define CC_REGISTER_OK 	   	0
#define CC_REGISTER_ERR 	   	1
#define CC_LINE_STAT_OK 	   	0
#define CC_LINE_STAT_ERR 	   	1
#define ETH_TIMEOUT  		50
#define ETHER_RECV_BUF_MAX 	512
#define ETHER_SEND_BUF_MAX 	512
#define ETHER_SNORMAL_BUF 	512
#define FAALRTN_OK 			0
#define FAAL_HEAD 			0x68
#define FAAL_TAIL 			0x16
#define LEN_FAALHEAD   		10
#define LEN_FAALTAIL   		2
#define UDP 				0
#define  ITF_WAY_PLC 			0
#define  ITF_WAY_GPRS		1
#define  ITF_WAY_ETHER		2
#define FAALITF_MAXNUM 		4
#define CONTROLDBYCC    		0
#define CONTROLDBYMANUAL    	1

SINT faal_connect(UCHAR itf);			//need to write
SINT faal_disconnect(UCHAR itf);		//need to write
SINT faal_logon(UCHAR itf);
SINT faal_linktest(UCHAR itf);
SINT faal_linkstat(UCHAR itf);			//need to write
SINT faal_sendpkt(UCHAR itf, faalpkt_t *pkt);
SINT faal_act_send(UCHAR itf, UCHAR wait, faalpkt_t *pkt);
SINT faal_rcvpkt(UCHAR itf,UCHAR *recv);
SINT TopUserKeepalive(UCHAR itf);		//need to write



#endif
