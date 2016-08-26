#include "hardwareinit.h"

extern AtCMDs AtCMDs_ACT[];

UINT HardWareInit()
{
	InterFaceInit();
	ExterDevicInit();
	//DatabaseInit();
	
 	return SUCCESS;
}


SINT DeviceGprsIint()
{	//echo mode off
	Uart_Send(Uart1_ttyS2_GPRS,   AtCMDs_ACT[ATE0].Cmd, strlen(AtCMDs_ACT[ATE0].Cmd), 0);
	return SUCCESS;
}
SINT Device485Init()
{
	return SUCCESS;
}
SINT DevicePlcInit()
{
	return SUCCESS;
}


SINT InterFaceInit()
{
	UartForDISInit();
	UartForGprsInit();
	UartForPlcInit();
	UartFor485Init();		//error segv	??
	
//	GpioForLedKey();
	return SUCCESS;
}


SINT ExterDevicInit()
{
	DeviceGprsIint();
	Device485Init();
	DevicePlcInit();
	return SUCCESS;
}



