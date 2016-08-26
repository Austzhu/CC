#include "serial.h"
#include "main.h"
 #ifndef  MC_ARM335X
#include "../emfuture/include/libs_emfuture_odm.h"
#endif

static s32 UartFd[TOTAL_UARTS_NUM] = {-1,-1,-1,-1,-1,-1};

/**
 *  打开一个串口
 *  port 	打开的端口
 *  返回值	成功返回:SUCESS 失败返回：FAIL
 */
s32 Uart_Open(u32  port)
{
	s8 dev[12];
	memset(dev,0,sizeof(dev));

 #ifndef  MC_ARM335X
	
	sprintf(dev,"/dev/ttyS%d", port);

	debug(DEBUG_ERR_RECD_SERIAL,"********%s\n",dev);
	if( ERROR_OK == OpenCom(dev, &UartFd[port],"9600,8,1,N,0") ){
		if(-1 != UartFd[port]){
			debug(DEBUG_ERR_RECD_SERIAL,"fd is %d\n",UartFd[port]);
			return SUCCESS;
		}	
	}else{
		goto err;
	}
 #else
	sprintf(dev,"/dev/ttyO%d", port);
	debug(DEBUG_Serial,"Serial: %s\n",dev);
	/* 读写，不阻塞 */
	if( (UartFd[port] = open( dev, O_RDWR|O_NOCTTY)) <0){
		debug(DEBUG_Serial,"Open %s Fail!\n",dev);
		goto err;
	}
	if( 0 == isatty(UartFd[port]) ){
		debug(DEBUG_Serial,"Is not tty devices!\n");
		close(UartFd[port]);
		UartFd[port] =-1;
		goto err;
	}else{
		debug(DEBUG_Serial,"open fd is %d\n",UartFd[port]);
	}
 #endif
	return SUCCESS;
 err:
	return FAIL;
}
/**
 * 关闭指定的串口
 * port 	关闭的端口
 * return:
 * 	成功返回 	ERROR_OK(==0)；
 *  	失败返回 	(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL
 */
s32 Uart_Close(u32 port)
{
 #ifndef  MC_ARM335X
	return CloseCom(UartFd[port]);
 #else
	close(UartFd[port]);
	return SUCCESS;
 #endif
}

void set_speed(int fd, int speed)
{
	struct termios   Opt;
	tcgetattr(fd, &Opt);
	
	tcflush(fd, TCIOFLUSH);
	cfsetispeed(&Opt, speed);
	cfsetospeed(&Opt, speed);
	if( 0 != tcsetattr(fd, TCSANOW, &Opt)){
		perror("tcsetattr fd err!");
		return;
	}
					
	tcflush(fd,TCIOFLUSH);
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if  ( tcgetattr( fd,&options)  !=  0){
		perror("SetupSerial 1");
		return FAIL;
	}
	options.c_cflag &= ~CSIZE;
	switch (databits){
		case 7:options.c_cflag |= CS7;break;
		case 8:options.c_cflag |= CS8;break;
		default:fprintf(stderr,"Unsupported data size\n");
			return FAIL;
	}
	switch (parity){
		case 'n':
		case 'N':options.c_cflag &= ~PARENB;   
			options.c_iflag &= ~INPCK;   break;
		case 'o':
		case 'O':options.c_cflag |= (PARODD | PARENB); 
			options.c_iflag |= INPCK;break;
		case 'e':
		case 'E':options.c_cflag |= PARENB;     
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;     break;
		case 'S':
		case 's':options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;
		default:fprintf(stderr,"Unsupported parity\n");
		return FAIL;
	}
	switch (stopbits){
		case 1:options.c_cflag &= ~CSTOPB;break;
		case 2:options.c_cflag |= CSTOPB;break;
		default:fprintf(stderr,"Unsupported stop bits\n");
			return FAIL;
	}
	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

	/* Set input parity option */
	if (parity != 'n'){options.c_iflag |= INPCK;}
	options.c_cc[VTIME] = 150; // 15 seconds
	options.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0){
		perror("SetupSerial 3");
		return FAIL;
	}
	return SUCCESS;
}
/**
 * 配置串口参数
 * 返回值:
 *	成功返回ERROR_OK(==0)；
 *	失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
 */
