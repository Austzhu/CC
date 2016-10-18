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
#include "link_method.h"

extern UCHAR 		UID[];
extern GlobalCCparam CCparamGlobalInfor;
extern volatile SINT 	TOPSocketConnectState;
extern volatile SINT 	TOPCCRegistr2ServerState;

faaltimer_t 	g_faaltimer[FAALITF_MAXNUM];
faalfsm_t 	g_faalfsm[FAALITF_MAXNUM];
ULONG 	G_Ether_Rcvbuf[ETHER_RECV_BUF_MAX/4];
ULONG 	G_Ether_Sndbuf[ETHER_SEND_BUF_MAX/4];
//volatile UINT svrcomm_line
vu8 	socket_comm_line_stat = 1;		//must
vu8	 cc_register_stat;

const faalitf_t 	g_faalitf[FAALITF_MAXNUM] = {
	/* 2 ITF_WAY_ETHER */
	[2] = {(UCHAR *)G_Ether_Rcvbuf, (UCHAR *)G_Ether_Sndbuf,
		Ether_Connect,Ether_Disconnect,Ether_Logon,Ether_HeartBeat,
		Ether_Linestat,Ether_RawSend,Ether_Getchar,Ether_Keepalive,
		NULL,NULL,NULL,NULL,
		ETH_TIMEOUT,ETHER_RECV_BUF_MAX,ETHER_SEND_BUF_MAX,ETHER_SNORMAL_BUF,0},

};
/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
void stop_faaltimer(UCHAR itf)
{
	g_faaltimer[itf].flag = 0;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
//void faal_clrstat(UCHAR itf,UCHAR *recv)
//UCHAR basic_clrstat(UCHAR itf, UCHAR * recv)
void basic_clrstat(UCHAR itf, UCHAR * recv)
{
	g_faalfsm[itf].pbuf = recv;
	g_faalfsm[itf].stat = 0;
	g_faalfsm[itf].cnt = 0;
	g_faalfsm[itf].maxlen = 0;

	stop_faaltimer(itf);
//	return 0;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
void htimer_faaldl(UCHAR itf,UCHAR *recv)
{
	if(!g_faaltimer[itf].flag) {
		g_faalfsm[itf].pbuf = recv;
		g_faalfsm[itf].stat = 0;
		g_faalfsm[itf].cnt = 0;
		g_faalfsm[itf].maxlen = 0;
//		return 0;
	}

	g_faaltimer[itf].cnt++;
	if(g_faaltimer[itf].cnt >= g_faaltimer[itf].max) {
		//faal_clrstat(itf,recv);
		basic_clrstat(itf,recv);
	}

//	return 0;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
//UINT set_faaltimer(UCHAR itf, int max)
static void set_faaltimer(UCHAR itf, int max)

{
	g_faaltimer[itf].cnt = 0;
	g_faaltimer[itf].max = max;
	g_faaltimer[itf].flag = 1;
//	return 0;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
#if 0
UINT faal_linktest(UCHAR itf)
{
	char sndbuf[50];
	//faalpkt_t *pkt = (faalpkt_t *)FAAL_SNDBUF(itf);
	faalpkt_t *pkt = (faalpkt_t *)sndbuf;
	UINT rtn;

	pkt->ctrl = 0xee;
	pkt->len = 1;
	pkt->data[0]=0xee;

	rtn = faal_act_send(itf, 0, pkt);
	if(DEBUG_LOCAL == 1){
		printf("rtn in faal_linktest is %d\n",rtn);
	}
	if(1 == rtn) {
		if(DEBUG_LOCAL == 1){
			printf("link test fail\r\n");
		}
		if(DEBUG_ERR_RECD == 1){
				debug_err_reccord(LINK_TEST_FAIL_ERR);
		}
			/*	del by zpf--2012年2月11日9:55:47
					DAIL_ERROR_TIMES_ALL++;

					if(DAIL_ERROR_TIMES_ALL>20){
				//rrrrrr
							gprsline_pwroff;
					//		gprsline_pwroff
							Sleep(3000);
							gprsline_pwron;
							Sleep(3000);

					}
			*/
	}
	return(rtn);
}
#endif

/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
#if 0
UINT faal_logon(UCHAR itf)
{
	char sndbuf[50];
	faalpkt_t *pkt = (faalpkt_t *)sndbuf;

	pkt->ctrl = 0xf0;
	pkt->len = 0;
//	pkt->len = 1;
//	pkt->data[0]=55;

	if(faal_act_send(itf, 0, pkt)) {

		if(DEBUG_LOCAL == 1){
			printf("logon fail.\n");
		}

	return 1;

	}


	socket_comm_line_stat = SOCKET_LINESTAT_OK;					//表示 正常链路
		if(DEBUG_LOCAL == 1){
			printf("logon ok.\n");
		}
	return SUCCESS;
}

#endif

/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
SINT faal_sendpkt(UCHAR itf, faalpkt_t *pkt)
{
	SINT len;

	//只有itf==FAALITF_SMSGPRS才有网络开关
	len = basic_makepkt(itf, pkt);
	debug(DEBUG_LOCAL_LINK_METH,"send itf=%d\n",itf);


	if((*g_faalitf[itf].rawsend)((UCHAR *)pkt, len)) {
		//debug(DEBUG_LOCAL_LINK_METH,"faal_sendpkt error : rawsend\n");
		return 1;
	}

	return SUCCESS;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
SINT faal_act_send(UCHAR itf, UCHAR wait, faalpkt_t *pkt)
{
	SINT rtn = FAALRTN_OK;
	faalpkt_t *psend = pkt;


	if((!wait)) {
		rtn = faal_sendpkt(itf, psend);
	} else {
		//rtn = faal_send_wait(itf, psend);
	}
	/*如果发送成功,Ether/GPRS接口清除心跳计数,重新开始(减少心跳包)lzh*/
	return rtn;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
//UINT faal_makepkt(unsigned char itf, faalpkt_t *pkt)
SINT  basic_makepkt(UCHAR itf, faalpkt_t *pkt)
{
	SINT i, len;
	UCHAR *puc;
	UCHAR chk;
	// extern struct  cc_para_term;

	len = pkt->len;
	if(len >9999) {
		debug(DEBUG_LOCAL_LINK_METH,"basic_makepkt error : len\n");
		#if DEBUG_ERR_RECD_LINK
			debug_err_reccord(FAAL_MAKEPACK_ERR);
		#endif

		return(-1);
	}
	len += LEN_FAALHEAD;

	pkt->head = pkt->dep = FAAL_HEAD;
	if( 0 != itf ) {		//packet send via plc already have node address in rtua field
		pkt->rtua[0] = CCparamGlobalInfor.CCUID[0];
		pkt->rtua[1] = CCparamGlobalInfor.CCUID[1];
		pkt->rtua[2] = CCparamGlobalInfor.CCUID[2];
		pkt->rtua[3] = CCparamGlobalInfor.CCUID[3];
		pkt->rtua[4] = CCparamGlobalInfor.CCUID[4];
		pkt->rtua[5] = CCparamGlobalInfor.CCUID[5];

	}

	puc = (UCHAR *)&pkt->head;
	chk = 0;
	for(i=0; i<len; i++) chk += *puc++;

	*puc++ = chk;
	*puc = FAAL_TAIL;
	len += LEN_FAALTAIL;

	return(len);
}





/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
//UINT faal_checkpkt(UCHAR itf, faalpkt_t *pkt)
SINT basic_checkpkt(UCHAR itf, faalpkt_t *pkt)
{
	USHORT i, len;
	UCHAR *puc;
	UCHAR chk;

	len = pkt->len;

	if(len > g_faalitf[itf].rcvmax) {
		debug(DEBUG,"recv data too long\n");
		return 1;
	}

	len += LEN_FAALHEAD;
	puc = &pkt->head;
	chk = 0;

	for(i=0; i<len; i++) chk += *puc++;

	if(chk != *puc++) {
		debug(1,"chk:%#x\n",chk);
		debug(DEBUG_ERR_RECD_LINK,"FAAL_PACKAGE_CHK_ERR\n");
		return 1;
	}
	if(FAAL_TAIL != *puc) {
		debug(DEBUG_ERR_RECD_LINK,"FAAL_PACKAGE_TAIL_ERR\n");
		return 1;
	}
	return SUCCESS;
}



/*****************************************************************
** 函数名:
** 输　入: a,b,c
**　　 a---
**　　 b---
**　　 c---
** 输　出: x---
**　　　x 为 1, 表示...
**　　　x 为 0, 表示...
** 功能描述:
** 全局变量:
** 调用模块:
** 作　者:
** 日　期:
** 修　改:
** 日　期:
** 版本
****************************************************************/
/**packet format
0x68 A0 A1 A2 A3 A4 A5 0x68 C L data(L bytes) CS 0x16
**/
SINT faal_rcvpkt(UCHAR itf,UCHAR *recv)
{
	faalfsm_t *fsm = &g_faalfsm[itf];
	faalitf_t *pitf = (faalitf_t *)&g_faalitf[itf];
	faalpkt_t *pkt;
	UCHAR uc;
	SINT len1=0;
	UCHAR reportBuf[128];
	/*主动上报*/
	len1 = sizeof(reportBuf);

	while(!(*pitf->getchar)(&uc)) {
		debug(DEBUG_LOCAL_LINK_METH,"%02x ",uc);
		switch(fsm->stat) {
			case 0:
				if(FAAL_HEAD == uc) {
					basic_clrstat(itf,recv);
					*(fsm->pbuf)++ = uc;
					fsm->stat = 1;
					fsm->maxlen = 6;
					set_faaltimer(itf, pitf->timeout);
				}
				break;
			case 1:
				*(fsm->pbuf)++ = uc;
				fsm->cnt++;
				if(fsm->cnt >= fsm->maxlen)
					fsm->stat = 2;
				break;
			case 2:
				if(FAAL_HEAD != uc) {
					debug(DEBUG_RECV,"\n帧头错误2\n");
					basic_clrstat(itf,recv);
					break;
				}
				*(fsm->pbuf)++ = uc;
				fsm->stat = 3;
				fsm->cnt = 0;
				fsm->maxlen = 2;	//ctrl+len
				break;
			case 3:
				*(fsm->pbuf)++ = uc;
				fsm->cnt++;
				if(fsm->cnt >= fsm->maxlen) {
					pkt = (faalpkt_t *)recv;
					len1 = pkt->len;
					if( (len1 > pitf->rcvmax) ) {
						debug(DEBUG_RECV,"\nrecv data too long!\n");
						basic_clrstat(itf,recv);
						break;
					}
					len1 += 1;	//CS
					fsm->cnt = 0;
					fsm->maxlen = len1;

					len1 /= 200;
					len1 *= 10;
					len1 += pitf->timeout;
					set_faaltimer(itf, len1);
					fsm->stat = 4;
				}
				break;
			case 4:
				*(fsm->pbuf)++ = uc;
				fsm->cnt++;
				if(fsm->cnt >= fsm->maxlen)
					fsm->stat = 5;
				break;
			case 5:
				if(FAAL_TAIL != uc) {
					debug(DEBUG_RECV,"\n帧尾错误\n");
					basic_clrstat(itf,recv);
					break;
				}
				*(fsm->pbuf)++ = uc;
				stop_faaltimer(itf);
				pkt = (faalpkt_t *)recv;
				if(basic_checkpkt(itf, pkt)){
					debug(DEBUG_RECV,"\n接收数据未通过校验\n");
					basic_clrstat(itf,recv);
				}
				else {
					debug(DEBUG_RECV,"\n接收到完整的一帧数据\n");
					basic_clrstat(itf,recv);
					debug(DEBUG_LOCAL_LINK_METH,"recv itf=%d\n",itf);
					print_hexbuf((UCHAR*)pkt,pkt->len+12);
					return SUCCESS;
				}
				break;
			default:
				//faal_clrstat(itf,recv);
				basic_clrstat(itf,recv);
				break;
		}
	}
	return FAIL;
}


SINT faal_connect(UCHAR itf)
{
	if(g_faalitf[itf].connect()){  /*链接失败*/
		debug(DEBUG_LOCAL_LINK_METH,"socket connect through %d fail :error code is %d\n",itf,SOCKET_CONNECT_ERR);
		#if DEBUG_ERR_RECD_LINK
			debug_err_reccord(SOCKET_CONNECT_ERR);
		#endif
		return FAIL;
	}else{	/*链接成功*/
		TOPSocketConnectState = SOCKET_LINESTAT_OK;
		debug(TOPSocketConnectState,"socket connect through %d surccess \n",itf);
		return SUCCESS;
	}

}


SINT faal_disconnect(UCHAR itf)
{
	if(g_faalitf[itf].disconnect()){
		debug(DEBUG_LOCAL_LINK_METH,"socket connect through %d fail :error code is %d\n",itf,SOCKET_CONNECT_ERR);
		#if DEBUG_ERR_RECD_LINK
			debug_err_reccord(SOCKET_CONNECT_ERR);
		#endif
		return FAIL;
	}else{
		TOPSocketConnectState = SOCKET_LINESTAT_OK;
		debug(DEBUG_LOCAL_LINK_METH,"socket connect break up through %d surccess \n",itf);
		return SUCCESS;
	}
}
SINT faal_logon(UCHAR itf)
{

	if(g_faalitf[itf].logon()){
		debug(DEBUG_LOCAL_LINK_METH,"CC logon  through %d fail :error code is %d\n",itf,CC_LOGON_ERR);
        		#if DEBUG_ERR_RECD_LINK
			debug_err_reccord(CC_LOGON_ERR);
		#endif
		TOPCCRegistr2ServerState = CC_REGISTER_ERR;
		debug(DEBUG_LOCAL_ETHNET,"TOPCCRegistr2ServerState = CC_REGISTER_ERR 5\n");

		return FAIL;

	}else{
		debug(DEBUG_LOCAL_LINK_METH,"CC logon  through %d surccess \n",itf);
		return SUCCESS;
	}
}
SINT faal_linktest(UCHAR itf)
{
	if(g_faalitf[itf].linetest()){
		debug(DEBUG_LOCAL_LINK_METH,"CC linktest  through %d fail :error code is %d\n",itf,CC_LINKTEST_ERR);
		#if DEBUG_ERR_RECD_LINK
			debug_err_reccord(CC_LINKTEST_ERR);
		#endif
		TOPCCRegistr2ServerState = CC_REGISTER_ERR;
		debug(DEBUG_LOCAL_ETHNET,"TOPCCRegistr2ServerState = CC_REGISTER_ERR 6\n");
		return FAIL;
	}else{
		debug(DEBUG_LOCAL_LINK_METH,"CC linktest  through %d surccess \n",itf);
		return SUCCESS;
	}
}
SINT faal_linkstat(UCHAR itf)
{
	if(g_faalitf[itf].linestat()){
		debug(DEBUG_LOCAL_LINK_METH,"CC link  through %d is breakup:error code is %d\n",itf,CC_LINKTEST_ERR);
		#if DEBUG_ERR_RECD_LINK
			debug_err_reccord(CC_LINKTEST_ERR);
		#endif
		TOPCCRegistr2ServerState = CC_REGISTER_ERR;	//yuan lai meiyou 0516
		debug(DEBUG_LOCAL_ETHNET,"TOPCCRegistr2ServerState = CC_REGISTER_ERR 7\n");
		return FAIL;
	}else{
		TOPSocketConnectState = SOCKET_LINESTAT_OK;
		debug(DEBUG_LOCAL_LINK_METH,"CC linkstat  through %d is ok \n",itf);
		return SUCCESS;
	}
}

SINT TopUserKeepalive(UCHAR itf)
{
	if(g_faalitf[itf].keepalive()){
		return FAIL;
	}else{
		TOPSocketConnectState = SOCKET_LINESTAT_OK;
		return SUCCESS;
	}
}
