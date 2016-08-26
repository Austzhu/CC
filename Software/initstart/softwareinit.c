#include "softwareinit.h"

GlobalCCparam CCparamGlobalInfor;

extern BurglarAlarmGroupGL 					BurglarAlarmElemList;
extern struct TimerTaskToSaveFormatStruct 	TimerTaskList[TimerTaskQuatityMax];
extern volatile UCHAR TOTAL_TASK_IN_EACH_QUEUE[QUEUE_TOTAL_QUANTITY];
extern volatile UCHAR TOPDeviceOprateRetryTMS[QUEUE_TOTAL_QUANTITY];
extern volatile SINT TOPSocketConnectState;						//socket connect state  step-1
extern volatile SINT TOPCCRegistr2ServerState;					//cc register state  step-1
extern struct MetterElecInforStruc TOPMetterElecInformation;
extern volatile UCHAR TOP_SERVERREQUI_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];	
extern volatile UCHAR TOP_LOCALREALISTIC_DIDOCIR_STAT[DIDO_DEVICE_QUANTITY];			
extern volatile UCHAR TOP_LOCALREALISTIC_DIINFOR_STAT[DIDO_DEVICE_QUANTITY];	
extern volatile SINT TOPGLOBALMEMALLOCTIMES;
extern volatile SINT TOPGLOBAL_HEAP_SIZE;
extern CHAR TOPDIDO_STATUS[DIDO_DEVICE_QUANTITY];
extern CHAR TOPMETER_STATUS[METER_DEVICE_QUANTITY];
extern volatile CHAR TOPSHOWPERTICK;

#define FILE_MAX_SIZE  1024*5
#define LEFT_START_BRACE '['
#define RIGHT_END_BRACE ']'
#define FILE_PARAMFOR_TIMERTASK 		"./taskoftime.data"
#define FILE_PARAMFOR_NODELIST 		"./node.data"
#define FILE_PARAMFOR_SYSTERMINI 		"./fileinit.ini"

enum{
	FLAG_PARAMFOR_TIMERTASK,		//0
	FLAG_PARAMFOR_NODELIST,		//1
	FLAG_PARAMFOR_SYSTERMINI,		//2
};

struct TimeTaskHeadStruct{
	USHORT FileMagic;
	USHORT FileCrc;
};

//typedef struct FileParaManagementStruct{

struct FileParaManagementStruct{
	UCHAR *Address;
	SINT FileSize;
	CHAR *FileName;
};

struct FileParaManagementStruct FileActExistList[]={
		//{(UCHAR *)&CCparamGlobalInfor,sizeof(CCparamGlobalInfor),FILE_PARAMFOR_SYSTERMINI},//0
	{(UCHAR *)&TimerTaskList,sizeof(TimerTaskList),FILE_PARAMFOR_TIMERTASK},		   //1

	{NULL,0,FILE_PARAMFOR_NODELIST},
	//{(UCHAR *)&SingleLampList,sizeof(SingleLampList),FILE_PARAMFOR_NODELIST},

	{(UCHAR *)&CCparamGlobalInfor,sizeof(CCparamGlobalInfor),FILE_PARAMFOR_SYSTERMINI},//0
	
};

#define FILE_PARAMFOR_TIMERTASK_SAV		0x01
#define FILE_PARAMFOR_NODELIST_SAV		0x02
#define FILE_PARAMFOR_SYSTERMINI_SAV	0x04



UINT ArgcParamAnlys()
{
	return SUCCESS;
}

UINT TOPDeviceInforShowInit()
{
	UCHAR ALLCHANLS=0xFF;
	UCHAR INITStat;

	struct MetterElecInforStruc *RealTimeElecInforInit;
	RealTimeElecInforInit = &TOPMetterElecInformation;

	//read meter
	if(!MetterRealTimeRDOprate(METER_START_ADDR,ALLCHANLS,RealTimeElecInforInit)){
		TOPMETER_STATUS[METER_START_ADDR - METER_START_ADDR] = SUCCESS;
	}
	//read DIDO--output
	if(!DIDOOutInputStatQuery(DIDI_START_ADDR,ALLCHANLS,DIDO_QUERY_OUTPUT,&INITStat)){
	    // printf(" it is in TOPDeviceInforShowInit\n");
		 TOP_LOCALREALISTIC_DIDOCIR_STAT[(DIDI_START_ADDR - DIDI_START_ADDR)] = INITStat;

		 TOPDIDO_STATUS[DIDI_START_ADDR - DIDI_START_ADDR] = SUCCESS;

	}
	
	//read DIDO--input			 	
	if(!DIDOOutInputStatQuery(DIDI_START_ADDR,ALLCHANLS,DIDO_QUERY_INPUT,&INITStat)){
		// printf(" it is in TOPDeviceInforShowInit ;OutPutstat is %x\n",INITStat);
   		 TOP_LOCALREALISTIC_DIINFOR_STAT[(DIDI_START_ADDR-DIDI_START_ADDR)] = INITStat;
		 TOPDIDO_STATUS[DIDI_START_ADDR-DIDI_START_ADDR] = SUCCESS;

	}
		
	return SUCCESS;

}

