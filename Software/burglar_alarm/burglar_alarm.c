#include "burglar_alarm.h"

#define MaxBurglarAlarmElement 10			//10 Cicurent
pthread_mutex_t BurglatAlarmTable_lock;
#define	BURGLAR_ELEM_TABLE_LOCK   pthread_mutex_lock(&BurglatAlarmTable_lock)
#define	BURGLAR_ELEM_TABLE_UNLOCK pthread_mutex_unlock(&BurglatAlarmTable_lock)
extern volatile SINT TOPGLOBALMEMALLOCTIMES;
extern volatile SINT TOPGLOBAL_HEAP_SIZE;
BurglarAlarmGroupGL BurglarAlarmElemList;


SINT DeleteBurglarAlarmTable(void)
{
	CHAR BurglarDelSql[]="delete from BurglarAlarmTable;";
	
	BURGLAR_ELEM_TABLE_LOCK;
	debug(DEBUG_LOCAL_BURGLAR,"%s\n",BurglarDelSql);
	if(0) //(sqlite3_exec(GetDatabase(), BurglarDelSql, NULL, 0, NULL)!=SQLITE_OK)
	{
		perror("DeleteBurglarAlarmTable fail");
		
	BURGLAR_ELEM_TABLE_UNLOCK;
		//	write_error_log("error in func cleanthefproof : clean fail\n",error_txt);
		return BURGLALARMTABLE_ELEM_DEL_ERR;
	}
	BurglarAlarmElemList.ElementOrder = 0;
	BURGLAR_ELEM_TABLE_UNLOCK;
	return 0;
}


SINT BurglarAlarmElemTableLoad(void)
{
	char **resultp = NULL;
	int nrow = 0,ncolumn = 0,i = 0;
	int m;
	BurglarAlarmElemenent *CurrentBurgElem;

	BurglarAlarmElemList.ElementOrder = 0;
	BurglarAlarmElemList.MaxElement = MaxBurglarAlarmElement;
	BurglarAlarmElemList.BurglarAlarmList = (BurglarAlarmElemenent *)malloc((BurglarAlarmElemList.MaxElement)*sizeof(BurglarAlarmElemenent));

	TOPGLOBALMEMALLOCTIMES++;
	TOPGLOBAL_HEAP_SIZE += (BurglarAlarmElemList.MaxElement)*sizeof(BurglarAlarmElemenent);
	debug(DEBUG_LOCAL_HEAPMEM_INCREASE,"HEAPMEM_INCREASE in function BurglarAlarmElemTableLoad \
				and increase size is %d\n",(BurglarAlarmElemList.MaxElement)*sizeof(BurglarAlarmElemenent));
	//sqlite3_get_table(GetDatabase(), "select * from BurglarAlarmTable;", &resultp, &nrow, &ncolumn, NULL);

	if(resultp&&nrow){
		debug(DEBUG_LOCAL_BURGLAR,"run in BurglarAlarmElemTableLoad && nrow is %d\n",nrow);
		CurrentBurgElem = BurglarAlarmElemList.BurglarAlarmList;

		for(i=1;i<=nrow&&i<=MaxBurglarAlarmElement;i++){
			m = 0;

			//String2Bytes(resultp[i*ncolumn+m], CurrentBurgElem->UID, strlen(resultp[i*ncolumn+m++]));
			String2Bytes(resultp[i*ncolumn+m], CurrentBurgElem->UID, strlen(resultp[i*ncolumn+m]));
			m++;
			//memcpy(CurrentBurgElem->UID,resultp[i*ncolumn+m],strlen(resultp[i*ncolumn+m++]));

			CurrentBurgElem->DeviceAddr = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->BurglarType = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->Channel = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->ChannelHardCirc = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->flag = atoi(resultp[i*ncolumn+m++]);

			//memcpy(CurrentBurgElem->LineName,resultp[i*ncolumn+m],strlen(resultp[i*ncolumn+m++])+1);
			memcpy(CurrentBurgElem->LineName,resultp[i*ncolumn+m],strlen(resultp[i*ncolumn+m])+1);
			m++;
				
			CurrentBurgElem->WireLineState = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->NetSend2User = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->SMSSend2User = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->ExterInfor1 = atoi(resultp[i*ncolumn+m++]);
			CurrentBurgElem->ExterInfor2 = atoi(resultp[i*ncolumn+m++]);

			BurglarAlarmElemList.ElementOrder++;
			CurrentBurgElem++;
		}
		debug(DEBUG_LOCAL_BURGLAR,"BurglarAlarmElemList.ElementOrder is %d,BurglarAlarmElemList.MaxElement is %d\n",BurglarAlarmElemList.ElementOrder,BurglarAlarmElemList.MaxElement);
		//sqlite3_free_table(resultp);
	}
	return SUCCESS;
}

	CHAR UID[6];
	CHAR DeviceAddr;

	CHAR BurglarType;
	CHAR Channel;
	CHAR ChannelHardCirc;
	CHAR flag;

	CHAR LineName[30];
	CHAR WireLineState;
	CHAR NetSend2User;
	CHAR SMSSend2User;

	CHAR ExterInfor1;
	UINT ExterInfor2;

