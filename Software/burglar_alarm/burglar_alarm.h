#ifndef __BURGLAR_ALARM__
#define __BURGLAR_ALARM__
#include "include.h"

typedef struct {							//ok too

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
}__attribute__((packed)) BurglarAlarmElemenent;


typedef struct{
	
	UINT MaxElement;
	UINT ElementOrder;
	//struct BurglarAlarmElemenent *BurglarAlarmList;
	BurglarAlarmElemenent *BurglarAlarmList;
}__attribute__((packed))BurglarAlarmGroupGL; 


typedef struct {
		char address;
		char command;
		char addr_high;
		char addr_low;
		char num_high;
		char num_low;
		char bytes;
		char  data0;
		char  data1;
		char crc_high;
		char crc_low;
}__attribute__((packed))theft_485;

SINT DeleteBurglarAlarmTable(void);
SINT BurglarAlarmElemTableLoad(void);
//SINT BurglarAlarmElemInforPrint(BurglarAlarmGroupGL *tt);
SINT BurglarAlarmElemInforPrint(BurglarAlarmElemenent *tt);

SINT InserBurglarInforTable(BurglarAlarmElemenent *BuglarElem);
SINT UpdatBurglarInforTable(BurglarAlarmElemenent *BuglarElem);
SINT SetBurglarInforTable(CHAR * infor,SINT n);

SINT PCQueryBurglarAlarmInfor();

SINT Set485BurglarDevic(CHAR deviceID);
SINT Read485BurglarDevic(CHAR deviceID);
SINT Write485BurglarDevic(CHAR deviceID);
SINT Analys485BurglarInfor(CHAR deviceID);

SINT SetCarrierBurglarDevic(CHAR *uid);
SINT ReadCarrierBurglarDevic();		//recvinfo
SINT WriteCarrierBurglarDevic();	//sendinfor
SINT AnalysCarrierBurglarInfor();

SINT NetBurglarAlarmSend();	//sendinfor
SINT SMSBurglarAlarmSend();

#endif