#if 1
s32 Uart_Config(u32 port, UartSpeed speed,  s32 bits,  s32 stop,s8 parity)
{
 #ifndef MC_ARM335X
	s8 cfg[24];
	memset(cfg,0,sizeof(cfg));
	sprintf(cfg,"%d,%d,%d,%c,0",speed,bits,stop,parity);
	debug(DEBUG_ERR_RECD_SERIAL,"^^^^^configs:%s\n",cfg);
	return SetComCfg(UartFd[port], cfg);
 #else
	int fd = UartFd[port];
	struct termios opt;
	//struct termios temp_opt;
	if( tcgetattr(fd, &opt) ){
		perror("tcgetattr fail!\n");
		goto err;
	}

	//tcflush(fd, TCIOFLUSH);
	/* 是否要增加判断speed是否在枚举内？Austzhu2016.5.7 */

	/* 设置波特率 */
	cfsetispeed(&opt, speed);
	cfsetospeed(&opt, speed);
	/* 设置数据位 */
	opt.c_cflag &= ~CSIZE;		//清0对应的bit
	switch(bits){
		case 7: opt.c_cflag |= CS7; break;
		case 8: opt.c_cflag |= CS8; break;
		default:goto err;
	}
	/* 设置停止位 */
	switch (stop){
		case 1: opt.c_cflag &= ~CSTOPB;	break;
		case 2: opt.c_cflag |= CSTOPB;	break;
		default:goto err;
	}
	/* 设置校验位 */
	switch(parity){
		case 'n':	//无校验
		case 'N':opt.c_cflag &= ~PARENB;   
			 opt.c_iflag &= ~INPCK;	break;
		case 'o':	//奇校验
		case 'O':opt.c_cflag |= (PARODD | PARENB); 
			opt.c_iflag |= INPCK;	break;
		case 'e':		//偶校验
		case 'E':opt.c_cflag |= PARENB;     
			opt.c_cflag &= ~PARODD;
			opt.c_iflag |= INPCK;	break;
		case 'S':		//空格
		case 's': opt.c_cflag &= ~PARENB;
			 opt.c_cflag &= ~CSTOPB;	break;
		 default:goto err;
	}

	opt.c_cflag |= CLOCAL;		//保证程序打开串口不阻塞
	opt.c_cflag |= CREAD;		//使得能够从串口中读取输入数据
	opt.c_cflag &= ~CRTSCTS;	//禁用硬件流控

	/*
	 * BREAK条件，接收到BREAK不产生信号，不标记奇偶校验，不剔除输入字符的第8位，
	 * 不将输入的NL转换为CR，不忽略CR，不将CR转换成NL，禁止输出流控，紧张输入流控
	 */
	opt.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF);
	opt.c_oflag &= ~OPOST;		//不执行输出处理，原始数据输出
	/* 不启用回显，不回显NL，不规范输入，终端产生的信号不起作用，扩充输入字符无效 */
	opt.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

	//if (parity != 'n'){opt.c_iflag |= INPCK;}
	opt.c_cc[VTIME] = 0;		//读取一个字符等待时间(150/10)S
	opt.c_cc[VMIN] = 0;		//读取字符的最少个数

	tcflush(fd,TCIOFLUSH);		//丢弃输入输出缓存数据
	/* 设置参数，立即生效 */
	if( tcsetattr(fd, TCSANOW, &opt)){
		perror("tcsetattr error!\n");
		goto err;
	}

	/* 把设置参数读回来校验 */
	// if( tcgetattr(fd, &temp_opt) ){
	// 	perror("Check tcgetattr fail!\n");
	// 	goto err;
	// }
	// if( memcmp(&temp_opt, &opt, sizeof(opt)) ){
	// 	perror("set teminal parameter fail!\n" );
	// 	goto err;	
	// }	
	return SUCCESS;
 err:
 	return FAIL;
 #endif
}
#else
s32 Uart_Config(u32 port, UartSpeed speed,  s32 bits,  s32 stop,s8 parity)
{
	int fd = UartFd[port];
 #ifndef MC_ARM335X
	s8 cfg[24];
	memset(cfg,0,sizeof(cfg));
	sprintf(cfg,"%d,%d,%d,%c,0",speed,bits,stop,parity);
	debug(DEBUG_ERR_RECD_SERIAL,"^^^^^configs:%s\n",cfg);
	return SetComCfg(UartFd[port], cfg);
 #else
	set_speed(fd, speed);
	return set_Parity(fd,bits,stop,parity);
 #endif
}

