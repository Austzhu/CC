/********************************************************************
	> File Name:	SingleOrCoordi.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com 
	> Created Time:	2016年06月02日 星期四 16时46分36秒
 *******************************************************************/
#include "SingleOrCoordi.h"
#include "serial.h"
#include "coordinate.h"
#define Single_A 	"./update/Single_A.bin"
#define Single_B	"./update/Single_B.bin"
#define Coordinate_A 	"./update/Coordinate_A.bin"
#define Coordinate_B 	"./update/Coordinate_B.bin"

#define Repeat_Num 		5
#define TimeoutNum 		5
static Package_Info 	UpdateInfo;
static Package_Data 	UpdatePackage;

static s32 Send_OneByte(char ch)
{
	return Uart_Send(Uart1_ttyO1_485, &ch,1,1000000);
}
static s8 Recv_OneByte(void)
{
	s8 Response = 0;
	return Uart_Recv(Uart1_ttyO1_485,&Response,1,1000000) ?FAIL:Response;
}
static s32 Update_Init(char Image)
{
	struct stat   FileStat;
	char Is_ImageA;
	int res = -1;
	switch(Image){
		case 'a':
		case 'A':Is_ImageA = 'A';break;
		case 'b':
		case 'B':Is_ImageA = 'B';break;
		case FAIL:
		default:
			debug(DEBUG_update,"Unknown Update for Image %d!\n",Image);
			return FAIL;
	}
	if(UpdateInfo.SinOrCoordi){//协调器
		debug(DEBUG_update,"Update Coordinate for Image %c\n",Is_ImageA);
		res = Is_ImageA == 'A' ? access(Coordinate_A,F_OK) : access(Coordinate_B,F_OK);
		if(res){ err_Print(DEBUG_update,"Update file not exist!\n"); return FAIL; }
		UpdateInfo.fd = Is_ImageA == 'A' ?  open(Coordinate_A,O_RDONLY) :  open(Coordinate_B,O_RDONLY);
		if(UpdateInfo.fd < 0){ err_Print(DEBUG_update,"Open update file failed!\n"); return Open_FAIL; }
	}else{//单灯
		debug(DEBUG_update,"Update Single for Image %c\n",Is_ImageA);
		res = Is_ImageA == 'A' ? access(Single_A,F_OK) : access(Single_B,F_OK);
		if(res){ err_Print(DEBUG_update,"Update file not exist!\n"); return FAIL; }
		UpdateInfo.fd = Is_ImageA == 'A' ?  open(Single_A,O_RDONLY) :  open(Single_B,O_RDONLY);
		if(UpdateInfo.fd < 0){ err_Print(DEBUG_update,"Open update file failed!\n"); return Open_FAIL; }
	}
	/* 初始化升级信息 */
	if(-1 == fstat(UpdateInfo.fd, &FileStat)){
		err_Print(DEBUG_update,"Get update info Fail!\n");
		return FAIL;
	}
	UpdateInfo.filesize 	= FileStat.st_size;
	debug(DEBUG_update,"FileSize:%d\n",UpdateInfo.filesize);
	UpdateInfo.DataNum 	= (UpdateInfo.filesize+UPACKSIZE-1)/UPACKSIZE;	//(UpdateInfo.filesize+1023)/256,计算数据包有多少帧
	debug(DEBUG_update,"DataNum:%d\n",UpdateInfo.DataNum);
	UpdateInfo.CntDataNum = 0;
	UpdateInfo.Repeat 	= Repeat_Num;			//重发次数
	UpdateInfo.CheckType 	= Crc_16;
	UpdateInfo.pend	= 0;
	UpdateInfo.Timeout	= TimeoutNum;	
	debug(DEBUG_update,"Start Update Coordinate 0r Single...\n");
	return SUCCESS;
}

