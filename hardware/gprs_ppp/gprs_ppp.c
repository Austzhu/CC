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
#include "gprs_ppp.h"

AtCMDs AtCMDs_ACT[]={
	{ATI,"ati\r",{"01.301","ERROR","OK"}},					//ok
	{ATE0,"ATE0",{"ERROR","OK"}},							//ok
	{ATD,"",{"DIALTONE","BUSY","CARRIER","CONNECT"}},		//ok
	{ATFLID_RD,"at+flid?\r",{"ERROR","OK"}},				//cha kan
	//{ATCNUM,"at+cnum\r",{"+CNUM:","ERROR","OK"}},		//see phone book
	//{ATFLID,"at+flid?\r",{"ERROR","OK"}},//cha kan
	{ATFLID_WR,"at+flid=",{"ERROR","OK"}},//set local num
	{ATF,"at&f\r",{"ERROR","OK"}},
	{2,"at+cgmi\r",{"ERROR","OK"}},
	{3,"at+cgmm\r",{"ERROR","OK"}},
	{4,"at+cpin?\r",{"+cpin:","ERROR","OK"}},
	{5,"at+csq\r",{"+csq:","ERROR","OK"}},
	{6,"at+cgdcont=1,ip,cmnet\r",{"ERROR","OK"}},
	{7,"at+cgact=1,1\r",{"ERROR","OK"}},
	{8,"at+cgreg=1\r",{"ERROR","OK"}},
	{9,"at+cgreg?\r",{"+cgreg:","ERROR","OK"}},
	//{10,"",{"no","ok","NO CARRIER","ARRIER"}},
	/****************startlogin******************/
	{10,"at^sics = 0,conType,gprs0\r",{"ERROR","OK"}},
	{11,"at^sics = 0,user,cm\r",{"ERROR","OK"}},
	{12,"at^sics = 0,passwd,gprs\r",{"ERROR","OK"}},
	{13,"at^sics = 0,apn,cmnet\r",{"ERROR","OK"}},
	{14,"at^siss=1,srvType,socket\r",{"ERROR","OK"}},
	{15,"at^siss=1,conId,0\r",{"ERROR","OK"}},
	{16,"at^siss=1,address,socktcp://115.236.70.154:60000\r",{"error","ok"}},
	{17,"at^siso=1\r",{"ERROR","OK"}},
	{18,"at^sisw=1,6\r",{"^sisw:","ERROR","OK"}},
	{19,"at^sisr=1,1024\r",{"^sisr:","ERROR","OK"}},
	/****************loginout*******************/
	{20,"at^sisc=1\r",{"^smso:","ERROR","OK"}},
	{21,"at^smso\r",{"^smso:","ERROR","OK"}},
	{23,"at+cmgs=27\r",{">","ERROR","OK"}},
	{24,"at+cimi\r",{"ERROR","OK"}},
	{26,"at+cscs?\r",{"+CSCS:","ERROR","OK"}},
	{27,"at+cmgf=0\r",{"+CMGF:","ERROR","OK"}},
	{28,"at+cops?\r",{"+COPS:","ERROR","OK"}},
	{29,"at+cnmi=2,1\r",{"+CNMI:","ERROR","OK"}},	//sms STOR IN CARD
	{30,"at+cind?\r",{"+CIND:","ERROR","OK"}},				 
};

UCHAR SingnalDBMGetFunc(void)
{

	return 0x01;
}

SINT Delay_ms(SINT times)
{
	usleep(times*1000);
	return SUCCESS;
}

SINT Gprs_StartUp()
{
	SINT fd,rt;
	#define DEVICE_NAME  "/dev/io_gprs_wd"

	fd=open(DEVICE_NAME,O_RDONLY);
	//err test
	if (fd == -1){
		printf("open DEVICE_NAME err\n");
	}

	ioctl(fd,OUTPUT_H,GPRS_POWER);
	ioctl(fd,OUTPUT_L,GPRS_IGT);
	ioctl(fd,OUTPUT_H,GPRS_RST);

	Delay_ms(1000);
	ioctl(fd,OUTPUT_H,GPRS_IGT);

	Delay_ms(3);	//> 10ms

	ioctl(fd,OUTPUT_L,GPRS_RST);
	//ioctl(fd,OUTPUT_L,GPRS_IGT);

	Delay_ms(20);	//> 10ms
	ioctl(fd,OUTPUT_L,GPRS_IGT);

	rt = close(fd);
	printf("rt is %d,and closed\n",rt);

 return SUCCESS;

}
SINT Gprs_Rst()
{
	#define DEVICE_NAME  "/dev/io_gprs_wd"
	SINT fd,rt;

	fd=open(DEVICE_NAME,O_RDONLY);
	//err test
	if (fd == -1){
		printf("open DEVICE_NAME err\n");
	}

	ioctl(fd,OUTPUT_L,GPRS_RST);
	Delay_ms(20);	//>10ms
	ioctl(fd,OUTPUT_H,GPRS_RST);

	rt = close(fd);
	printf("rt is %d,and closed\n",rt);

	return SUCCESS;
}

SINT Gprs_TurnOff()
{
	//at instruct  needed 
	return SUCCESS;
}