#endif
/**
 * 从串口中读取数据
 * port  端口
 * buf   数据缓存区
 * len   数据长度
 * block 是否阻塞，<=0表示阻塞，>0 等待的时间
 * 返回值:
 *	成功返回ERROR_OK(==0)；
 *	失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
 */
s32 Uart_Recv(u32  port,  s8* buf,   u32 len,  s32  block)
{
	int res = -1;
	memset(buf,0,len);
 	#ifdef SingleCheckThread
 		pthread_mutex_lock(&mutex_serial) ;//获取锁
 	#endif
 #ifndef MC_ARM335X
 	u32 WPtr = 0;
 	u32 ReadCnt = len;
 	int i = len/20 + 1;
 	int WiatTime = block/i;
 	while(WPtr < len ){
 		res = ReadCom(UartFd[port], buf+WPtr, &ReadCnt, WiatTime);
 		//debug(1,"*****Recv len=%d\n",ReadCnt);
 		WPtr += ReadCnt;
 		ReadCnt = len-WPtr;
 		if(i-- < 0){break;}	
 	}
	//res = ReadCom(UartFd[port], buf, &len, block);
	//debug(1,"*****Recv len=%d\n",len);

	#ifdef SingleCheckThread
 		pthread_mutex_unlock(&mutex_serial) ;//释放锁
 	#endif
		return res;
 #else
	fd_set fdset;
	struct timeval tv;
	int fd = UartFd[port];
	int Read_Cnt = 0;
	int n = 0;
	if(fd <=0){
		err_Print(DEBUG_Serial,"fd < 0!\n");
		return FAIL;
	}
	tv.tv_sec   = block/1000000;
	tv.tv_usec = block%1000000;
 R_continue:
	FD_ZERO(&fdset);
	FD_SET(fd,&fdset);
	switch(select(fd+1, &fdset,NULL,NULL,&tv)){
		case -1:
			debug(DEBUG_Serial,"select err:%s\n",strerror(errno));
			return FAIL;
		case 0: //tcflush(fd,TCIFLUSH);
			//debug(DEBUG_Serial,"Recv Timeout!\n");
			return FAIL;
		default:
			if(FD_ISSET(fd, &fdset)){
				n = read( fd, buf+Read_Cnt, len-Read_Cnt);
				Read_Cnt += n;
				if(Read_Cnt < len){
					debug(DEBUG_Serial,"Can't Recv enough long data!\n");
					//tcflush(fd,TCIOFLUSH);
					goto R_continue;
				}
				#ifdef SingleCheckThread
 					pthread_mutex_unlock(&mutex_serial) ;//释放锁
 				#endif
				return SUCCESS;
			}break;		
	}
	#ifdef SingleCheckThread
 		pthread_mutex_unlock(&mutex_serial) ;//释放锁
 	#endif
	return FAIL;
 #endif
}


/**
 * 从串口中写数据
 * port  端口
 * buf   数据缓存区
 * len   数据长度
 * block 是否阻塞，<=0表示阻塞，>0 等待的时间
 * 返回值:
 *	成功返回ERROR_OK(==0)；
 *	失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
 */