UINT TOPDetailTaskInforShow()
{	
	UCHAR tt = 0;
	
	debug(DEBUG_LOCAL_GLOBAL_INFOR,"************************************************* DETAIL TASK INFOR *************************************************\n");
	for(tt=0;tt<5;tt++){		
		debug(DEBUG_LOCAL_GLOBAL_INFOR,"taskID[%d] is: %x:%x    ",tt,TimerTaskList[tt].TaskID[0],TimerTaskList[tt].TaskID[1]);
		debug(DEBUG_LOCAL_GLOBAL_INFOR,"CMD: %x, Contex:%x:%x:%x:%x:%x:%x:%x    TaskBegin: %ld    TaskEnd: %ld\n",TimerTaskList[tt].TaskCMDLen,TimerTaskList[tt].TaskCMDPakt[0],
			TimerTaskList[tt].TaskCMDPakt[1],TimerTaskList[tt].TaskCMDPakt[2],TimerTaskList[tt].TaskCMDPakt[3],
				TimerTaskList[tt].TaskCMDPakt[4],TimerTaskList[tt].TaskCMDPakt[5],TimerTaskList[tt].TaskCMDPakt[6],\
					TimerTaskList[tt].TaskBegin,TimerTaskList[tt].TaskEnd);
	}debug(DEBUG_LOCAL_GLOBAL_INFOR,"\n");
	return SUCCESS;
}

UINT DefineParamLoad()
{
	ParamFileFormtLoad();
	//ParamSqliteFormtLoad();

	return SUCCESS;
}
UINT DownLoadTaskLoad()
{	

	TopFileParamLoad(0);		//for TimerTask
	//TopFileParamLoad(1);		//for LEDNODE

	return SUCCESS;
}

UINT DefineParamCheck()
{
	//UartParamCheck();

	return SUCCESS;
}


SINT ParamFileFormtLoad()
{
	FileParamLoad();
	//FileParamLoadedResultPrint();
	return SUCCESS;

}


SINT IsRightBrach(CHAR c)
{
	return RIGHT_END_BRACE == c ? 1 : 0;

}
SINT EndofFileBufstr(CHAR c)
{
	return '\0' == c ? 1 : 0;

}
SINT IsNewLine(CHAR c)
{
	return ('\n' == c || '\r' == c)? 1:0;

}
SINT IsLeftBarch(CHAR c)
{
	return LEFT_START_BRACE == c ? 1 : 0;

}



SINT FileLoad2buf(const CHAR *file,CHAR *filebuf,SINT *filesize)
{
	FILE *input=NULL;
	SINT i=0;
	*filesize = 0;
	assert(file != NULL);
	assert(filebuf != NULL);


	input = fopen(file,"r");
	if(input == NULL){
		debug(DEBUG_LOCAL_FILEPARAM_LOAD,"打开文件失败\n");
		return PARAM_FILE_NOT_EXIST;
	}
	//else{}
	filebuf[i] = fgetc(input);
	while(filebuf[i] != (CHAR)EOF){
		i++;
		assert(i < FILE_MAX_SIZE);
		filebuf[i] = fgetc(input);

	}
	filebuf[i] = '\0';
	*filesize = i;
	fclose(input);
	return SUCCESS;
}


SINT ParamFindInFileBuf(const CHAR *section,const CHAR *key,const CHAR *filebuf,
		SINT *sectionSt,SINT *sectionEd,SINT *keySt,SINT *keyEd,SINT *valueSt,SINT *valueEd)
{
	const CHAR *p = filebuf;
	SINT i=0;
	assert(filebuf != NULL);
	assert((section != NULL)&&(strlen(section)));
	assert((key != NULL)&&(strlen(key)));

	*sectionEd = *sectionSt = *keyEd = *keySt = *valueEd = *valueSt = -1;
	//debug(DEBUG_LOCAL_FILEPARAM_LOAD,"p[%d] is %c\n",i,p[i]);
	while(!EndofFileBufstr(p[i])){
		if(((i == 0)||(IsNewLine(p[i-1])))&&(IsLeftBarch(p[i]))){
			SINT secttempstart = i+1;
			debug(DEBUG_LOCAL_FILEPARAM_LOAD,"in while p[%d] is %c\n",i,p[i]);			

			do{	
				//debug(DEBUG_LOCAL_FILEPARAM_LOAD,"in do p[%d] is %c\n",i,p[i]);			
				i++;
			}while((!IsRightBrach(p[i]))&&(!EndofFileBufstr(p[i])));	//end of do
			

			#if DEBUG_LOCAL_FILEPARAM_LOAD
				CHAR tmp[15];
				printf("after do && section is %s\n",section);
				printf("i = %d,secttempstart=%d i-secttempstart =%d\n",
										i,secttempstart,i-secttempstart);
				strncpy(tmp,p+secttempstart,i-secttempstart);
				printf("tmp is %s\n",tmp);
			#endif

			//process FIHead	&& 必须满足
			if(0 == strncmp(p+secttempstart,section,i-secttempstart)){
				debug(DEBUG_LOCAL_FILEPARAM_LOAD,"key is %s.....FIHead eque\n",key);
				SINT NewLineStart = 0;
				i++;
				while(isspace(p[i])){
					i++;
				}
				//find the section
				*sectionSt = secttempstart;
				*sectionEd = i;

				while(!((IsNewLine(p[i-1]))&&(IsLeftBarch(p[i])))&&(!EndofFileBufstr(p[i]))){
					SINT j = 0;
					NewLineStart = i;

					//while((!IsNewLine(p[i-1]))&&(!EndofFileBufstr(p[i]))){
					while((!IsNewLine(p[i]))&&(!EndofFileBufstr(p[i]))){
						i++;
					}

					j=NewLineStart;
					if(';' != p[j]){
						while((j<i)&&(p[j] != '=')){
							j++;
							if('=' == p[j]){
								if(strncmp(key,p+NewLineStart,j-NewLineStart) == 0){
									debug(DEBUG_LOCAL_FILEPARAM_LOAD,"key eque\n");
									*keySt = NewLineStart;
									*keyEd = j-1;
									*valueSt = j+1;
									*valueEd = i;
									return SUCCESS;
								}
							}//end of if('=' == p[j]){
						}//end of while((j<i)&&(p[j] != '=')){
					}//END OF IF(';' != P[j])

					i++;

				}	//end of while((!IsNewLine(p[i-1]))&&
				
			}	//end of if(0 == strcmp(p+secttemps...
			

		}	//end of if(((i == 0)||(IsNewLine(p[i-1])))&&(IsLeftBarch(p[i])))
		else{
			i++;
		}

	}//end of while(!EndofString())
	return FAIL;
}