SINT InserBurglarInforTable(BurglarAlarmElemenent *BuglarElem)
{
	CHAR sql[200];
	CHAR *insburglar = "insert into BurglarAlarmTable values(\'%s\',%d,%d,%d,%d,%d,\'%s\',%d,%d,%d,%d,%d);";
	CHAR uid[13];
	SINT len;


	//BurglarAlarmElemInforPrint(BuglarElem);

	if(!BuglarElem){
		perror("burglar alarm element union error\n");
		return BURGLALARMTABLE_ELEM_UNION_ERR;
	}
	
	len = Bytes2String((UCHAR *)BuglarElem->UID, uid,6);
	debug(DEBUG_LOCAL_BURGLAR,"len is %d\tuid is %x%x%x%x%x%x\n",
						len,uid[0],uid[1],uid[2],uid[3],uid[4],uid[5]);
	
	
	if(len){
		uid[len] = '\0';
	}
	else{
		perror("burglar alarm element uid formchange error\n");
		return BURGLALARMTABLE_ELEM_UNION_ERR;
	}
	debug(DEBUG_LOCAL_BURGLAR,"uid in S is %s\n",uid);
	
	
	sprintf(sql,insburglar,uid,BuglarElem->DeviceAddr,BuglarElem->BurglarType,\
		BuglarElem->Channel,BuglarElem->ChannelHardCirc,BuglarElem->flag,BuglarElem->LineName,\
		BuglarElem->WireLineState,BuglarElem->NetSend2User,BuglarElem->SMSSend2User,\
		BuglarElem->ExterInfor1,BuglarElem->ExterInfor2);
	debug(DEBUG_LOCAL_BURGLAR,"after sprintf\n%s\n",sql);
	
	if(0) //(sqlite3_exec(GetDatabase(),sql,NULL,0,NULL)!=SQLITE_OK)
	{
		perror("burglar alarm table insert error\n");
		return BURGLALARMTABLE_ELEM_INSERT_ERR;
	}
	
	return SUCCESS;
}
SINT UpdatBurglarInforTable(BurglarAlarmElemenent *BuglarElem)
{

	CHAR sql[200];
	CHAR *updburglar = "update BurglarAlarmTable set DeviceAddr=%d,BurglarType=%d,Channel=%d,ChannelHardCirc=%d,flag=%d,LineName = \'%s\',WireLineState=%d,NetSend2User=%d,SMSSend2User=%d,ExterInfor1 = %d,ExterInfor2=%d where UID=\'%s\';";
	CHAR uid[13];
	SINT len;

	#if DEBUG_LOCAL_BURGLAR
		printf("before sprintf\n");
		puts(updburglar);
	#endif
	
	if(!BuglarElem){
		perror("burglar alarm element union error when update\n");
		return BURGLALARMTABLE_ELEM_UNION_ERR;
	}
	
	len = Bytes2String((UCHAR *)BuglarElem->UID, uid,6);
		if(len){
		uid[len] = '\0';
	}
	else{
		perror("burglar alarm element uid formchange error when update\n");
		return BURGLALARMTABLE_ELEM_UNION_ERR;
	}

	sprintf(sql,updburglar,BuglarElem->DeviceAddr,BuglarElem->BurglarType,\
		BuglarElem->Channel,BuglarElem->ChannelHardCirc,BuglarElem->flag,BuglarElem->LineName,\
		BuglarElem->WireLineState,BuglarElem->NetSend2User,BuglarElem->SMSSend2User,\
		BuglarElem->ExterInfor1,BuglarElem->ExterInfor2,uid);

	#if DEBUG_LOCAL_BURGLAR 
		printf("after sprintf\n");
		puts(sql);
	#endif

	if(0)   //(sqlite3_exec(GetDatabase(),sql,NULL,0,NULL)!=SQLITE_OK)
	{
		perror("burglar alarm table update error\n");
		return BURGLALARMTABLE_ELEM_UPDATE_ERR;
	}

	return SUCCESS;
}

