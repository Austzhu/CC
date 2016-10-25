/******************************************************************
** 文件名:	Interface.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "Interface.h"
#include "single.h"

#define TopUser_Init(TopUser,class,func,Message,args...) do{\
	if(TopUser->class && TopUser->class->func && \
		SUCCESS == TopUser->class->func(TopUser->class,##args)){\
	}else{\
		printf(Message"\n");\
		return FAIL;\
	}\
}while(0)

static s32 UID_Check(appitf_t *this,void *r_uid)
{
	assert_param(this,NULL,FAIL);
	assert_param(r_uid,NULL,FAIL);
	u8 *pr_uid = r_uid;
	for(int i=0;i<6;++i)
		if( pr_uid[i] != this->param.CCUID[i])  return FAIL;
	return SUCCESS;
}

static s32 TopUser_ProcQue(appitf_t *this)
{
	assert_param(this,NULL,FAIL);
	if(SUCCESS != this->Queue->Task_Exec(this->Queue)) {
		this->msleep(5);		//执行失败睡眠5ms,否则cpu的占用率会很高
		return FAIL;
	}
	return SUCCESS;
}

static s32 TopUser_InsertQue(appitf_t *this)
{
	assert_param(this,NULL,FAIL);
	/* if the connect_status is't ok return */
	if(this->Connect_status != Connect_ok){
		this->msleep(10);
		return FAIL;
	}

	u8 recvbuf[300];
	if(SUCCESS != this->TopUserRecvPackage(this,recvbuf,300)) return FAIL;

	if(SUCCESS != this->UID_Check(this,&recvbuf[1]) ){		// 增加对集中器UID的判断
		debug(DEBUG_TaskAppend,"CC UID Unmatch!\n");
		return FAIL;
	}

	/* get Queue type and level
	 *  return_value&0xff is Queue type
	 *  (return_value>>8)&0xff is Queue level
	 *  return_value == -1,get Queue type error
	 */
	int Que_type = this->Queue->get_Quetype(this->Queue,recvbuf[8]);
	if(-1 == Que_type) return FAIL;

	if(SUCCESS != this->Queue->Task_Append(this->Queue,Que_type&0xff,(Que_type>>8) &0xff,&recvbuf[8],recvbuf[9]+2)){
		debug(1,"new task append error!\n");
		return FAIL;
	}
	return SUCCESS;
}

static int TopUser_Keepalive(appitf_t *this)
{
	assert_param(this,NULL,FAIL);

	if(this->HeartBeat_status == HeartBeat_ok){
		this->HeartBeat_status = HeartBeat_error;
		/* HeartBeat */
		if(this->ethernet && this->ethernet->ether_heartbeat && \
			SUCCESS != this->ethernet->ether_heartbeat(this->ethernet))
			return FAIL;
		this->msleep(1000*this->param.HeartBCycle);
	}else{
		this->Connect_status = Connect_error;
		/* 判断是否使用网络方式 */
		if(this->param.ItfWay == ether_net){
			/* connect to server */
			if(this->ethernet && this->ethernet->ether_connect)
				this->Connect_status = this->ethernet->ether_connect(this->ethernet) == \
				SUCCESS ? Connect_ok : Connect_error;
			/* logon and heartbeat */
			if(   this->Connect_status == Connect_ok /* connect ok */ &&\
				this->ethernet->ether_logon /* function logon is not null */ && \
				this->ethernet->ether_heartbeat /* function HeartBeat is not null */ && \
				SUCCESS == this->ethernet->ether_logon(this->ethernet) /* log on success? */&&\
				SUCCESS == this->ethernet->ether_heartbeat(this->ethernet) ){
				this->msleep(1000*this->param.HeartBCycle);
				return SUCCESS;
			}else{
				err_Print(1,"ethernet logon or heartbeat error!\n");
				this->Connect_status = Connect_error;
				this->msleep(3000);
				return FAIL;
			}
		}	//end of if(this->ItfWay == ether_net){
	}
	return SUCCESS;
}