/*************************************************
Copyright (C), 2011-2018, FDT. Co., Ltd.
Author:				 // wang_guilin 
Version:			 // v1.0.0 
Date: 				//2011_07_05
Chip:				//atmel_9260
File name: 			//softwareinit.c
Function:			//read_profile_string( const char *section, const char *key,char *value, 
										 int size, const char *default_value, const char *file)
					//  
Description: 		//读取*file文件内容， 成功的话将*key参数赋值给char *value,  否则将*default_value付给value 值
Calls: 								// 被本函数调用的函数清单
Called By: 			//
Table Accessed:		 // 被访问的表（此项仅对于牵扯到数据库操作的程序）
Table Updated: 		// 被修改的表（此项仅对于牵扯到数据库操作的程序）
Input:				// 输入参数说明，包括每个参数的作
      				 // 用、取值说明及参数间关系。
Output: 			//

Return:				 // 1 -----------not find the key
					 // 0 -----------have find the key

Others: 								// 其它说明
History: 			// 修改历史记录列表如下
1.Date:				// 修改日期2011_07_05
Author:			 	//wang_guilin
Modification:		

function  : ProfileStringRead("term","ConnectType",val,sizeof(val),"2",ini_term);

*************************************************/

SINT ProfileStringRead(const CHAR *section,const CHAR *key,char *value,int size,CHAR *DefaultValue,const CHAR *file)
{
	CHAR filebuf[FILE_MAX_SIZE]={0};
	SINT filesize;
	SINT valuecnt;
	SINT sectSt,sectEd,keySt,keyEd,valueSt,valueEd;
	//param assert ,if error break;
	assert((section != NULL)&&(strlen(section)));
	assert((key != NULL)&&(strlen(key)));
	assert(value != NULL);
	assert(size > 0);
	assert((file != NULL)&&(strlen(file)));

	//debug(DEBUG_LOCAL_FILEPARAM_LOAD,"XXXXXXXXXXXProfileStringReadXXXXXXXXXXXXXXXXX\n");
	//FileParamLoadedResultPrint();
	
	if(FileLoad2buf(file,filebuf,&filesize)){
		if(DefaultValue !=NULL){
			strncpy(value,DefaultValue,size);
		}
		debug(DEBUG_LOCAL_FILEPARAM_LOAD,"PARAM_FILE_LOAD_ERR\n");
		return PARAM_FILE_LOAD_ERR;
	}
	
	//filebuf parse
	if(ParamFindInFileBuf(section,key,filebuf,&sectSt,&sectEd,&keySt,&keyEd,&valueSt,&valueEd)){
		
		if(DefaultValue !=NULL){
			strncpy(value,DefaultValue,size);
		}
		debug(DEBUG_LOCAL_FILEPARAM_LOAD,"PARAM_FILE_FIND_ERR\n");
		return PARAM_FILE_FIND_ERR;
	}else{
		valuecnt = valueEd - valueSt;
		if(valuecnt > (size-1)){
			valuecnt = (size - 1);
		}

		memset(value,0,sizeof(value));
		memcpy(value,(filebuf+valueSt),valuecnt);
		value[valuecnt] = '\0';
		return SUCCESS;
	}

}

#if 1
UINT test()
{
	BurglarAlarmElemenent *BuglarElem;
	BurglarAlarmElemenent tttt;
	{
	tttt.UID[0]= 0x0a;tttt.UID[1]= 1;tttt.UID[2]= 2;tttt.UID[3]= 3;tttt.UID[4]= 4;tttt.UID[5]= 6;
	tttt.DeviceAddr =1;
	tttt.BurglarType =2;
	tttt.Channel = 3;
	tttt.ChannelHardCirc = 4;
	tttt.flag =5;
	tttt.LineName[0] = '1';tttt.LineName[1] = '2';tttt.LineName[2] = '3';
	tttt.LineName[3] = '3';tttt.LineName[4] = '2';tttt.LineName[5] = '\0';

	tttt.WireLineState = 7;
	tttt.NetSend2User =8;
	tttt.SMSSend2User =9;
	tttt.ExterInfor1 = 10;
	tttt.ExterInfor2 = 110119;
	}
	BuglarElem= &tttt;

//BurglarAlarmElemenentTest noattr;
//struct 
//	BurglarAlarmElemenent hasattr;
//printf("sizeof(UINT) is %d,sizeof(char) is %d\n",sizeof(UINT),sizeof(CHAR));

//printf("sizeof(BurglarAlarmElemenentTest) is %d,sizeof(BurglarAlarmElemenent) is %d\n",sizeof(hasattr),sizeof(noattr));
//	printf("before del\n");
//	DeleteBurglarAlarmTable();		//ok

	InserBurglarInforTable(BuglarElem);

	BurglarAlarmElemTableLoad();

	BurglarAlarmElemInforPrint(BurglarAlarmElemList.BurglarAlarmList);


	//BurglarAlarmElemInforPrint(BurglarAlarmElemList.BurglarAlarmList);

	//char *str="123456789";
	//char i[20];
	//String2Bytes(str,i,strlen(str));

	//printf("str is %s\n",str);
	//printf("i is %s\n",i);

	//printf("ssss is %d:%d:%d:%d::%d:%d\n",i[0],i[1],i[2],i[3],i[4],i[5]);

return SUCCESS ;
}
#endif


