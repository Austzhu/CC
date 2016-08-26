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

#ifndef __GPRS_PPP_H__
#define __GPRS_PPP_H__
#include "include.h"
#include "serial.h"

#define OUTPUT_H 1
#define OUTPUT_L 0

#define GPRS_POWER		7
#define GPRS_RST		17
#define GPRS_IGT		19

#define WTD_OE			23
#define WTD_IN			22

//gprs infor
#define Max_Echo_Infor_Line 30
#define Max_Judge_Param_Num 6
#define Max_Infor_Return    400


#define SPLITE_LINE_RULES "\r\n"
//#define Module_Healthy "OK";
//#define Module_Echo_Error  "ERROR"
enum {
	ATI = 0,
	ATE0 = 1,
	ATD = 2,
	ATFLID_RD,
	ATFLID_WR,

	
	ATF,
	ATCGMI,
	ATCGMM,
	ATCPIN,
	ATCSQ,
	ATCGDCONT,
	ATCGACT,
	ATCGREG,
	ATSICS,
	ATSISS,
	ATSISO,
	ATSISW,
	ATSISR,
	//ATCNUM,
	ATCMGS,
	ATCIMI,
	ATCNUM ,		//

	ATCSCS,
	
	ATCMGF,
	ATCOPS,
	ATCNMI,
	ATCIND,

};



typedef struct AtCMDs{
	 CHAR CodeType; 
	 CHAR Cmd[60];
	 CHAR *P_TheoryInforReceiveKeys[Max_Judge_Param_Num]; //InforReceiveKeys(zhizsz)
	 CHAR ComplateInforReceive[Max_Infor_Return]; //all recived
	 CHAR *P_Splited_Infor[Max_Echo_Infor_Line];		//逐条分割后的小  buf指针
}AtCMDs;


UCHAR SingnalDBMGetFunc(void);


SINT Delay_ms(SINT times);

SINT Gprs_StartUp(void);
SINT Gprs_Rst(void);


SINT Gprs_TurnOff(void);

SINT GPRS_ATCMD_INTER(char *cmds,char *ComplateInforReceive,char *P_Splited_Infor_PerLine[Max_Echo_Infor_Line],char *judg_code[Max_Judge_Param_Num]);
SINT Point2StartOfEveryLine(char *Src,char *P_Spite[Max_Echo_Infor_Line],char *Delim);
SINT WetherGetInforSuccess(char *buf, char *at_cmd_num[Max_Judge_Param_Num]);
SINT SMS_MODULE_INIT_SET_UpLoad(UCHAR *Buf,UCHAR ActRequCMD,UCHAR ActRequSubCMD,UCHAR *Telphone);

extern SINT GPRS_GSM_Call(CHAR *TelphoNum,CHAR *Infor_Back_Bufs);
#endif