static s32 Send_Package(void)
{
	s32 ReadSize = 0;
	s32 IsEOT = UpdateInfo.CntDataNum >= (UpdateInfo.DataNum-1) ? 1:0;
	UpdatePackage.F_Num 	= UpdateInfo.CntDataNum;
	UpdatePackage.__Num 	= ~UpdateInfo.CntDataNum;

	if(IsEOT){	
		UpdatePackage.Header = EOT;
	}else{	
		UpdatePackage.Header = SOH;
	}

	if(-1 == lseek(UpdateInfo.fd, UPACKSIZE*UpdateInfo.CntDataNum,SEEK_SET) ){
		err_Print(DEBUG_update,"lseek error!\n");
		return FAIL;
	}

	ReadSize = read(UpdateInfo.fd, UpdatePackage.Data,UPACKSIZE);

	if(ReadSize < 0){
		err_Print(DEBUG_update,"Read update file error!\n");
		return FAIL;
	}else if(ReadSize < UPACKSIZE){
		//debug(DEBUG_update,"\nReadSize < UPACKSIZE\n");
		//debug(DEBUG_update,"UpdateInfo.CntDataNum=%d\n",UpdateInfo.CntDataNum);
		if(IsEOT){
			//debug(DEBUG_update,"fill databuf size %d\n",UPACKSIZE-ReadSize);
			memset(UpdatePackage.Data+ReadSize,CTRLZ,UPACKSIZE-ReadSize);
		}
	}
	/* 计算CRC16 */
	Crc16(UpdatePackage.CRC16,UpdatePackage.Data,UPACKSIZE);
	/* 发送数据 */
	return Uart_Send(Uart1_ttyO1_485, (s8*)&UpdatePackage.Header,  sizeof(Package_Data),1000000);
}

void *Update_Single_Coordi(void)
{
	s32 NAK_CNT = 0;
	Send_Package();
	while(1){
		switch(Recv_OneByte()){
			case ACK:
				UpdateInfo.Repeat 	= Repeat_Num;
				UpdateInfo.Timeout 	= TimeoutNum;
				NAK_CNT =0;
				if(++UpdateInfo.CntDataNum >= UpdateInfo.DataNum){
					UpdateInfo.Update_OK = 1;
					debug(DEBUG_update,"Done!\n");
					debug(DEBUG_update,"UpdateInfo.CntDataNum=%d\n",UpdateInfo.CntDataNum);
					//Close_Update();
					return  (void*)SUCCESS;
				}
				debug(DEBUG_update,"#");fflush(stdout);
				Send_Package();
				break;
			case NAK:
				Send_Package();
				if(!--UpdateInfo.Repeat){
					UpdateInfo.pend =!0;
				}
				break;
			case FAIL://超时
				if(++NAK_CNT > 5){
					NAK_CNT = 0;
					debug(DEBUG_update,"Timeout Send package agin!\n");
					Send_Package();
					--UpdateInfo.Timeout;
		
				}
				break;
			default:break;
		}
		if(UpdateInfo.Update_OK){	return (void*)SUCCESS;}
		if(UpdateInfo.pend){		return (void*)Update_Pend;}
		if(!UpdateInfo.Timeout){	return (void*)TIME_OUT;}
	}
}

void  inline GetUpdateInfo(Package_Info *Info)
{
	memcpy(Info,&UpdateInfo,sizeof(Package_Info));

}
void inline SetUpdateInfo(Package_Info *Info)
{
	memcpy(&UpdateInfo,Info,sizeof(Package_Info));
}

void close_fd(int *fd)
{
	close(*fd);
	*fd = -1;
}
void Close_Update(void)
{
	if(UpdateInfo.fd >0){
		close_fd(&UpdateInfo.fd);
	}else{
		debug(1,"close fd < 0\n");
	}
	//memset(&UpdateInfo,0,sizeof(Package_Info));
}

int WaitAck(char CMD,char Ack,int block)
{
	while(block--){
		Send_OneByte(CMD);
		if(Ack == Recv_OneByte())
			return 0;
	}
	return 1;
}
char RecvWait(int bolck)
{
	char ch;
	while(bolck--){
		ch = Recv_OneByte();
		if(FAIL != ch){return ch;}
	}
	return FAIL;
}


