#ifndef __METER_CMDRDWR__
#define __METER_CMDRDWR__
#include "include.h"
//#include "softwareinit.h"
#include "loadfile.h"

#define DIDO_OPEN 	    		1
#define DIDO_CLOSE	    		2
#define DIDO_QUERY_INPUT		1
#define DIDO_QUERY_OUTPUT 	2
#define METTERCMD_LENMAX    	30
#define DONumPerMeter		8

//#define METERD_WAIT_TIME_CONTROL 655340		//newmsg
#define METERD_WAIT_TIME_CONTROL 655340		//maichong

/* change 2011_10_13 add */
typedef struct MetterElecInforStruc{				

	FLOAT Ua;
	FLOAT Ub;
	FLOAT Uc;
	FLOAT Up;
	
	FLOAT Uab;
	FLOAT Ubc;
	FLOAT Uca;
	FLOAT Ul;
	
	FLOAT Ia;
	FLOAT Ib;
	FLOAT Ic;
	FLOAT I;

	FLOAT Un;
	FLOAT In;
	FLOAT F;
	
	FLOAT Pa;
	FLOAT Pb;
	FLOAT Pc;
	FLOAT P;
	
	FLOAT Qa;
	FLOAT Qb;
	FLOAT Qc;
	FLOAT Q;
	
	FLOAT Sa;
	FLOAT Sb;
	FLOAT Sc;
	FLOAT S;
	
	FLOAT PFa;
	FLOAT PFb;
	FLOAT PFc;
	FLOAT PF;
	
	DOUBALE kWh;			//you gong
	DOUBALE kvarh;		//wu gong
	DOUBALE kVAh;		//shi zai

	FLOAT Id;					//add 2012-07-11 start
	FLOAT Ie;
	FLOAT If;

	FLOAT Pd;					
	FLOAT Pe;
	FLOAT Pf;

	FLOAT Qd;
	FLOAT Qe;
	FLOAT Qf;

	FLOAT Sd;
	FLOAT Se;
	FLOAT Sf;

	FLOAT PFd;
	FLOAT PFe;
	FLOAT PFf;
}MetterElecInforStruc;


typedef struct MetterOpratePack{
	UCHAR address;			//设备地址
	UCHAR cmd;				//控制指令
	
	UCHAR start_addr_high;	//起始地址高位
	UCHAR start_addr_low;	//起始地址低位
	
	UCHAR num_reg_high;		//寄存器个数高位
	UCHAR num_reg_low;		//寄存器个数低位
	
	UCHAR crc_low;			//crc校验的低位
	UCHAR crc_high;			//crc校验的高位
}__attribute__((packed)) MetterOpratePack;

typedef struct TOPDevicePowerStruct{				//change 2011_10_13 add

		UCHAR MeterDeviceID;

		FLOAT Ua;
		FLOAT Ub;
		FLOAT Uc;
		FLOAT Up;
		FLOAT Ul;
		
		FLOAT Ia;
		FLOAT Ib;
		FLOAT Ic;
		FLOAT I;

		FLOAT Friq;
		
		FLOAT Pa;
		FLOAT Pb;
		FLOAT Pc;
		FLOAT P;

		FLOAT Qa;
		FLOAT Qb;
		FLOAT Qc;
		FLOAT Q;

		FLOAT Sa;
		FLOAT Sb;
		FLOAT Sc;
		FLOAT S;

		FLOAT PFa;
		FLOAT PFb;
		FLOAT PFc;
		FLOAT PF;

		FLOAT  DrainCurrent;			//漏电流
		double KWH;					//有功功率
		double Kvarh;				//无功功率


		FLOAT Id;					//add 2012-07-11 start
		FLOAT Ie;
		FLOAT If;

		FLOAT Pd;					
		FLOAT Pe;
		FLOAT Pf;

		FLOAT Qd;
		FLOAT Qe;
		FLOAT Qf;

		FLOAT Sd;
		FLOAT Se;
		FLOAT Sf;

		FLOAT PFd;
		FLOAT PFe;
		FLOAT PFf;

}__attribute__((packed)) TOPDevicePowerStruct;


typedef struct TOPDeviceMeterStatStruct{				//change 2011_10_13 add

	UCHAR MeterInnerDeviceID;
	UCHAR MeterInnerNum;
	UCHAR MeterInnerStatus;

	UCHAR MeterExternFirDeviceID;
	UCHAR MeterExternFirNum;
	UCHAR MeterExternFirStatus;
		
}__attribute__((packed)) TOPDeviceMeterStatStruct;


typedef struct TOPDeviceGPRSMDStatStruct{				//change 2011_10_13 add

		UCHAR MDFactory[15];
		UCHAR MDType[10];
		UCHAR Healthy;
		UCHAR SignalDBM;
		UCHAR NetActivate;
		
		FLOAT CostAlready;
		FLOAT CostLeft;
		
		UCHAR NetAccessed;
		
}__attribute__((packed)) TOPDeviceGPRSMDStatStruct;


extern struct MetterElecInforStruc 	TOPMetterElecInformation;

SINT UART1_TTYS2_PLC_change485RICIVEtest();
void UART1_TTYS2_PLC_change485test();

SINT MetterOprate(UCHAR DeviceId,UCHAR chanel,UCHAR action);
SINT MetterOutInputStatQuery(UCHAR DeviceId,UCHAR chanel,UCHAR type,UCHAR *stat);
SINT MetterParamQuery(UCHAR DeviceId,UCHAR chanel,UCHAR action);

SINT DIDOOprate(UCHAR DeviceId,UCHAR chanel,UCHAR action);
SINT DIDOOutInputStatQuery(UCHAR DeviceId,UCHAR chanel,UCHAR type,UCHAR *stat);
SINT DIDOParamQuery(UCHAR DeviceId,UCHAR chanel,UCHAR action);
SINT DIDOOutInputStatQuery(UCHAR DeviceId,UCHAR chanel,UCHAR type,UCHAR *stat);

SINT Device485Reciv(UCHAR UARTPORT,UCHAR *Reciv,SINT *RecivLen);


#endif