SINT ParamSqliteFormtLoad()
{
	if(!test()){
		return SUCCESS;
	}
	return PARAM_SQLITE_LOAD_ERR;
}


SINT ProfileIntRead(const CHAR *section,const CHAR *key,SINT defaltvalue,const CHAR *file)
{
	char value[32] = {0};
	//if(!ProfileStringRead(section,key,value, sizeof(value),NULL,file))
	if(ProfileStringRead(section,key,value, sizeof(value),NULL,file)){
		return defaltvalue;
	}
	else
	{
		return atoi(value);
	}
}

SINT ProfileStringWrite(const CHAR *section,const CHAR *key,const char *value,const CHAR *file)
{
	
	
	return SUCCESS;
}

SINT ProfileIntWrite(const CHAR *section,const CHAR *key,char *value,CHAR *DefaultValue,const CHAR *file)
{

	return SUCCESS;

}

SINT BCDToHex(UCHAR *src, UCHAR *dst, SINT lens)

{	
	UINT len;
	//unsigned char *tmp_uid=src;
	//CHAR *tmp_uid=src;
	UCHAR *tmp_uid=src;

	UCHAR buf[3]={0,0,0};
	
	unsigned int uid[6];		//20120704 test	ok    :0:0:0:0:c1:21
	
	int i,j,m,n;
	len=strlen((CHAR *)tmp_uid);
	
	for(i=0;i<len;i++)
		
		if(!((tmp_uid[i]>='0'&&tmp_uid[i]<='9')||(tmp_uid[i]>='a'&&tmp_uid[i]<='f')|| (tmp_uid[i]>='A'&&tmp_uid[i]<='F'))){
			strcpy((CHAR *)tmp_uid,"303133353032");
			break;
		}
		
	if(len<12)
	{
		for(i=len-1;i>=0;i--)
			tmp_uid[i+12-len]=tmp_uid[i];
		
		for(i=0;i<12-len;i++)
			tmp_uid[i]='0';
	}
	
	//printf("tmp_uid=%s\n",tmp_uid);
	
	for(m=0,i=0;i<12;m++){
		for(n=i,j=i;j<i+2;j++){
			buf[j-i]=tmp_uid[n++];
		}
		
		i=i+2;
		//sscanf((CHAR *)buf,"%x",(CHAR *)&uid[m]);
		sscanf((char*)buf,"%x",&uid[m]);
	}
	
	for(j=0;j<6;j++){
		dst[j]=(UCHAR)uid[j];
	}
	//end ke
	return lens/2;
}

UINT TOPPowerStatInforShow()
{


		//POWER stat
		printf("**************************************************** POWER SUPPLY Stat *****************************************************\n");
		printf("METER STATUS %d  (1:err  0-ok)\n",TOPMETER_STATUS[0]);
		printf("    I1 / I2                I3 / I4             I5 / I6             UA / Uab            Ub / bc             Uc / Uac \n");

		printf("     %3.2f / %3.2f            %3.2f / %3.2f         %3.2f / %3.2f         %3.2f / %3.2f      %3.2f / %3.2f     %3.2f / %3.2f \n ",\
			TOPMetterElecInformation.Ia,TOPMetterElecInformation.Ib,TOPMetterElecInformation.Ic,TOPMetterElecInformation.Id,TOPMetterElecInformation.Ie,TOPMetterElecInformation.If, \
			TOPMetterElecInformation.Ua,TOPMetterElecInformation.Uab,TOPMetterElecInformation.Ub,TOPMetterElecInformation.Ubc,TOPMetterElecInformation.Uc,TOPMetterElecInformation.Uca);

		printf("   P1 / Q1                P2 / Q2             P3 / Q3             P4 / Q4             P5 / Q5             P6 / Q6 \n");

		printf("     %3.2f / %3.2f            %3.2f / %3.2f         %3.2f / %3.2f         %3.2f / %3.2f         %3.2f / %3.2f         %3.2f / %3.2f \n ",\
			TOPMetterElecInformation.Pa,TOPMetterElecInformation.Qa,TOPMetterElecInformation.Pb,TOPMetterElecInformation.Qb,TOPMetterElecInformation.Pc,TOPMetterElecInformation.Qc, \
			TOPMetterElecInformation.Pd,TOPMetterElecInformation.Qd,TOPMetterElecInformation.Pe,TOPMetterElecInformation.Qe,TOPMetterElecInformation.Pf,TOPMetterElecInformation.Qf);

		printf("   S1 / PF1               S2 / PF2            S3 / PF3            S4 / PF4            S5 / PF5            S6 / PF6  \n");
		printf("     %3.2f / %3.2f            %3.2f / %3.2f         %3.2f / %3.2f          %3.2f / %3.2f        %3.2f / %3.2f         %3.2f / %3.2f \n",\
			TOPMetterElecInformation.Sa,TOPMetterElecInformation.PFa,TOPMetterElecInformation.Sb,TOPMetterElecInformation.PFb,TOPMetterElecInformation.Sc,TOPMetterElecInformation.PFc,\
			TOPMetterElecInformation.Sd,TOPMetterElecInformation.Qd,TOPMetterElecInformation.Se,TOPMetterElecInformation.PFe,TOPMetterElecInformation.Sf,	TOPMetterElecInformation.PFf);

		printf("    I_tatol                S                   PF                  KWH                 KVARH               KVAH  \n");
		printf("     %3.2f                   %3.2f                %3.2f                 %3.2f               %3.2f                %3.2f\n",\
			TOPMetterElecInformation.I,TOPMetterElecInformation.S,TOPMetterElecInformation.PF,TOPMetterElecInformation.kWh,TOPMetterElecInformation.kvarh,TOPMetterElecInformation.kVAh);

		printf("\n");
		return SUCCESS;
}