static int package_check(void *package)
{
	assert_param(package,NULL,FAIL);

	int package_len = ((faalpkt_t*)package)->len + 10;
	u8 *pchar = (u8*)package;
	u8 chk = 0;

	for(int i=0;i<package_len;++i) chk += *pchar++;

	if(chk != *pchar++) return FAIL;
	if(*pchar != 0x16){
		debug(1,"package end error!\n");
		return FAIL;
	}
	return SUCCESS;
}
static int TopUser_RecvPackage(appitf_t *this,u8 *buffer,int bufsize)
{
	assert_param(this,NULL,FAIL);
	assert_param(buffer,NULL,FAIL);
	memset(buffer,0,bufsize);
	int res = -1;
	u8 *pchar = buffer;
	int len = 0;
	/* recv header */
	while(*buffer != 0x68){
		res = this->ethernet->ether_getchar(this->ethernet,pchar);
		if(res != SUCCESS) this->msleep(5);		//接收失败睡眠5ms,否则cpu的占用率会很高
	}
	pchar++;
	res = this->ethernet->ether_recv(this->ethernet,pchar,9);
	/* recv the ccuid ctr and len */
	if(res == 9){
		pchar += 9;
		len = *(pchar-1)+2;
	}else{
		debug(1,"^^^^^errno:%d\n",res);
		if( FAIL == res){
			this->Connect_status = Connect_error;
			this->HeartBeat_status = HeartBeat_error;
		}
		return FAIL;
	}
	/* check header 2 */
	if(buffer[7] != 0x68){
		debug(1,"[%02x]package header 2 error!\n",buffer[7]);
		return FAIL;
	}
	/* recv  remain data */
	res = this->ethernet->ether_recv(this->ethernet,pchar,len);
	if(res == len){
		/* check the package */
		if(SUCCESS != this->packagecheck(buffer) ){
			err_Print(1,"Recv package check error!\n");
			return FAIL;
		}
		this->Connect_status = Connect_ok;
		this->HeartBeat_status = HeartBeat_ok;
		return SUCCESS;
	}else if( FAIL == res){
		this->Connect_status = Connect_error;
		this->HeartBeat_status = HeartBeat_error;
	}
	return FAIL;
}

static void TopUser_usleep(u32 us)
{
	struct timeval tv;
	tv.tv_sec   = us/1000000;
	tv.tv_usec = us%1000000;
	select(1, NULL,NULL,NULL,&tv);
}

void TopUser_msleep(u32 ms)
{
	struct timeval tv;
	ms *= 1000;
	tv.tv_sec   = ms/1000000;
	tv.tv_usec = ms%1000000;
	select(0, NULL,NULL,NULL,&tv);
}

static int appitf_init(appitf_t *this)
{
	assert_param(this,NULL,FAIL);
	loadParam(&g_appity);
	this->Connect_status = Connect_error;
	this->HeartBeat_status = HeartBeat_error;

	/* init for Queue */
	TopUser_Init(this,Queue,Que_init,"Queue init error!",this);
	switch(this->param.ItfWay){
		case ether_net: 	/* init for ethernet */
			TopUser_Init(this,ethernet,ether_init,"ethernet init error!",this);
			break;
		case gprs:break;
		case zigbee:break;
		default:debug(DEBUG_app,"Unknow communication %d.\n",this->param.ItfWay); return FAIL;
	}
	/* init for serial */
	TopUser_Init(this,Serial,serial_init,"Serial init error!",(0x01<<COM_485) | (0x01<<COM_DIDO),9600,9600);
	/* init for sqlite */
	TopUser_Init(this,sqlite,sql_init,"Sqlite init error!");
	/* init for single */
	TopUser_Init(this,single,sin_init,"Single init error!",this);
	/* init for dido */
	#ifdef Config_Meter
	TopUser_Init(this,meter,meter_init,"meter init error!",this);
	#endif

	if( access("cc_corl.db",F_OK))
		system("./config/Create_Database.sh &");
	this->msleep(1000);
	return SUCCESS;
}
static char* Hex2Str(char*dest,const u8 *src,int size)
{
	assert_param(dest,NULL,NULL);
	assert_param(src,NULL,NULL);

	char *pdest = dest;
	char table[] = {"0123456789ABCDEF"};
	for(int i=0;i<size;++i){
		*pdest++ = table[(*src>>4)&0x0f];
		*pdest++ = table[*src&0x0f];
		++src;
	}
	return dest;
}