s32 Uart_Send(u32  port,  s8* buf,   u32 len,  s32  block)
{
	int res =-1;
	#ifdef SingleCheckThread
 		pthread_mutex_lock(&mutex_serial) ;//获取锁
 	#endif
 #ifndef MC_ARM335X
 		res = WriteCom(UartFd[port], buf, &len, block);
	#ifdef SingleCheckThread
 		pthread_mutex_unlock(&mutex_serial) ;//释放锁
 	#endif
 		return res;
 #else
	fd_set fdset;
	struct timeval tv;
	int fd = UartFd[port];
	int Read_Cnt = 0;
	int n=0;
	if(fd <=0){
		debug(DEBUG_Serial,"fd < 0!\n");
		return FAIL;
	}
	tv.tv_sec   = block/1000000;
	tv.tv_usec = block%1000000;
 S_continue:
	FD_ZERO(&fdset);
	FD_SET(fd,&fdset);
	switch(select(fd+1, NULL,&fdset,NULL,&tv)){
		case -1:
			debug(DEBUG_Serial,"select err!\n");
			return FAIL;
		case 0:tcflush(fd,TCOFLUSH);
			debug(DEBUG_Serial,"Recv Timeout!\n");
			return FAIL;
		default:if(FD_ISSET(fd, &fdset)){
			n = write( fd, buf+Read_Cnt, len-Read_Cnt);
			Read_Cnt += n;
			if(Read_Cnt < len){
				debug(DEBUG_Serial,"Can't write enough long data!\n");
				goto S_continue;
			}
			#ifdef SingleCheckThread
 				pthread_mutex_unlock(&mutex_serial) ;//释放锁
 			#endif
			return SUCCESS;
		}break;		
	}
	#ifdef SingleCheckThread
 		pthread_mutex_unlock(&mutex_serial) ;//释放锁
 	#endif
	return FAIL;
 #endif
}

s32 UartForGprsInit(void)
{
	Uart_Open(Uart1_ttyO1_485);
	Uart_Config(Uart1_ttyO1_485, B2400, 8,1,'N');
	printf("Uart1_ttyO1_Baud = 2400\n");
	return SUCCESS;
}

s32 UartForDISInit(void)
{
	Uart_Open(Uart0_ttyS1_DIS);
	Uart_Config(Uart0_ttyS1_DIS, Bd115200, 8,1,'N');
	return SUCCESS;
}

s32 UartForPlcInit()
{
	Uart_Open(Uart5_ttyS6_PLC);
	Uart_Config(Uart5_ttyS6_PLC, Bd9600, 8,1,'N');
	return SUCCESS;

}
s32 UartFor485Init()
{	
	Uart_Open(Uart2_ttyS3_485);
	Uart_Config(Uart2_ttyS3_485, Bd9600, 8,1,'N');											//无校验   n
	return SUCCESS;
}

s32 UartForCoordi(void)
{
	if(SUCCESS == Uart_Open(Uart1_ttyO1_485) ){
		#ifndef MC_ARM335X
			Uart_Config(Uart1_ttyO1_485, Bd9600, 8,1,'N');
		#else
			Uart_Config(Uart1_ttyO1_485, B9600, 8,1,'N');
		#endif
	}
	if(SUCCESS == Uart_Open(Uart3_tty03_DIDO)){
		#ifndef MC_ARM335X
			Uart_Config(Uart3_tty03_DIDO, Bd9600, 8,1,'N');
		#else
			Uart_Config(Uart3_tty03_DIDO, B9600, 8,1,'N');
		#endif
	}
	return SUCCESS;
}
/**
 * 获取串口的句柄
 * port 	串口的号
 * 返回值：
 * 	成功 串口的描述符，失败  -1
 */
s32 Uart_GetFd(u32  port)
{
	if(UartFd[port] <0){
		return -1;
	}
	return UartFd[port];
}

/****************************************************************************/



s32 UartBufClear(u32 port, Flush_IO IO)
{
	int res =0;
	if(IO&Flush_Input){
		res = tcflush(UartFd[port],TCIFLUSH);
	}
	if(IO&Flush_Output){
		res |= tcflush(UartFd[port],TCOFLUSH);
	}
	return res;
}