UINT TOPDIDOStatInforShow()
{
		s32 DeviceCon = 0;
		
		//DIDO stat
		for (DeviceCon = 0;DeviceCon<(DIDO_DEVICE_QUANTITY);DeviceCon++){
			printf("************************************************ DIVICE%d DIDO CURRENT STAT **************************************************\n",DeviceCon);
			printf("DEVICE_DIDO_%d STATUS  %d (1:err  0-ok)\n",DeviceCon,TOPDIDO_STATUS[DeviceCon]);

			printf("Device_%d DO STAT:  (LED_ON-1;LED_OFF-0)                          Device_%d DI STAT:\n",DeviceCon,DeviceCon);
			
			printf("   DO_R1/DO_L1 : DO2 : DO3 : DO4 : DO5 : DO6 : DO7 : DO8              CIR_1: C2: C3: C4: C5: C6: C7: C8\n");

			printf("      %d/%d :      %d/%d : %d/%d : %d/%d : %d/%d : %d/%d : %d/%d : %d/%d                  %d:  %d:  %d:  %d:  %d:  %d:  %d:  %d  \n ",\
				(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x01) ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x01),(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x02)>>1 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x02)>>1,\
				(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x04)>>2 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x04)>>2,(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x08)>>3 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x08)>>3,\
				(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x10)>>4 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x10)>>4,(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x20)>>5 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x20)>>5,\
				(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x40)>>6 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x40)>>6,(TOP_SERVERREQUI_DIDOCIR_STAT[DeviceCon]&0x80)>>7 ,(TOP_LOCALREALISTIC_DIDOCIR_STAT[DeviceCon]&0x80)>>7,\

				(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x01) ,(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x02)>>1 ,\
				(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x04)>>2 ,(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x08)>>3,\
				(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x10)>>4 ,(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x20)>>5,\
				(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x40)>>6 ,(TOP_LOCALREALISTIC_DIINFOR_STAT[DeviceCon]&0x80)>>7);

			printf("\n");
	
		}


		return SUCCESS;
}

UINT TOPGlobalInforPrintAndSave(void)
{
	UCHAR TaskQueueQuantity;
	static CHAR ShowFreq = 0;
	//total task infor show
	if (TOPSHOWPERTICK){
		ShowFreq++;
	}
	
	if (ShowFreq > 200){
		ShowFreq = 0;
		system("clear");


		//cc stat
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX CC rigister stat relate AND MEMERY RELATE  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("SocketConnect     CCRegist     G_MEMALLOCTIMES     G_HEAP_SIZE     MemeryOccupy\n");

		printf("    %d                %d              %d                 %d                 %d     \n \n",\
			TOPSocketConnectState,TOPCCRegistr2ServerState,TOPGLOBALMEMALLOCTIMES,TOPGLOBAL_HEAP_SIZE,((TOPGLOBAL_HEAP_SIZE)/(53*10000)));

		//device retry times
		printf("************************************************ DeviceAnd Task Queue Stat *************************************************\n");
		printf("QUEUE:\n PLC-0      GPRS-1      CC-2      485-3      ETH-4\n");

		for (TaskQueueQuantity=0;TaskQueueQuantity<QUEUE_TOTAL_QUANTITY;TaskQueueQuantity++){
			
			printf("Device/Queue: %d         RetryTimes %d       TaskQuantity %d\n",TaskQueueQuantity,TOPDeviceOprateRetryTMS[TaskQueueQuantity],TOTAL_TASK_IN_EACH_QUEUE[TaskQueueQuantity]);
		}
		printf("\n");

		//power stat infor show
		TOPPowerStatInforShow();

		//dido stat infor show
		TOPDIDOStatInforShow();
		
		//DetailTaskInforShow
		TOPDetailTaskInforShow();

		printf("\n");
		//printf("\n");
		sleep(1);

	}
	return SUCCESS;
}

UINT SetDebugShowLvl(UCHAR i)
{

	return i;
}