static u8 *Str2Hex(u8 *dest,const char *src)
{
	assert_param(dest,NULL,NULL);
	assert_param(src,NULL,NULL);

	int size = strlen(src);
	u8 s1 = 0,s2 = 0;
	u8 *pdest = dest;
	for(int i=0,len=size/2;i<len;++i){
		s1 = toupper(src[2*i]) - '0';
		s2 = toupper(src[2*i+1])-'0';
		s1 -= s1 > 9 ? 7 : 0;
		s2 -= s2 > 9 ? 7 : 0;
		*pdest++ =  (s1<<4) | s2;
	}
	if(size%2){
		s1 = toupper(*(src+size-1)) - '0';
		s1 -= s1 > 9 ? 7 : 0;
		*pdest = s1;
	}
	return dest;
}


extern int crc16_checkout(u8*,u8*,int);
extern int Crc16(u8*,u8*,int);

static int get_check(int cmd,u8*crc,u8 *pMessage,int len)
{
	assert_param(crc,NULL,FAIL);
	assert_param(pMessage,NULL,FAIL);
	return cmd == crc_get ? \
	Crc16(crc,pMessage,len) : crc16_checkout(crc,pMessage,len);
}

static int get_checkhl(int cmd,u8 *crc, u8 *message, int nLen)
{
	u8 _crc[2] = {0};
	Crc16(_crc,message,nLen);

	if(cmd == crc_get){
		crc[0] = _crc[1];crc[1] = _crc[0];
		return SUCCESS;
	}
	/* compare crc */
	return (_crc[0] == crc[1] && _crc[1] == crc[0] ) ? SUCCESS : FAIL;
}

appitf_t g_appity = {
	.Queue = &Que,
	.ethernet = &ethernet,
	.Serial = &g_serial,
	.sqlite = &g_sqlite,
	.single = &g_single,
	#ifdef Config_Meter
	.meter = &g_meter,
	#endif

	.app_Init = appitf_init,
	.UID_Check = UID_Check,
	.TopUserProcQue = TopUser_ProcQue,
	.TopUserInsertQue = TopUser_InsertQue,
	.TopUserKeepalive = TopUser_Keepalive,
	.TopUserRecvPackage = TopUser_RecvPackage,
	.packagecheck = package_check,
	.usleep = TopUser_usleep,
	.msleep = TopUser_msleep,
	.hex2str = Hex2Str,
	.str2hex = Str2Hex,
	.Crc16 = get_check,
	.Crc16_HL = get_checkhl,
};


u8 auchCRCHi[]={
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x00, 0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

u8 auchCRCLo[]={
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,0x07, 0xC7, 0x05, 0xC5, 0xC4,
	0x04, 0xCC, 0x0C, 0x0D, 0xCD,0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
	0x1D, 0x1C, 0xDC, 0x14, 0xD4,0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,0xF2, 0x32, 0x36, 0xF6, 0xF7,
	0x37, 0xF5, 0x35, 0x34, 0xF4,0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
	0x2E, 0x2F, 0xEF, 0x2D, 0xED,0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,0x61, 0xA1, 0x63, 0xA3, 0xA2,
	0x62, 0x66, 0xA6, 0xA7, 0x67,0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,0x78, 0xB8, 0xB9, 0x79, 0xBB,
	0x7B, 0x7A, 0xBA, 0xBE, 0x7E,0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,0x70, 0xB0, 0x50, 0x90, 0x91,
	0x51, 0x93, 0x53, 0x52, 0x92,0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,0x99, 0x59, 0x58, 0x98, 0x88,
	0x48, 0x49, 0x89, 0x4B, 0x8B,0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};


int Crc16(u8 *crc,u8 *puchMsg, int usDataLen)
{
	unsigned char uchCRCHi = 0xFF ; 	/* 低CRC 字节初始化 */
	unsigned char uchCRCLo = 0xFF ;
	unsigned long uIndex ;			/* CRC循环中的索引 */
	if(usDataLen<=0) return FAIL;
	/* 传输消息缓冲区 */
	while (usDataLen--){
		uIndex = uchCRCHi ^ *puchMsg++ ; 			/* 计算CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ; 	/* 计算CRC 高低字节对调*/
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	crc[0] = uchCRCLo;
	crc[1] = uchCRCHi;
	return SUCCESS;
}

int crc16_checkout(u8 *crc,  u8 *puchMsg,  s32 usDataLen)
{
	u8 _Crc[2] = {0};
	Crc16( _Crc, puchMsg, usDataLen);
	if(_Crc[0] == crc[0] && _Crc[1] == crc[1] ){
		return SUCCESS;
	}else{
		return FAIL;
	}
}
