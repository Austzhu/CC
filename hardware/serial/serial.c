/******************************************************************
** 文件名:	serial.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "serial.h"
#ifdef Config_EC_6018
#include "libs_emfuture_odm.h"
#endif

static int serial_Config(serial_t *this,u32 port,u32 speed,s32 bits,s32 stop,s8 parity)
{
	assert_param(this,FAIL);
#ifdef CFG_MC_ARM335X
	struct {int speed;int bspeed;} _speed[] = { {2400,B2400},{4800,B4800},{9600,B9600},
					{19200,B19200},{38400,B38400},{57600,B57600},{115200,B115200} };
	struct termios opt;
	int fd = this->serialfd[port];
	int set_speed = B9600;
	for(int i=0,size=sizeof(_speed)/sizeof(_speed[0]); i<size;++i)
		if( speed ==  _speed[i].speed){set_speed = _speed[i].bspeed;  break; }

	if( tcgetattr(fd, &opt) ) goto out;
	/* 设置波特率 */
	cfsetispeed(&opt, set_speed);
	cfsetospeed(&opt, set_speed);
	/* 设置数据位 */
	opt.c_cflag &= ~CSIZE;		//清0对应的bit
	switch(bits){
		case 7: opt.c_cflag |= CS7; break;
		case 8: opt.c_cflag |= CS8; break;
		default:goto out;
	}
	/* 设置停止位 */
	switch (stop){
		case 1: opt.c_cflag &= ~CSTOPB;	break;
		case 2: opt.c_cflag |= CSTOPB;	break;
		default:goto out;
	}
	/* 设置校验位 */
	switch(parity){
		case 'n':	//无校验
		case 'N': opt.c_cflag &= ~PARENB; opt.c_iflag &= ~INPCK; break;

		case 'o':	//奇校验
		case 'O': opt.c_cflag |= (PARODD | PARENB); opt.c_iflag |= INPCK; break;

		case 'e':	//偶校验
		case 'E':opt.c_cflag |= PARENB; opt.c_cflag &= ~PARODD;opt.c_iflag |= INPCK; break;

		case 'S':		//空格
		case 's': opt.c_cflag &= ~PARENB; opt.c_cflag &= ~CSTOPB; break;
		 default:	goto out;
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
	opt.c_cc[VMIN] = 0;			//读取字符的最少个数

	tcflush(fd,TCIOFLUSH);		//丢弃输入输出缓存数据
	/* 设置参数，立即生效 */
	if( tcsetattr(fd, TCSANOW, &opt)) goto out;
	return SUCCESS;
 out:
 	perror("error!");
 	return FAIL;
#else
 	return SetComCfg(this->serialfd[port], Asprintf("%d,%d,%d,%c,0",speed,bits,stop,parity));
#endif
}

static int serial_Open(serial_t *this,u32  port)
{
	if(port > SerialMax || !this) return FAIL;
#ifdef  CFG_MC_ARM335X
	this->serialfd[port] = open( Asprintf("/dev/ttyO%d",port), O_RDWR|O_NOCTTY);
	if(this->serialfd[port] < 0) goto out;
	if( !isatty(this->serialfd[port])){
		debug(DEBUG_Serial,"/dev/ttyO%d is't tty devices!\n",port);
		this->serial_close(this,port);
		return FAIL;
	}
#else
	this->serialfd[port] = open( Asprintf("/dev/ttyS%d",port), O_RDWR | O_NONBLOCK);
	if(this->serialfd[port] < 0) goto out;
#endif
	return SUCCESS;
 out:
	debug(DEBUG_Serial,"Open Serial port %d error!\n",port);
	return FAIL;
}

static int serial_Close(serial_t *this,u32  port)
{
	assert_param(this,FAIL);
	if( this->serialfd[port] > 0 ){
		close( this->serialfd[port] );
		this->serialfd[port] = -1;
	}
	return SUCCESS;
}