SINT FileParamLoad(void)
{
	CHAR val[128];
	SINT i;

	UCHAR tmp_uid[30];
	
	memset( &CCparamGlobalInfor, 0, sizeof(CCparamGlobalInfor) );

	ProfileStringRead("FIHead","CCUID",(CHAR *)tmp_uid,18,"888888888888",FILE_PARAM_PASS);

	BCDToHex(tmp_uid, CCparamGlobalInfor.CCUID, 12);

	i = ProfileIntRead("FIHead","DebugLevel",0,FILE_PARAM_PASS);											//dbglevel is int
	SetDebugShowLvl(i);
	if( !memcmp(val,"0",1))				//0:cc control: 1:-manu
		CCparamGlobalInfor.ControlMethod = CONTROLDBYCC;
	else if(!memcmp(val,"1",1))
		CCparamGlobalInfor.ControlMethod = CONTROLDBYMANUAL;
		debug(DEBUG_LOCAL_FILEPARAM_LOAD,"^CCparamGlobalInfor.ControlMethod =%d\n",CCparamGlobalInfor.ControlMethod);
	CCparamGlobalInfor.ItfWay = ITF_WAY_ETHER;
	ProfileStringRead("FIHead","ConnectType",val,sizeof(val),"2",FILE_PARAM_PASS);
	if( !memcmp(val,"2",1))
		CCparamGlobalInfor.ItfWay = ITF_WAY_ETHER;
	else if(!memcmp(val,"1",1))
		CCparamGlobalInfor.ItfWay = ITF_WAY_GPRS;
	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"^CCparamGlobalInfor.ItfWay =%d\n",CCparamGlobalInfor.ItfWay);
	ProfileStringRead("FIHead","ServerIpaddr",CCparamGlobalInfor.ServerIpaddr,32,"192.168.1.120",FILE_PARAM_PASS);				//ip is char
	CCparamGlobalInfor.ServerPort= ProfileIntRead("FIHead","ServerPort",8888,FILE_PARAM_PASS);							//port is int
	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"^CCparamGlobalInfor.ServerIpaddr is %s\n",CCparamGlobalInfor.ServerIpaddr);
	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"^CCparamGlobalInfor.ServerPort is %d\n",CCparamGlobalInfor.ServerPort);

	CCparamGlobalInfor.HeartBCycle = ProfileIntRead("FIHead","HeartBeatcycle",5,FILE_PARAM_PASS);	
			
#if 0
	CCparamGlobalInfor.HeartBCycle = ProfileIntRead("FIHead","HeartBeatcycle",5,FILE_PARAM_PASS);								//heartbeat is int
	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"^CCparamGlobalInfor.HeartBCycle is %d\n",CCparamGlobalInfor.HeartBCycle);
	
	CCparamGlobalInfor.TimeOut = ProfileIntRead("FIHead","TimeOut",5,FILE_PARAM_PASS);	//超时时间(Sec)
	CCparamGlobalInfor.Retry = ProfileIntRead("FIHead","Retry",5,FILE_PARAM_PASS);		//重试次数
	CCparamGlobalInfor.ConfirmFlag = ProfileIntRead("FIHead","ConfirmFlag",1,FILE_PARAM_PASS);	

	CCparamGlobalInfor.Proto = ProfileIntRead("FIHead","Proto",0,FILE_PARAM_PASS);
	CCparamGlobalInfor.Dcd = ProfileIntRead("FIHead","Dcd",0,FILE_PARAM_PASS);
	//ProfileStringRead("FIHead","Apn",CCparamGlobalInfor.Apn,32,"cmnet",FILE_PARAM_PASS);							//apn is char
	ProfileStringRead("FIHead","Apn",(CHAR *)CCparamGlobalInfor.Apn,32,"cmnet",FILE_PARAM_PASS);							//apn is char

	CCparamGlobalInfor.Art = ProfileIntRead("FIHead","Art",0,FILE_PARAM_PASS);
	CCparamGlobalInfor.NetSwitch = ProfileIntRead("FIHead", "NetSwitch", 1, FILE_PARAM_PASS);
#endif
	//ProfileStringRead("FIHead","AlarmTelNumberAdmin",CCparamGlobalInfor.AlarmTelNumberAdmin,20,"0000000000000",FILE_PARAM_PASS);
	//ProfileStringRead("FIHead","AlarmTelNumberUser1",CCparamGlobalInfor.AlarmTelNumberUser1,20,"0000000000000",FILE_PARAM_PASS);
	//ProfileStringRead("FIHead","AlarmTelNumberUser2",CCparamGlobalInfor.AlarmTelNumberUser2,20,"0000000000000",FILE_PARAM_PASS);
	//ProfileStringRead("FIHead","AlarmTelNumberUser3",CCparamGlobalInfor.AlarmTelNumberUser3,20,"0000000000000",FILE_PARAM_PASS);
	//ProfileStringRead("FIHead","AlarmTelNumberUser4",CCparamGlobalInfor.AlarmTelNumberUser4,20,"0000000000000",FILE_PARAM_PASS);
 #if 0
	ProfileStringRead("FIHead","AlarmTelNumberAdmin",(CHAR *)CCparamGlobalInfor.AlarmTelNumberAdmin,20,"0000000000000",FILE_PARAM_PASS);
	ProfileStringRead("FIHead","AlarmTelNumberUser1",(CHAR *)CCparamGlobalInfor.AlarmTelNumberUser1,20,"0000000000000",FILE_PARAM_PASS);
	ProfileStringRead("FIHead","AlarmTelNumberUser2",(CHAR *)CCparamGlobalInfor.AlarmTelNumberUser2,20,"0000000000000",FILE_PARAM_PASS);
	ProfileStringRead("FIHead","AlarmTelNumberUser3",(CHAR *)CCparamGlobalInfor.AlarmTelNumberUser3,20,"0000000000000",FILE_PARAM_PASS);
	ProfileStringRead("FIHead","AlarmTelNumberUser4",(CHAR *)CCparamGlobalInfor.AlarmTelNumberUser4,20,"0000000000000",FILE_PARAM_PASS);

	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"AlarmTelNumberAdmin:%s\nAlarmTelNumberUser1:%s\nAlarmTelNumberUser2:%s\nAlarmTelNumberUser3:%s\nAlarmTelNumberUser4:%s\n",
			CCparamGlobalInfor.AlarmTelNumberAdmin,CCparamGlobalInfor.AlarmTelNumberUser1,CCparamGlobalInfor.AlarmTelNumberUser2,
								CCparamGlobalInfor.AlarmTelNumberUser3,CCparamGlobalInfor.AlarmTelNumberUser4);


	CCparamGlobalInfor.MeterRDInterval = ProfileIntRead("FIHead","MeterRDInterval",0,FILE_PARAM_PASS);							//para_term.interval is int     		 轮训读电表时间     ( 小时)
	CCparamGlobalInfor.MeterInfRecDays = ProfileIntRead("FIHead","MeterInfRecDays",0,FILE_PARAM_PASS);									//para_term.days is int    			保存电能信息时间 ( 天)
 #endif	

	FileParamLoadedResultPrint();
		
	return 0;
}