void * Updatethread(void *arg)
{
	int RetuenValue = -1;
	char temp = 0;
	pthread_detach(pthread_self());

	

	memset(&UpdateInfo,0,sizeof(Package_Info));
	#ifdef Config_NewProtocol
	Pag_Single *package = (Pag_Single *)arg;
	if(package->Single_Addr[0] == 0 && package->Single_Addr[1] == 0){
		UpdateInfo.SinOrCoordi 	=  !0;
	}else{
		UpdateInfo.SinOrCoordi 	=  0;
	}

	UpdatePackage.Coor_Addr		= package->Coordi_Addr;
	UpdatePackage.Single_Addr_H	= package->Single_Addr[0];
	UpdatePackage.Single_Addr_L 	= package->Single_Addr[1];
	debug(DEBUG_update,"Addr:Coor_Addr=0x%02x,Single_Addr_H=0x%02x,Single_Addr_L=0x%02x\n",
				UpdatePackage.Coor_Addr,UpdatePackage.Single_Addr_H,UpdatePackage.Single_Addr_L);
	temp = package->Data[0];
	//pthread_mutex_lock(&mutex_task);	//获取task锁
	RetuenValue = Update_Init(temp) == SUCCESS ? (int)Update_Single_Coordi():-1;
	#else
	Frame_485  *pkg = (Frame_485 *)arg;
	
	if(pkg->single_addrH == 0 && pkg->single_addrL == 0){
		UpdateInfo.SinOrCoordi 	=  !0;
	}else{
		UpdateInfo.SinOrCoordi 	=  0;
	}
	UpdatePackage.Coor_Addr		= pkg->slave_addr;
	UpdatePackage.Single_Addr_H	= pkg->single_addrH;
	UpdatePackage.Single_Addr_L 	= pkg->single_addrL;
	debug(DEBUG_update,"Addr:Coor_Addr=0x%02x,Single_Addr_H=0x%02x,,Single_Addr_L=0x%02x\n",
				UpdatePackage.Coor_Addr,UpdatePackage.Single_Addr_H,UpdatePackage.Single_Addr_L);
	temp = pkg->light_level;
	//pthread_mutex_lock(&mutex_task);	//获取task锁
	RetuenValue = Update_Init(temp) == SUCCESS ? (int)Update_Single_Coordi():-1;
	#endif
	//pthread_mutex_unlock(&mutex_task);	//解task锁
	switch(  RetuenValue  ){
		case SUCCESS:
			Close_Update();
			debug(DEBUG_update,"Update Success!\n");
			break;
		case Update_Pend:
			Close_Update();
			debug(DEBUG_update,"Update will be Pend!\n");
			break;
		case TIME_OUT:
			Close_Update();
			debug(DEBUG_update,"Update time out!\n");
			break;
		case -1:
			Close_Update();
			debug(DEBUG_update,"Update Init error!\n");
			break;
		default:break;

	}

	pthread_exit(NULL);
}

s32 UpdateThread(void * arg)
{
	int RetuenValue = -1;
	char temp = 0;

	memset(&UpdateInfo,0,sizeof(Package_Info));
	#ifdef Config_NewProtocol
		Pag_Single *package = (Pag_Single *)arg;
		if(package->Single_Addr[0] == 0 && package->Single_Addr[1] == 0){
			UpdateInfo.SinOrCoordi 	=  !0;
		}else{
			UpdateInfo.SinOrCoordi 	=  0;
		}

		UpdatePackage.Coor_Addr		= package->Coordi_Addr;
		UpdatePackage.Single_Addr_H	= package->Single_Addr[0];
		UpdatePackage.Single_Addr_L 	= package->Single_Addr[1];
		debug(DEBUG_update,"Addr:Coor_Addr=0x%02x,Single_Addr_H=0x%02x,Single_Addr_L=0x%02x\n",
					UpdatePackage.Coor_Addr,UpdatePackage.Single_Addr_H,UpdatePackage.Single_Addr_L);
		temp = package->Data[0];
		RetuenValue = Update_Init(temp) == SUCCESS ? (int)Update_Single_Coordi():-1;
	#else
		Frame_485  *pkg = (Frame_485 *)arg;
		
		if(pkg->single_addrH == 0 && pkg->single_addrL == 0){
			UpdateInfo.SinOrCoordi 	=  !0;
		}else{
			UpdateInfo.SinOrCoordi 	=  0;
		}
		UpdatePackage.Coor_Addr		= pkg->slave_addr;
		UpdatePackage.Single_Addr_H	= pkg->single_addrH;
		UpdatePackage.Single_Addr_L 	= pkg->single_addrL;
		debug(DEBUG_update,"Addr:Coor_Addr=0x%02x,Single_Addr_H=0x%02x,,Single_Addr_L=0x%02x\n",
					UpdatePackage.Coor_Addr,UpdatePackage.Single_Addr_H,UpdatePackage.Single_Addr_L);
		temp = pkg->light_level;
		RetuenValue = Update_Init(temp) == SUCCESS ? (int)Update_Single_Coordi():-1;
	#endif

	switch(  RetuenValue  ){
		case SUCCESS:
			Close_Update();
			break;
		case Update_Pend:
			Close_Update();
			debug(DEBUG_update,"Update will be Pend!\n");
			break;
		case TIME_OUT:
			Close_Update();
			debug(DEBUG_update,"Update time out!\n");
			break;
		case -1:
			Close_Update();
			debug(DEBUG_update,"Update Init error!\n");
			return FAIL;
		default:break;

	}
	return SUCCESS;
}