static int serial_Recv(serial_t *this,u32  port,s8* buf,u32 len,s32  block)
{
	assert_param(this,FAIL);
	assert_param(buf,FAIL);

	int fd = this->serialfd[port];
	if( fd < 0 ){
		return FAIL;
	}
	memset(buf,0,len);

	fd_set fdset;
	struct timeval tv;
	int Read_Cnt = 0;
	int n = 0;
	tv.tv_sec   = block/1000000;
	tv.tv_usec = block%1000000;
 R_continue:
	FD_ZERO(&fdset);
	FD_SET(fd,&fdset);
	switch(select(fd+1, &fdset,NULL,NULL,&tv)){
		case -1:
			debug(DEBUG_Serial,"select err:%s\n",strerror(errno));
			return FAIL;
		case 0: 		//Recv Timeout
			return FAIL;
		default:
			if(FD_ISSET(fd, &fdset)){
				//ioctl(fd,FIONREAD,&CombufSize);	//读出内核缓存中有多少字节的数据
				//debug(1,"Kernel Buffer size:%d,read size:%d\n",CombufSize,len);
				n = read( fd, buf+Read_Cnt, len-Read_Cnt);
				Read_Cnt += n;
				if(Read_Cnt < len){
					//debug(DEBUG_Serial,"Can't Recv enough long data!\n");
					goto R_continue;
				}return SUCCESS;
			}
			break;
	}
	return FAIL;
}

static int serial_Send(serial_t *this,u32  port,s8* buf,u32 len,s32  block)
{
	assert_param(this,FAIL);
	assert_param(buf,FAIL);

	int fd = this->serialfd[port];
	if( fd < 0 )  return FAIL;
	fd_set fdset;
	struct timeval tv;
	int Read_Cnt = 0;
	int n=0;
	tv.tv_sec   = block/1000000;
	tv.tv_usec = block%1000000;
 S_continue:
	FD_ZERO(&fdset);
	FD_SET(fd,&fdset);
	switch(select(fd+1, NULL,&fdset,NULL,&tv)){
		case -1:
			debug(DEBUG_Serial,"select err!\n");
			return FAIL;
		case 0:		//Send Time out
			debug(DEBUG_Serial,"Recv Timeout!\n");
			return FAIL;
		default:
		if(FD_ISSET(fd, &fdset)){
			n = write( fd, buf+Read_Cnt, len-Read_Cnt);
			Read_Cnt += n;
			if(Read_Cnt < len){
				//debug(DEBUG_Serial,"Can't write enough long data!\n");
				goto S_continue;
			}
			return SUCCESS;
		}
		break;
	}
	return FAIL;
}

static void serial_flush(serial_t *this,int port)
{
	assert_param(this,;);
	if(port < 0 || port >SerialMax) return;
	tcflush(this->serialfd[port],TCIFLUSH|TCOFLUSH);
}

static void serial_Relese(serial_t **this)
{
	assert_param(this,;);
	assert_param(*this,;);
	for(int i=0;i<SerialMax;++i)
		if( (*this)->serialfd[i] > 0 )
			close( (*this)->serialfd[i] );
	FREE(*this);
}

serial_t *serial_Init(serial_t *this,u32 port,...)
{
	int speed = 0;
	va_list 	arg_ptr;
	serial_t *temp = this;
	if(!this){
		temp = malloc(sizeof(serial_t));
		if(!temp) return NULL;
	}	bzero(temp,sizeof(serial_t));
	memset(temp->serialfd,-1,sizeof(temp->serialfd));

	temp->serial_open = serial_Open;
	temp->serial_close = serial_Close;
	temp->serial_config = serial_Config;
	temp->serial_recv = serial_Recv;
	temp->serial_send = serial_Send;
	temp->serial_flush = serial_flush;
	temp->serial_relese = serial_Relese;
	if( !temp->serial_open  ||  !temp->serial_close  || !temp->serial_config ||\
		 !temp->serial_recv  || !temp->serial_send  || !temp->serial_flush  || !temp->serial_relese)
		goto out1;

	va_start(arg_ptr,port);
	for(int i=0; i<SerialMax; ++i,port >>= 1){
		if(!(port&0x01)) continue;
		if( SUCCESS == temp->serial_open(temp,i) ){
			speed = va_arg(arg_ptr, int);
			if(SUCCESS != temp->serial_config(temp,i,speed,8,1,'N')) goto out;
		}else  goto out;
	}
	va_end(arg_ptr);
	return temp;
out:
	va_end(arg_ptr);
out1:
	if(!this)  FREE(temp);
	return NULL;
}