#if 1
SINT FileParamLoadedResultPrint(void)
{
	#if (MY_DEBUG && (DEBUG_LOCAL_FILEPARAM_LOAD || DEBUG_LOCAL_PARA_RECIV))
		printf("\n\n\n****************in FileParamLoadedResultPrint start**************************\n");
		puts((CHAR *)CCparamGlobalInfor.CCUID);
		printf("ccuid is:%x:%x:%x:%x:%x:%x\n",CCparamGlobalInfor.CCUID[0],CCparamGlobalInfor.CCUID[1],CCparamGlobalInfor.CCUID[2],CCparamGlobalInfor.CCUID[3],CCparamGlobalInfor.CCUID[4],CCparamGlobalInfor.CCUID[5]);
		printf("CCparamGlobalInfor.DebugLevel is %d\n",CCparamGlobalInfor.DebugLevel);
		printf("CCparamGlobalInfor.ControlMethod is %d\n",CCparamGlobalInfor.ControlMethod);
		printf("CCparamGlobalInfor.ItfWay is %d\n",CCparamGlobalInfor.ItfWay);
		puts((CHAR *)CCparamGlobalInfor.ServerIpaddr);
		printf("CCparamGlobalInfor.ServerPort is %d\n",CCparamGlobalInfor.ServerPort);
		printf("CCparamGlobalInfor.HeartBCycle is %d\n",CCparamGlobalInfor.HeartBCycle);
		printf("CCparamGlobalInfor.TimeOut is %d\n",CCparamGlobalInfor.TimeOut);
		printf("CCparamGlobalInfor.Retry is %d\n",CCparamGlobalInfor.Retry);
		printf("CCparamGlobalInfor.ConfirmFlag is %d\n",CCparamGlobalInfor.ConfirmFlag);
		printf("CCparamGlobalInfor.Proto is %d\n",CCparamGlobalInfor.Proto);
		printf("CCparamGlobalInfor.dcd is %d\n",CCparamGlobalInfor.Dcd);
		printf("CCparamGlobalInfor.art is %d\n",CCparamGlobalInfor.Art);
		printf("CCparamGlobalInfor.apn is %s\n",(CHAR *)(CCparamGlobalInfor.Apn));
		printf("CCparamGlobalInfor.NetSwitch is %d\n",CCparamGlobalInfor.NetSwitch);
		puts((CHAR *)CCparamGlobalInfor.AlarmTelNumberAdmin);
		puts((CHAR *)CCparamGlobalInfor.AlarmTelNumberUser1);
		puts((CHAR *)CCparamGlobalInfor.AlarmTelNumberUser2);
		puts((CHAR *)CCparamGlobalInfor.AlarmTelNumberUser3);
		puts((CHAR *)CCparamGlobalInfor.AlarmTelNumberUser4);
		printf("CCparamGlobalInfor.MeterInfRecDays is %d\n",CCparamGlobalInfor.MeterInfRecDays);
		printf("CCparamGlobalInfor.MeterRDInterval is %d\n", CCparamGlobalInfor.MeterRDInterval);

		printf("****************in FileParamLoadedResultPrint end**************************\n\n\n");
	#endif
	return SUCCESS;
}


SINT TOPTaskSaveAndFileParamRelatOprat(UCHAR fileflag)
{
	debug(DEBUG_LOCAL_TIME_TASK,"in fun TaskSaveAndFileParamRelatOprat22-22-22\n");
	
	switch(fileflag) {
	 case FILE_PARAMFOR_TIMERTASK_SAV:
		FileParamSave(FLAG_PARAMFOR_TIMERTASK);
		break;

	 case FILE_PARAMFOR_NODELIST_SAV:
		FileParamSave(FLAG_PARAMFOR_NODELIST);
		break;
		
	 case FILE_PARAMFOR_SYSTERMINI_SAV:
		FileParamSave(FLAG_PARAMFOR_SYSTERMINI);
		break;
		
	default:
		break;
	}
	return SUCCESS;
}

USHORT FileCrcTable[]={
	
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0,
};

USHORT FileCrcGenerate(UCHAR *InfoBuf,SINT FileSize)
{
	USHORT CrcContext = 0;
	
	if(FileSize <= 0){return 0x00;}
	while(FileSize-- != 0){
		CrcContext = (CrcContext<<8)^FileCrcTable[(CrcContext>>8)^(*InfoBuf++)];
	}

	return CrcContext;
}
SINT TOPCCPARAMSaveToFile(void)
{

	//ProfileStringWrite(const CHAR *section,const CHAR *key,const char *value,const CHAR *file);
	//ProfileIntWrite(const CHAR * section, const CHAR * key, char * value, CHAR * DefaultValue, const CHAR * file)();
	return SUCCESS;

}

