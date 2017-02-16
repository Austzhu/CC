/******************************************************************
** 文件名:	UART.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "UART.h"

static int uart_open(struct uart_t *this)
{
	assert_param(this,FAIL);
#ifdef  CFG_MC_ARM335X
	this->uart_fd = open( Asprintf("/dev/ttyO%d",this->uart_port),  O_RDWR|O_NOCTTY);
	if(this->uart_fd < 0) goto out;
	if( !isatty(this->uart_fd) ){
		debug(DEBUG_UART,"/dev/ttyO%d is't tty devices!\n",this->uart_port);
		this->uart_close(this);
		return FAIL;
	}
#else
	this->uart_fd = open( Asprintf("/dev/ttyS%d",this->uart_port), O_RDWR | O_NONBLOCK);
	if(this->uart_fd < 0) goto out;
#endif
	return SUCCESS;
 out:
	debug(DEBUG_UART,"Open Serial port %d error!\n",this->uart_port);
	return FAIL;
}

static int uart_close(struct uart_t *this)
{
	assert_param(this,FAIL);
	if( this->uart_fd > 0 ){
		close( this->uart_fd );
		this->uart_fd = -1;
	}
	return SUCCESS;
}

static int uart_config(struct uart_t *this)
{
	assert_param(this,FAIL);
	if(this->uart_fd < 0){
		if(SUCCESS != uart_open(this))
			return FAIL;
	}
#ifdef CFG_MC_ARM335X
	struct termios opt;
	int fd = this->uart_fd;
	int set_speed = B9600;
	if( tcgetattr(fd, &opt) ) goto out;
	switch(this->uart_speed){
		case 2400:		set_speed = B2400; break;
		case 4800:		set_speed = B4800; break;
		case 9600:		set_speed = B9600; break;
		case 19200: 	set_speed = B19200; break;
		case 38400: 	set_speed = B38400; break;
		case 57600: 	set_speed = B57600; break;
		case 115200: 	set_speed = B115200; break;
		default : goto out;
	}
	/* 设置波特率 */
	cfsetispeed(&opt, set_speed);
	cfsetospeed(&opt, set_speed);
	/* 设置数据位 */
	opt.c_cflag &= ~CSIZE;		//清0对应的bit
	switch(this->uart_bits){
		case 7: opt.c_cflag |= CS7; break;
		case 8: opt.c_cflag |= CS8; break;
		default:goto out;
	}
	/* 设置停止位 */
	switch (this->uart_stop){
		case 1: opt.c_cflag &= ~CSTOPB;	break;
		case 2: opt.c_cflag |= CSTOPB;	break;
		default:goto out;
	}
	/* 设置校验位 */
	switch(this->uart_parity){
		case 'n':		//无校验
		case 'N': opt.c_cflag &= ~PARENB; opt.c_iflag &= ~INPCK; break;

		case 'o':	//奇校验
		case 'O': opt.c_cflag |= (PARODD | PARENB); opt.c_iflag |= INPCK; break;

		case 'e':		//偶校验
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
 	return SetComCfg(this->uart_fd , Asprintf("%d,%d,%d,%c,0",\
 		this->uart_speed,this->uart_bits, this->uart_stop, this->uart_parity));
#endif
}

static int uart_readall(struct uart_t *this, char *buf,uint32_t block)
{
	if(!this || !buf || this->uart_fd < 0) return FAIL;
	int fd = this->uart_fd;
	int length = 0;
	msleep(block);
	ioctl(fd,FIONREAD,&length);	//获取内核中缓存了多少数据
	return read(fd,buf,length);
}

static int uart_recv(struct uart_t *this, char *buf,uint32_t length,int32_t block)
{
	if(!this || !buf || this->uart_fd < 0) return FAIL;

	int fd = this->uart_fd;
	int rptr = 0, cnt = 0;
	fd_set fdset;
	block = (block<=0) ? 1000000 : block;
	struct timeval tv = {
		.tv_sec = block/1000000,
		.tv_usec= block%1000000,
	};
	bzero(buf,length);
	//ioctl(fd,FIONREAD,&CombufSize);	//读出内核缓存中有多少字节的数据
 repeat:
	FD_ZERO(&fdset);
	FD_SET(fd,&fdset);
	switch(select(fd+1, &fdset,NULL,NULL,&tv)){
		case -1:	debug(DEBUG_UART,"select err:%s\n",strerror(errno));
			return FAIL;
		case 0:	return FAIL;	/* time out */
		default:
		if(FD_ISSET(fd, &fdset)){
			cnt = read(fd,buf+rptr,length-rptr);
			if(cnt < 0){
				perror("uart read");
				return FAIL;
			}
			rptr += cnt;
			if(rptr < length) goto repeat;
			return SUCCESS;
		}	break;
	}	//end of switch
	return FAIL;
}

static int uart_send(struct uart_t *this,const char * buf,uint32_t length,int32_t block)
{
	if(!this || !buf || this->uart_fd < 0) return FAIL;

	int fd = this->uart_fd;
	int wptr = 0, cnt = 0;
	fd_set fdset;
	struct timeval tv = {
		.tv_sec   = block/1000000,
		.tv_usec = block%1000000,
	};
 repeat:
	FD_ZERO(&fdset);
	FD_SET(fd,&fdset);
	switch(select(fd+1, NULL,&fdset,NULL,&tv)){
		case -1:	debug(DEBUG_UART,"select err!\n");
			return FAIL;
		case 0:	return FAIL;	/* time out */
		default:
		if(FD_ISSET(fd, &fdset)){
			cnt = write( fd, buf+wptr, length-wptr);
			if(cnt < 0)	return FAIL;
			wptr += cnt;
			if(wptr < length) goto repeat;
			return SUCCESS;
		}	break;
	}	//end of switch
	return FAIL;
}

static void uart_flush(struct uart_t *this)
{
	if(!this) return ;
	tcflush(this->uart_fd,TCIFLUSH|TCOFLUSH);
}

static void uart_relese(struct uart_t **this)
{
	if(!this || !*this) return;
	(*this)->uart_close(*this);
	FREE(*this);
}


struct uart_t *uart_init(uart_t *this, uint8_t port,const char *cfg)
{
	assert_param(cfg,NULL);
	uart_t *pthis = this;
	if(!pthis){
		this = malloc(sizeof(uart_t));
		if(!this)	return NULL;
	}
	bzero(this,sizeof(uart_t));

	this->uart_open = uart_open;
	this->uart_close = uart_close;
	this->uart_config = uart_config;
	this->uart_readall = uart_readall;
	this->uart_recv = uart_recv;
	this->uart_send = uart_send;
	this->uart_flush = uart_flush;
	this->uart_relese = uart_relese;
	this->uart_fd = -1;

	const char*pcfg=cfg;
	this->uart_port = port;
	this->uart_speed = (uint32_t)strtol(cfg,NULL,10);
	while(*pcfg != ',' && *pcfg != '\0') ++pcfg;
	this->uart_bits = strtol(++pcfg,NULL,10);
	while(*pcfg != ',' && *pcfg != '\0') ++pcfg;
	this->uart_stop = strtol(++pcfg,NULL,10);
	while(*pcfg != ',' && *pcfg != '\0') ++pcfg;
	this->uart_parity = *(++pcfg);

	debug(DEBUG_UART,"CFG: speed=%d,bits=%d,stop=%d,parity=%c\n",\
		this->uart_speed,this->uart_bits,this->uart_stop,this->uart_parity);
	if(SUCCESS != this->uart_config(this)) goto out;

	return this;
out:
	if(!pthis) FREE(this);
	return NULL;
}