SINT GPRS_GSM_Call(CHAR *TelphoNum,CHAR *Infor_Back_Bufs)
{
	//char buf_s[512];
	//atcmds atcmds_call;
	//char *jug_cod[4] = {"no","ok","NO CARRIER","ARRIER"};
	sprintf(AtCMDs_ACT[ATD].Cmd,"atd%s;\r",TelphoNum);
	
	//APEND TO TASK LIST
	printf("in function GPRS_GSM_Call and AtCMDs_ACT[ATD].Cmd is %s\n",AtCMDs_ACT[ATD].Cmd);

	Uart_Send(Uart1_ttyS2_GPRS,AtCMDs_ACT[ATD].Cmd,strlen(AtCMDs_ACT[ATD].Cmd ),  0);

	//appuart_send(UART1_TTYS2_GPRS, AtCMDs_ACT[ATD].Cmd);
	printf("after send\n");
	
	sleep(3);
	while(1)
	{
		
		//appuart_rev(UART1_TTYS2_GPRS, Infor_Back_Bufs);
		Uart_Recv(Uart1_ttyS2_GPRS, Infor_Back_Bufs,400,100000);
		
		if(strlen(Infor_Back_Bufs)>1){
			printf("strlen(Infor_Back_Bufs)>1: %s \n",Infor_Back_Bufs);
			break;
		}	
	}
	return 0;
}



SINT GPRS_ATCMD_INTER(char *cmds,char *ComplateInforReceive,char *P_Splited_Infor_PerLine[Max_Echo_Infor_Line],char *P_TheoryInforReceiveKeys[Max_Judge_Param_Num]) 
{
	int i = 0;
	int LineNums = 0;
	CHAR Echo_Errors = 0;

	while(Echo_Errors < 3){
		Uart_Send(Uart1_ttyS2_GPRS,cmds,strlen(cmds),  0);
		//appuart_send(UART_NUM, cmds);
		
		
		debug(DEBUG_LOCAL_GPRS_ATSEND,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\nat_cmd_atall send: %sXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n",(char*)P_TheoryInforReceiveKeys);

		


		usleep(400000);
		
		Uart_Recv(Uart1_ttyS2_GPRS, ComplateInforReceive,400,100000);

		//appuart_rev(UART_NUM, ComplateInforReceive);

		debug(DEBUG_LOCAL_GPRS_ATSEND,"at_cmd_atall ComplateInforReceive: %s:\n",ComplateInforReceive);
			

		
			//function: 依照空格键 、回车键 逐条分割到c_ext[1024]
			//delim 分割依据	//return :分割后条数

			//i_cmds = char_ext(ComplateInforReceive,P_Splited_Infor_PerLine,delim_t);
		LineNums = Point2StartOfEveryLine(ComplateInforReceive,P_Splited_Infor_PerLine,
													(char*)P_TheoryInforReceiveKeys);


		
		for(i = 0;i<= LineNums;i++){
			debug(DEBUG_LOCAL_GPRS_ATSEND,"LineNums is %d P_Splited_Infor_PerLine[%d] is %s\n",LineNums,i,P_Splited_Infor_PerLine[i]);
		}
		


		if (WetherGetInforSuccess(ComplateInforReceive,P_TheoryInforReceiveKeys)){
			printf("Error in WetherGetInforSuccess\n");
			Echo_Errors++;
		}
		else{
			printf("befor return success Echo_Errors is %d\n",Echo_Errors);
            return SUCCESS;
		}
	  
	}//end of while

	return FAIL;
}

SINT Point2StartOfEveryLine(char *Src,char *P_Spite[15],char *Delim)
{
	char *s = Src;
	char *p= Src;
	char *p_space;
	//char *p_test = Src;
	
	int i=0;


	//\r\n*****\r\n*****\\r\n****\0
	printf("src is %s\ns is %s\n",Src,s);

	
	//char src[]="\r\n123 456 789 \r\n356\r\n333\r\nOK\r\n555\r\n\0";

	while(p){
		//s = s+1;
		p=(strstr(s,Delim));
		if((p != NULL)&&(*(p+2) != '\0'))
		{
			
			p_space = p;
			while((*p_space == ' ')||(*p_space == 0x0d)||(*p_space == 0x0a)){
				p_space++;
			}

		   	P_Spite[i] = (p_space);
		    s = (p_space);
			
			printf("p_spite[%d] is %s\ns is %s\n",i,P_Spite[i],s);

			i++;
			
		  printf("in if((p != NULL)&&(*(s+2) != '\\r')&&(*(s+2) != '\\0')) \n");
		}

		else{
			break;
		}
		printf("btween && p is %s\n",p);

	}
	
	printf("sizeof(p_spite[]) is %d\n",sizeof(P_Spite));

	return i;

}

SINT WetherGetInforSuccess(char *buf, char *TheoryEchoBackInfor[Max_Judge_Param_Num])
{
	int i=0;
	//int len_cmd;

	char *Module_Healthy="OK";
	char *Module_Echo_Error ="ERROR";

	//fast pass
	if((strstr(buf,Module_Healthy))){

		printf("Module_Healthy\n");
		return SUCCESS;
	}

	if((strstr(buf,Module_Echo_Error))){
		printf("Module_Echo_Error\n");
		return FAIL;
	}


	while((i < Max_Judge_Param_Num)&&(TheoryEchoBackInfor[i] != NULL)){

		if ((strstr(buf,TheoryEchoBackInfor[i]) != 0)&&(!strcmp(TheoryEchoBackInfor[i],Module_Echo_Error))){
			printf("(strstr(buf,TheoryEchoBackInfor[i]) != 0) && TheoryEchoBackInfor[%d] is %s\n",i,TheoryEchoBackInfor[i]);
			return SUCCESS;
		}
		i++;
	}

	printf("SEARCH FOR PARAM ERROR\n");	
	return FAIL;
}