SINT FileParamSave(UCHAR Flag)
{
	struct FileParaManagementStruct *P_FileParaManage;
	CHAR FileName[80];
	FILE *FilePf;
	struct TimeTaskHeadStruct TimeTaskHead;
	P_FileParaManage = (struct FileParaManagementStruct *)&FileActExistList[Flag];

	//FOR TIMERTASK and SINGLELED NODE
	if((Flag < 2)&&(Flag >=0)){
		TimeTaskHead.FileMagic = 0x1234;
		TimeTaskHead.FileCrc = FileCrcGenerate(P_FileParaManage->Address,P_FileParaManage->FileSize);
		//strcat(FileName,P_FileParaManage->FileName);
		strcpy(FileName,P_FileParaManage->FileName);


		FilePf = fopen(FileName,"wb");

		
		fwrite((UCHAR *)&TimeTaskHead,1,sizeof(TimeTaskHead),FilePf);
		
		fwrite(P_FileParaManage->Address,1,P_FileParaManage->FileSize,FilePf);


		fclose(FilePf);

		
		debug(DEBUG_LOCAL_PARAFILE_SAVE,"file para for %d has been saved\n",Flag);
		
	}
	
	//FOR CCPARASAVE
	if(Flag == 2){
		TOPCCPARAMSaveToFile();
	}
	
	return SUCCESS;
}

SINT GetSizeOfFile(FILE *FilePt)
{
	long start, end;

	fseek(FilePt, 0, SEEK_END);
	end = ftell(FilePt);
	fseek(FilePt, 0, SEEK_SET);
	start = ftell(FilePt);

	return(end-start);
}

SINT ActFileParamLoad(CHAR *FileName,struct FileParaManagementStruct *P_FileParaPt)
{
	FILE *FilePt;
	UCHAR *ReadTmpBuf;
	SINT FileSize;
	SINT I_Dbug;
	
	struct TimeTaskHeadStruct TimeTaskHead;
	FilePt = fopen(FileName,"rb");
//add start
	if(FilePt == NULL){
		debug(DEBUG_LOCAL_TIME_TASK,"TASK_QUEUE_NO_TIMERTASK_DATFILE IS %d\n",TASK_QUEUE_NO_TIMERTASK_DATFILE);
		
		return TASK_QUEUE_NO_TIMERTASK_DATFILE;

	}

//add end 
	FileSize = GetSizeOfFile(FilePt);
	debug(DEBUG_LOCAL_TIME_TASK,"FileSizeis %d\n",FileSize);
		
	

	//add start
	if(FileSize < 30){
		debug(DEBUG_LOCAL_TIME_TASK,"TASK_QUEUE_EMPTY is %d\n",TASK_QUEUE_EMPTY);
		fclose(FilePt);
		//free(ReadTmpBuf);

		return TASK_QUEUE_EMPTY;
	}

	//add end
	
	ReadTmpBuf = (UCHAR *)malloc(FileSize);

	TOPGLOBALMEMALLOCTIMES++;
	TOPGLOBAL_HEAP_SIZE += FileSize;
	debug(DEBUG_LOCAL_HEAPMEM_INCREASE,"HEAPMEM_INCREASE in function ActFileParamLoad \
				and increase size is %d\n",FileSize);

	fread((UCHAR *)&TimeTaskHead,sizeof(TimeTaskHead),1,FilePt);
	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"TimeTaskHead.FileCrc is %d,TimeTaskHead.FileMagic is %d\n",TimeTaskHead.FileCrc,TimeTaskHead.FileMagic);
	
	
	FileSize = FileSize-4;
	
	fread(ReadTmpBuf,FileSize,1,FilePt);
	
	memcpy(P_FileParaPt->Address, ReadTmpBuf, FileSize);

	debug(DEBUG_LOCAL_FILEPARAM_LOAD,"ReadTmpBuf is %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",ReadTmpBuf[0],
		ReadTmpBuf[1],ReadTmpBuf[2],ReadTmpBuf[3],ReadTmpBuf[4],
		ReadTmpBuf[5],ReadTmpBuf[6],ReadTmpBuf[7],ReadTmpBuf[8],ReadTmpBuf[9],
		ReadTmpBuf[10],ReadTmpBuf[11],ReadTmpBuf[12],ReadTmpBuf[13],ReadTmpBuf[14]);

	for(I_Dbug = 0;I_Dbug<TimerTaskQuatityMax;I_Dbug++){
		debug(DEBUG_LOCAL_FILEPARAM_LOAD,"TimerTaskList[%d] is %x:%x\n",I_Dbug,TimerTaskList[I_Dbug].TaskID[0],TimerTaskList[I_Dbug].TaskID[1]);
	}
	
	
	fclose(FilePt);
	free(ReadTmpBuf);
	
	TOPGLOBAL_HEAP_SIZE -= (FileSize+4);

	
	debug(DEBUG_LOCAL_HEAPMEM_INCREASE,"HEAPMEM_INCREASE in function ActFileParamLoad \
			and reduse size is %d\n",FileSize+4);
	


	
	return SUCCESS;
}

SINT TopFileParamLoad(UCHAR Flag)
{
	CHAR FileName[80];
	struct FileParaManagementStruct *P_FileParaManage;
	
//	printf("sizeof(FileActExistList)/sizeof(FileActExistList[0]) is %d\n",sizeof(FileActExistList)/sizeof(FileActExistList[0]));

	if((Flag<0) || (Flag > sizeof(FileActExistList)/sizeof(FileActExistList[0]))){return FAIL;}
	P_FileParaManage = (struct FileParaManagementStruct *)&FileActExistList[Flag];
	strcpy(FileName,P_FileParaManage->FileName);
	//printf("FileName in TopFileParamLoad is %s\n",FileName);
	ActFileParamLoad(FileName,P_FileParaManage);
	return SUCCESS;
}

#endif