SINT SetBurglarInforTable(CHAR * infor,SINT n)
{
	return SUCCESS;
}

SINT PCQueryBurglarAlarmInfor()
{
	return SUCCESS;
}
SINT Set485BurglarDevic(CHAR deviceID)
{
	return SUCCESS;
}
SINT Read485BurglarDevic(CHAR deviceID)
{
	return SUCCESS;
}
SINT Write485BurglarDevic(CHAR deviceID)
{
	return SUCCESS;
}
SINT Analys485BurglarInfor(CHAR deviceID)
{
	return SUCCESS;
}

SINT SetCarrierBurglarDevic(CHAR *uid)
{
	return SUCCESS;
}
SINT ReadCarrierBurglarDevic()		//recvinfo
{
	return SUCCESS;
}
SINT WriteCarrierBurglarDevic()	//sendinfor
{
	return SUCCESS;
}
SINT AnalysCarrierBurglarInfor()
{
	return SUCCESS;
}
SINT NetBurglarAlarmSend()	//sendinfor
{
	return SUCCESS;
}
SINT SMSBurglarAlarmSend()
{
	return SUCCESS;
}









SINT BurglarAlarmElemInforPrint(BurglarAlarmElemenent *tt)
{
	SINT elemnum;
	BurglarAlarmElemenent *CurrentBurgElem;

	CurrentBurgElem = tt;
	
	for(elemnum=0;elemnum<BurglarAlarmElemList.ElementOrder;elemnum++){

			printf("********************JUST for DEBUG*******************************\n");
			printf("CurrentBurgElem->UID is :\n");
			printf(" in string is :%s \n",CurrentBurgElem->UID);
			printf("in hex :%x:%x:%x:%x:%x\n",CurrentBurgElem->UID[0],CurrentBurgElem->UID[1],CurrentBurgElem->UID[2],CurrentBurgElem->UID[3],CurrentBurgElem->UID[4]);
			printf("in dec :%d:%d:%d:%d:%d\n",CurrentBurgElem->UID[0],CurrentBurgElem->UID[1],CurrentBurgElem->UID[2],CurrentBurgElem->UID[3],CurrentBurgElem->UID[4]);
			printf("in cha :%c:%c:%c:%c:%c\n",CurrentBurgElem->UID[0],CurrentBurgElem->UID[1],CurrentBurgElem->UID[2],CurrentBurgElem->UID[3],CurrentBurgElem->UID[4]);
			printf("CurrentBurgElem->DeviceAddr is %d\n",CurrentBurgElem->DeviceAddr);
			printf("CurrentBurgElem->BurglarType is %d\n",CurrentBurgElem->BurglarType);
			printf("CurrentBurgElem->Channel is %d\n",CurrentBurgElem->Channel);
			printf("CurrentBurgElem->ChannelHardCirc is %d\n",CurrentBurgElem->ChannelHardCirc);
			printf("CurrentBurgElem->flag is %d\n",CurrentBurgElem->flag);
			printf("CurrentBurgElem->LineName is %s\n",CurrentBurgElem->LineName);
			printf("CurrentBurgElem->WireLineState is %d\n",CurrentBurgElem->WireLineState);
			printf("CurrentBurgElem->NetSend2User is %d\n",CurrentBurgElem->NetSend2User);
			printf("CurrentBurgElem->SMSSend2User is %d\n",CurrentBurgElem->SMSSend2User);
			printf("CurrentBurgElem->ExterInfor1 is %d\n",CurrentBurgElem->ExterInfor1);
			printf("CurrentBurgElem->ExterInfor2 is %d\n",CurrentBurgElem->ExterInfor2);
			printf("*********************JUST for DEBUG******************************\n");

			CurrentBurgElem++;

	}
	return SUCCESS;
}



