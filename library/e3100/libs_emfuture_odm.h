/*
 * libs_emfuture_odm.h
 *
 *  Created on: 2016-3-3
 *      Author: Administrator
 */

#ifndef LIBS_EMFUTURE_ODM_H_
#define LIBS_EMFUTURE_ODM_H_

#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <dirent.h>
#include <termios.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/rtc.h>
#include <sys/wait.h>
#include <memory.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <getopt.h>
#include <sys/file.h>
#include <signal.h>
#include <linux/serial.h>

#define  EM_AT91_PD


#if defined(EM_AT91_PD)
#define CPU_AT91SAM9X5		1
#define CFG_USART_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_TIMER_LIB		1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_DAC_LIB			1
#define CFG_ADC_LIB			1
#elif defined(EM_AM335X_PD)
#define CPU_TIAM335X		1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#define CFG_DAC_LIB			1
#define CFG_ADC_LIB			1
#elif defined(EM_MCIMX6Q_PD)
#define CPU_MCIMX6Q			1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#define CFG_ADC_LIB			1
#define CFG_DAC_LIB			1
#elif defined(EM_LS1B_PD)
#define CPU_LOONGSON1B		1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#elif defined(EM_MPC83XX_PD)
#define CPU_MPC8315E		1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#endif

//错误码定义
#define ERROR_NOSUPPORT	-5		//当前不支持该功能
#define ERROR_SYS		-4		//系统API返回错误（系统调用返回错误）
#define ERROR_INPARA	-3		//参数错误（如参数不合法、缓冲区过小等）
#define ERROR_TIMEOUT	-2		//操作超时
#define ERROR_FAIL		-1		//其他错误，原因不明
#define ERROR_OK		0		//操作成功

//命令字定义
#define	EMFUTURE_IOCTL_BASE	'E'


//*-----------------------------------------------------------
/* IIO - useful set of util functionality
 *
 * Copyright (c) 2008 Jonathan Cameron
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

/* Made up value to limit allocation sizes */
#define IIO_MAX_NAME_LENGTH 30
#define FORMAT_SCAN_ELEMENTS_DIR "%s/scan_elements"
#define FORMAT_TYPE_FILE "%s_type"

extern const char *iio_dir;

/**
 * struct iio_channel_info - information about a given channel
 * @name: channel name
 * @generic_name: general name for channel type
 * @scale: scale factor to be applied for conversion to si units
 * @offset: offset to be applied for conversion to si units
 * @index: the channel index in the buffer output
 * @bytes: number of bytes occupied in buffer output
 * @mask: a bit mask for the raw output
 * @is_signed: is the raw value stored signed
 * @enabled: is this channel enabled
 **/
struct iio_channel_info {
	char *name;
	char *generic_name;
	float scale;
	float offset;
	unsigned index;
	unsigned bytes;
	unsigned bits_used;
	unsigned shift;
	unsigned mask;
	unsigned be;
	unsigned is_signed;
	unsigned enabled;
	unsigned location;
};

//*-----------------------------------------------------------



//下面是分别针对库所支持部件的函数描述
#ifdef CFG_USART_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**               串口设备相关部分
//**
//**
//**  注：若在同一个进程中需要对多个串口进行轮询读，  为达到更优的
//**      阻塞效果，建议直接使用select+read机制，
//**      而不采用库中读API函数ReadCom，其他函数仍可使用。
//*
//*-----------------------------------------------------------
//************************************************************

//串口的默认参数
#define COM_BAUDDEF		(115200)
#define	COM_DABITSDEF	(8)
#define COM_STBITSDEF	(1)
#define COM_PARITYDEF	('N')
#define COM_RTSCTSDEF	(0)
#define DEFCFGFORCOM 	"115200,8,1,N,0"

//读写宽度
typedef enum {
	RS232_MODE = 0,
	RS422_MODE,
	RS485_MODE
} com_type_t;

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  OpenCom
//* 功能描述:  打开并初始化串口
//* 入口参数:
//*           -- dev:      - 串口设备名，如/dev/ttyS1
//*           -- configs:  - 初始化通讯参数字符串:波特率,数据位，停止位，校验位，
//*                          如:   "115200,8,1,N"，若为NULL，则使用默认参数("115200,8,1,N")
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int OpenCom(char *dev, int* fd, char *configs);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetComCfg
//* 功能描述:  设置串口参数
//* 入口参数:
//*           -- fd:      - 串口设备文件句柄
//*           -- configs: - 通讯参数字符串:波特率,数据位，停止位，校验位
//*                         如:   "115200,8,1,N"
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int SetComCfg(int fd, char *configs);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetComMode
//* 功能描述:  设置串口工作模式(RS232/RS485/RS422)
//* 入口参数:
//*           -- dev:      - 串口设备名，如/dev/ttyS1
//*           -- mode: - 串口的工作模式
//*                         如:   RS232_MODE --- RS232
//*                         	      RS485_MODE --- RS485
//*                         	      RS422_MODE --- RS422
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int SetComMode(char *dev, com_type_t mode);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadCom
//* 功能描述:  读串口数据
//* 入口参数:
//*           -- fd:         - 指定串口设备的句柄(即OpenCom()的返回值)
//*           -- buffer:     - 读取的数据存放缓冲区
//*           -- len:        - 读取的数据长度(指针)，同时返回实际读取的数据数目
//*           -- timesoutus: - 最大的阻塞读取等待间隔，单位: useconds(微秒)
//*                            若timesoutus<=0，阻塞读模式，即直到读够指定数据个数后函数返回.
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、分配采样值存放缓冲区时，一定要足够存放指定的数据个数，否则可能造成缓冲区溢出；
//*           2、当置为阻塞读模式时，指定的读数据个数需要恰当，否则若没有相应个数数据，则进程会阻塞；
//*           3、成功读取数据大小，由参数len返回；
//*------------------------------------------------------------------------------------------------
extern int ReadCom(int fd, char *buffer, unsigned int *len, int timesoutus);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  WriteCom
//* 功能描述:  向串口写数据
//* 入口参数:
//*           -- fd:         - 指定串口设备的句柄(即OpenCom()的返回值)
//*           -- buffer:     - 读取的数据存放缓冲区
//*           -- len:        - 读取的数据长度(指针)，同时返回实际读取的数据数目
//*           -- timesoutus: - 最大的阻塞读取等待间隔，单位: useconds(微秒)
//*                            若timesoutus<=0，阻塞读模式，即直到读够指定数据个数后函数返回.
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、当置为阻塞写模式时，指定的数据个数需要恰当，否则若设备当前不可写的话，可能会造成进程阻塞;
//*           2、实际写的数据大小，由参数len返回;
//*------------------------------------------------------------------------------------------------
extern int WriteCom(int fd, char *buffer, unsigned int *len, int timesoutus);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  CloseCom
//* 功能描述:  关闭串口设备
//* 入口参数:
//*           -- fd:  - 指定串口设备的句柄(即OpenCom()的返回值)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int CloseCom(int fd);

#ifdef __cplusplus
}
#endif
#endif



#ifdef CFG_FIFO_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**               FIFO 设备相关部分
//**
//**
//**  注：若在同一个进程中需要对多个串口进行轮询读，  为达到更优的
//**      阻塞效果，建议直接使用select+read机制，
//**      而不采用库中读API函数ReadFifo，其他函数仍可使用。
//*
//*-----------------------------------------------------------
//************************************************************

//FIFO 的默认参数

#define PORTDEV		"/dev/fifo0"

//port的ioctl指令
#define FIFO_IOC_MAGIC       'p'
#define FIFO_RESET_IOCFG   _IOW(FIFO_IOC_MAGIC, 1, unsigned char)



#ifdef __cplusplus
extern "C" {
#endif

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadFifo
//* 功能描述:  读FIFO 数据
//* 入口参数:
//*           -- fd:         - 指定fifo 设备的句柄(即Open()的返回值)
//*           -- buffer:     - 读取的数据存放缓冲区
//*           -- len:        - 读取的数据长度(指针)，同时返回实际读取的数据数目
//*           -- timesoutus: - 最大的阻塞读取等待间隔，单位: useconds(微秒)
//*                            若timesoutus<=0，阻塞读模式，即直到读够指定数据个数后函数返回.
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、分配采样值存放缓冲区时，一定要足够存放指定的数据个数，否则可能造成缓冲区溢出；
//*           2、当置为阻塞读模式时，指定的读数据个数需要恰当，否则若没有相应个数数据，则进程会阻塞；
//*           3、成功读取数据大小，由参数len返回；
//*------------------------------------------------------------------------------------------------
extern int ReadFifo(int fd, char *buffer, unsigned int *len, int timesoutus);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  WriteFifo
//* 功能描述:  向串口写数据
//* 入口参数:
//*           -- fd:         - 指定fifo 设备的句柄(即Open()的返回值)
//*           -- buffer:     - 读取的数据存放缓冲区
//*           -- len:        - 读取的数据长度(指针)，同时返回实际读取的数据数目
//*           -- timesoutus: - 最大的阻塞读取等待间隔，单位: useconds(微秒)
//*                            若timesoutus<=0，阻塞读模式，即直到读够指定数据个数后函数返回.
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、当置为阻塞写模式时，指定的数据个数需要恰当，否则若设备当前不可写的话，可能会造成进程阻塞;
//*           2、实际写的数据大小，由参数len返回;
//*------------------------------------------------------------------------------------------------
extern int WriteFifo(int fd, char *buffer, unsigned int *len, int timesoutus);


#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_ADC_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*        ADC设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************

//定义采样值宽度(16-bit)
typedef unsigned short cyg_adc_sample;

//ADC设备的Ioctl命令接口定义
//不再使用
#define EM_SETADC_START_IOCFG      _IOWR(EMFUTURE_IOCTL_BASE, 20, int)
#define EM_SETADC_STOP_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 21, int)
#define EM_SETADC_RATE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 22, int)
#define EM_SETADC_MODE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 23, int)
#define EM_SETADC_ENCHAN_IOCFG     _IOWR(EMFUTURE_IOCTL_BASE, 24, int)
#define EM_SETADC_DISCHAN_IOCFG    _IOWR(EMFUTURE_IOCTL_BASE, 25, int)
#define EM_GETADC_STAT_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 26, int)
#define EM_GETADC_RATE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 27, int)
#define EM_GETADC_MODE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 28, int)
#define EM_SETADC_SWITCHKEY        _IOWR(EMFUTURE_IOCTL_BASE, 29, int)

//ADC的采样触发方式定义
//支持模式与硬件平台相关
//不再使用
#define EM_ADCTRIGER_ONESHOT 		_IOWR(EMFUTURE_IOCTL_BASE, 20, char)
#define EM_ADCTRIGER_SOFTPERIOD		_IOWR(EMFUTURE_IOCTL_BASE, 21, char)
#define EM_ADCTRIGER_HARDPERIOD		_IOWR(EMFUTURE_IOCTL_BASE, 22, char)

//AD采样值变换结构(cyg_adc_sample -> float)
//不再使用
struct __sample_to_float {
	cyg_adc_sample ivalue;
	float fvalue;
};

#ifdef __cplusplus
extern "C" {
#endif

//*------------------------------------------------------------------------------------------------
//* 函数名称:  OpenAdc
//* 功能描述:  打开ADC设备
//* 入口参数:
//*           -- dev:        -不再使用，为保证兼容性，指定为NULL；
//*           -- keys:       -不再使用，为保证兼容性，指定为NULL
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//*------------------------------------------------------------------------------------------------
extern int OpenAdc(char *dev, struct __sample_to_float *keys);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  CloseAdc
//* 功能描述 ：空函数，为保证兼容性
//* 入口参数:  无
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int CloseAdc(void);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetAdcCfg
//* 功能描述:  对ADC设备进行配置
//* 备注    :  不再使用，为保证兼容性，为空函数
//*------------------------------------------------------------------------------------------------
extern int SetAdcCfg(int chan, int cmd, char *buffer, unsigned int len);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetAdcCfg
//* 功能描述:  读取ADC设备的当前配置
//* 备注    :  不再使用，为保证兼容性，为空函数
//*------------------------------------------------------------------------------------------------
extern int GetAdcCfg(int chan, int cmd, char *buffer, unsigned int *len);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadAdcSampling
//* 功能描述:  读取ADC的采样值
//* 入口参数:
//*           -- chan:     - 读取ADC采样通道号
//*                          若chan>0,  读取指定的某单一通道
//*                          若chan==0, 读取所有通道
//*           -- buffer:   - 采样值缓冲区
//*           -- len:      - 读取采样次数，同时返回实际读取的采样值数目。
//*                          若chan>0, len表示该通道的采样次数
//*                          若chan=0, len表示每一通道的采样次数
//*           -- type:     - 数据类型
//*                          若type=0,则读取为转换的原始采样值
//*                          若type=1,则读取转换后的模拟量
//*           -- timesout： - 该参数不再使用，为保证兼容性，指定为0
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、分配采样值存放缓冲区时，一定要足够存放指定的采样值个数：
//*              若chan > 0时： 若读取原始值，则缓冲区大小 >=  len * sizeof(cyg_adc_sample)
//*              				若读取转换后的浮点数，则缓冲区大小 >=  len * sizeof(float)
//*              若chan = 0时： 若读取原始值，则缓冲区大小 >=  有效总通道数 * len * sizeof(cyg_adc_sample)
//*              				若读取转换后的浮点数，则缓冲区大小 >=  有效总通道数 * len * sizeof(float)
//*------------------------------------------------------------------------------------------------
extern int ReadAdcSampling(unsigned char chan, void* buffer, unsigned int *len, unsigned char type, int timesout);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadAdcMinScale
//* 功能描述:  读取ADC最小采样比例尺数值
//* 入口参数:
//*           -- val:  - ADC最小输出比例尺量，浮点数(单位:毫伏mv)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、val为ADC当前采样最小比例尺值，模拟量（单位:毫伏mv）
//*------------------------------------------------------------------------------------------------
extern int ReadAdcMinScale(float *val);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadAdcValidChans
//* 功能描述:  读取Adc当前有效总通道数
//* 入口参数:
//*           -- val:  - 返回Adc当前有效通道数
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注      :  1、val为板载Adc有效通道数
//*------------------------------------------------------------------------------------------------
extern int ReadAdcValidChans(int *val);

#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_DAC_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**   DA设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************

//默认DA输出宽度(16-bit)
typedef unsigned short cyg_dac_out;

//DA采样值变换结构(float -> cyg_dac_out)
//不再使用
struct __float_to_dacout {
	float fvalue;
	cyg_dac_out ivalue;
};

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  OpenDac
//* 功能描述:  打开DAC设备
//* 入口参数:
//*           -- dev:        -不再使用，为保证兼容性，指定为NULL；
//*           -- keys:       -不再使用，为保证兼容性，指定为NULL
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//*------------------------------------------------------------------------------------------------
extern int OpenDac(char *dev, struct __float_to_dacout *keys);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetDacCfg
//* 功能描述:  设置DAC各通道转换参数
//* 备注    :  不再使用，为保证兼容性，为空函数
//*------------------------------------------------------------------------------------------------
extern int SetDacCfg(int chan, struct __float_to_dacout *keys);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  WriteDacRawVal
//* 功能描述:  向DAC输出数据，数据为转换后的整数值
//* 入口参数:
//*           -- chan:    - 设置DAC通道号
//*                         若chan>0, 则只输出某单一通道
//*                         若chan==0, 则所有通道输出
//*           -- intval:  - 输出数字量
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、intval为拟输出的16位数字量
//*------------------------------------------------------------------------------------------------
extern int WriteDacRawVal(int chan,unsigned short intval);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  WriteDacfloatVal
//* 功能描述:  向DAC输出数据，数据为未经转换的模拟量
//* 入口参数:
//*           -- chan:    - 设置DAC通道号
//*                         若chan>0, 则只输出某单一通道
//*                         若chan==0, 则所有通道输出
//*           -- val:  - 输出模拟量，浮点数
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、val为输出的模拟量（浮点数）
//*------------------------------------------------------------------------------------------------
extern int WriteDacfloatVal(int chan,float val);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadDacRawVal
//* 功能描述:  读取DAC当前输出值，数据为转换后的整数值
//* 入口参数:
//*           -- chan:    - 设置DAC通道号
//*                         若chan>0, 则指某单一通道
//*                         若chan==0, 代表所有通道输出值
//*           -- intval:  - 当前输出数字量
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、intval为拟输出的16位数字量
//*------------------------------------------------------------------------------------------------
extern int ReadDacRawVal(int chan,unsigned short *intval);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadDacfloatVal
//* 功能描述:  读取DAC当前输出值，数据为转换后的浮点数值
//* 入口参数:
//*           -- chan:    - 设置DAC通道号
//*                         若chan>0, 则只输出某单一通道
//*                         若chan==0, 则所有通道输出
//*           -- val:  - 当前输出模拟量，浮点数(单位:毫伏mv)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、val为当前输出的模拟量（单位:毫伏mv）
//*------------------------------------------------------------------------------------------------
extern int ReadDacfloatVal(int chan,float *val);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadDacMinScale
//* 功能描述:  读取DAC最小输出比例尺数值
//* 入口参数:
//*           -- val:  - DAC最小输出比例尺量，浮点数(单位:毫伏mv)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、val为当前输出的最小输出比例数值，模拟量（单位:毫伏mv）
//*------------------------------------------------------------------------------------------------
extern int ReadDacMinScale(float *val);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadDacValidChans
//* 功能描述:  读取DAC当前有效通道数
//* 入口参数:
//*           -- val:  - 返回DAC当前有效通道数
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注      :  1、val为板载DAC有效通道数
//*------------------------------------------------------------------------------------------------
extern int ReadDacValidChans(int *val);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  CloseDac
//* 功能描述 ：空函数，为保证兼容性
//* 入口参数:  无
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int CloseDac(void);
#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_ENGPIO_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**   GPIO设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetEnIOOutState
//* 功能描述:  设置GPIO为电平输出，并设置IO的输出状态
//* 入口参数:
//*           -- gpio:    - IO端口号
//*           -- state:   - IO输出状态(0:Low/1:High)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :   无
//*------------------------------------------------------------------------------------------------
extern int SetEnIOOutState(int gpio, int state);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetEnIOOutState
//* 功能描述:  读取为电平输出类型IO的当前输出状态
//* 入口参数:
//*           -- gpio:     - IO端口号
//*           -- curstate: - 返回IO的当前输出状态(0:Low/1:High)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :   无
//*------------------------------------------------------------------------------------------------
extern int GetEnIOOutState(int gpio,int *curstate);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetEnIOPulseState
//* 功能描述:  设置GPIO为脉冲输出，并指定输出参数
//* 入口参数:
//*           -- gpio:       - IO端口号
//*           -- hitime10us: - 指定脉冲高电平输出时间，单位10us
//*           -- period10us: - 指定脉冲输出周期，单位：10us
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、hitime10us和period10us指定单位均为10us
//*           2、特殊值：若hitime10us==0，则一直输出低电平
//*           3、特殊值，若hitime10us!=0,period10us==0,则只输出一次hitime10us时间的高电平脉冲
//*           4、特殊值，若hitime10us>=period10us>0,则一直输出高电平
//*------------------------------------------------------------------------------------------------
extern int SetEnIOPulseState(int gpio, int hitime10us, int period10us);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetEnIOPulseCfg
//* 功能描述:  读取为脉冲输出类型IO的当前脉冲参数
//* 入口参数:
//*           -- gpio:       - IO端口号
//*           -- hitime10us: - 返回脉冲输出的高电平输出时间参数，单位10us
//*           -- period10us: - 返回脉冲输出的当前输出周期参数，单位：10us
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、hitime10us和period10us指定单位均为10us
//*------------------------------------------------------------------------------------------------
extern int GetEnIOPulseCfg(int gpio, int *hitime10us, int *period10us);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetEnIOInValue
//* 功能描述:  读取为电平输入类型IO的当前输入值
//* 入口参数:
//*           -- gpio:     - IO端口号
//*           -- curval:   - 返回IO的当前输入状态(0:Low/1:High)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :   无
//*------------------------------------------------------------------------------------------------
extern int GetEnIOInValue(int gpio,int *curval);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetEnIOEventCfg
//* 功能描述:  设置GPIO为事件IO输入，并指定事件触发参数
//* 入口参数:
//*           -- gpio:       - IO端口号
//*           -- func:     	 - IO事件回调执行函数，即检测到IO事件发生，该函数会被触发执行
//*           -- funcmode:   - 回调函数响应模式，即同时检测到IO事件发生，回调函数执行模式
//*                            0: 执行一次；1：执行次数与检测到IO事件数量相同
//*           -- trigermode: - IO事件类型
//*                            0: HI_to_LOW；1：LOW_to_HI；2：ALL
//*           -- filter10us:   - 防抖滤波参数，单位：10us,即有效电平持续时间
//*                          若电平持续时间<filter10us,则假定为干扰,事件不响应
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、func为回调函数指针，若为NULL,则清除该事件IO。
//*              因此当需要将事件IO重置为为正常输入IO时，需指定func为NULL并调用SetEventIOCfg函数
//*           2、func回调函数的参数即为当前gpio
//*------------------------------------------------------------------------------------------------
extern int SetEnIOEventCfg(int gpio,void(*func)(int gpio),int funcmode,int trigermode,int filter10us);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetEnIOEventCfg
//* 功能描述:  读取输入类型为事件IO的参数
//* 入口参数:
//*           -- gpio:       - IO端口号
//*           -- trigermode: - 返回IO事件检测类型
//*                          0: HI_to_LOW；1：LOW_to_HI；2：ALL
//*           -- filter10us: - 返回脉冲防抖参数，单位：10us
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  若返回ERROR_INPARA，则说明该可能没有设定为事件IO，或参数传入错误
//*------------------------------------------------------------------------------------------------
extern int GetEnIOEventCfg(int gpio,int *trigermode,int *filter10us);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_DATETIM_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**   RTC时间设置相关部分
//*
//*-----------------------------------------------------------
//************************************************************

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetRtcTime
//* 功能描述:  读取RTC时间
//* 入口参数:
//*           -- data:        - 返回当前RTC时间
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、数组data中时间规则如下：
//*              data[0] --- year
//*              data[1] --- month
//*              data[2] --- day
//*              data[3] --- hour
//*              data[4] --- minute
//*              data[5] --- second
//*              data[6] --- wday
//*------------------------------------------------------------------------------------------------
extern int GetRtcTime(unsigned short* data);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetRtcTime
//* 功能描述:  设置RTC时间
//* 入口参数:
//*           -- configs:     - 拟设定时间参数
//*           -- syn:         - 同步系统时间(0-NO/1-Yes)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、数组configs中时间规则如下：
//*              configs[0] --- year
//*              configs[1] --- month
//*              configs[2] --- day
//*              configs[3] --- hour
//*              configs[4] --- minute
//*              configs[5] --- second
//*------------------------------------------------------------------------------------------------
extern int SetRtcTime (unsigned short* configs, unsigned char syn);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_TIMER_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*               定时器相关部分
//*
//*-----------------------------------------------------------
//************************************************************

//定时器执行函数指针定义
typedef void (*FUNC) (int sig);

//定时器触发模式
#define	EMTIMER_SINSHOT_MODE 	    _IOWR(EMFUTURE_IOCTL_BASE, 40, int) // 单次触发
#define	EMTIMER_PERSHOT_MODE 		_IOWR(EMFUTURE_IOCTL_BASE, 41, int) // 周期性触发

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  CreateTimer
//* 功能描述:  创建用户定时器
//* 入口参数:
//*           -- handler:    - 定时器的触发函数句柄
//*           -- args:       - 传递给触发函数的参数
//*           -- usecs:      - 定时器的定时间隔，单位: useconds(微秒)
//*           -- mode :      - 定时器的触发模式：
//*                            EMTIMER_SINSHOT_MODE 	    ―― 单次触发
//*                            EMTIMER_PERSHOT_MODE 		―― 周期性触发
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int CreateTimer(FUNC handler, void *args, unsigned int usecs, int mode);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  UserusDelayms
//* 功能描述:  用户层延时函数（相对精确的毫秒级延时）
//* 入口参数:
//*           -- millsecs:    - 延时大小(单位：毫秒ms)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int UserusDelayms(unsigned int millsecs);
#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_HIGHTIMER_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*               高精度定时器相关部分
//*               （需要硬件支持）
//*-----------------------------------------------------------
//************************************************************

//定时器执行函数指针定义
typedef void (*HFUNC) (int sig, siginfo_t *extra, void *args);

//高精度定时器触发模式
#define	EMTIMER_HIGHSINSHOT_MODE 	    _IOWR(EMFUTURE_IOCTL_BASE, 42, int) // 单次触发
#define	EMTIMER_HIGHSPERSHOT_MODE 		_IOWR(EMFUTURE_IOCTL_BASE, 43, int) // 周期性触发

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  GreateHighResTimer
//* 功能描述:  创建用户高精度定时器
//* 入口参数:
//*           -- handler:    - 定时器的触发函数句柄
//*           -- timerid:    - 高精度定时器句柄指针（供删除定时器时使用）
//*           -- nsecs:      - 定时器的定时间隔，单位: neconds(纳秒)
//*           -- mode :      - 定时器的触发模式：
//*                            EMTIMER_HIGHSINSHOT_MODE 	  ―― 单次触发
//*                            EMTIMER_HIGHSPERSHOT_MODE      ―― 周期性触发
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、该函数需要平台支持高精度定时器功能
//*           2、高精度定时器使用完后，需要调用定时器删除函数（参数timerid），否则可能会影响系统效率
//*           3、当项目使用高精度定时器后，需要在编译链接程序时指定“-lrt”。
//*------------------------------------------------------------------------------------------------
extern int GreateHighResTimer(HFUNC handler, timer_t *timerid, unsigned int nsecs,int mode);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  DelHighResTimer
//* 功能描述:  删除高精度定时器
//* 入口参数:
//*           -- timerid:    - 高精度定时器句柄指针（由GreateHighResTimer返回）
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、参数必须是GreateHighResTimer返回的定时器句柄
//*------------------------------------------------------------------------------------------------
extern int DelHighResTimer(timer_t timerid);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  UserusDelayus
//* 功能描述:  用户层延时函数（相对精确的微秒级延时）
//* 入口参数:
//*           -- usecs:    - 延时大小(单位：微妙us)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int UserusDelayus(unsigned int usecs);

#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_PORT_LIB
//************************************************************
//************************************************************
//*
//**   端口设备相关部分
//*
//************************************************************
//************************************************************

//读写宽度
typedef enum {
	PORT_WIDTH_8 = 0,
	PORT_WIDTH_16,
	PORT_WIDTH_32
} port_wid_t;

//合法的BANK编号
typedef enum {
	PORT_BANK_2 = 0,
	PORT_BANK_4,
	PORT_BANK_5,
	PORT_BANK_6,
	PORT_BANK_7,
} port_bank_t;

//BANK属性
typedef struct {
	port_bank_t bank;
	port_wid_t width;
	short reserved;
} port_attr_t;

//
typedef struct {
	int regAddr;
	int regData;
} port_data_t;

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  OpenPortDev
//* 功能描述:  打开端口设备
//* 入口参数:
//*           -- dev:         - 端口设备名称，如/dev/port5
//*           -- fd:          - 返回文件句柄
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int OpenPortDev(char *dev,int* fd);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  ClosePortDev
//* 功能描述:  关闭端口设备
//* 入口参数:
//*           -- fd:          - 文件句柄
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int ClosePortDev(int fd);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetPortCfg
//* 功能描述:  设置端口属性
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- bank:        - 端口的Bank配置
//*           -- width:       - 端口的读写宽度(8/16/32),与硬件平台有关
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  1、端口的Bank及读写width由硬件配置决定
//*------------------------------------------------------------------------------------------------
extern int SetPortCfg(int fd, unsigned char bank, unsigned char width);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetPortCfg
//* 功能描述:  获取端口属性
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- bank:        - 返回端口的Bank配置
//*           -- width:       - 返回端口的读写宽度(8/16/32)
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int GetPortCfg(int fd, unsigned char *bank, unsigned char *width);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  InPortw
//* 功能描述:  16-bit读端口
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- addr:        - 读取的端口偏差地址
//*
//* 返回值    :  成功返回读取值，否则返回-1
//* 备注         :  1、addr必须2字节对齐
//*------------------------------------------------------------------------------------------------
extern inline short InPortw(int fd, unsigned long addr);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  OutPortw
//* 功能描述:  16-bit写端口
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- addr:        - 端口的偏差地址
//*           -- data:        - 数值
//*
//* 返回值    :  成功返回ERROR_OK，否则返回ERROR_FAIL
//* 备注         :  1、addr必须2字节对齐
//*------------------------------------------------------------------------------------------------
extern inline int OutPortw(int fd, unsigned long addr, short data);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  InPortl
//* 功能描述:  32-bit读端口
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- addr:        - 端口的偏差地址
//*
//* 返回值    :  成功返回读取值，否则返回-1
//* 备注         :  1、addr必须4字节对齐
//*------------------------------------------------------------------------------------------------
extern inline int InPortl(int fd, unsigned long addr);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  OutPortl
//* 功能描述:  32-bit写端口
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- addr:        - 端口的偏差地址
//*           -- data:        - 数值
//*
//* 返回值    :  成功返回ERROR_OK，否则返回ERROR_FAIL
//* 备注         :  1、addr必须4字节对齐
//*------------------------------------------------------------------------------------------------
extern inline int OutPortl(int fd, unsigned long addr, int data);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  InPortb
//* 功能描述:  8-bit读端口
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- addr:        - 读取的端口偏差地址
//*
//* 返回值    :  成功返回读取值，否则返回-1
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern inline char InPortb(int fd, unsigned long addr);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  OutPortb
//* 功能描述:  8-bit写端口
//* 入口参数:
//*           -- fd:          - 文件句柄(即OpenPortDev()的返回值)
//*           -- addr:        - 端口的偏差地址
//*           -- data:        - 数值
//*
//* 返回值    :  成功返回ERROR_OK，否则返回ERROR_FAIL
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern inline int OutPortb(int fd, unsigned long addr, char data);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_WDT_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*                看门狗设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  EnableWtd
//* 功能描述:  使能看门狗设备
//* 入口参数:
//*           -- TimeOut:     - 指定看门狗的最大喂狗间隔（秒）
//*
//* 返回值：      成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:     无
//*------------------------------------------------------------------------------------------------
extern int EnableWtd(int TimeOut);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  FeedWtd
//* 功能描述:  喂狗操作
//* 入口参数:  无
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int FeedWtd(void);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  DisableWtd
//* 功能描述:  关闭看门狗
//* 入口参数:  无
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int DisableWtd(void);
#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_KBD_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*                键盘设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************

//区分弹起/按下的阈值（需与驱动关联）
#define UP_THRESHOLD_DOWN	(0x80)

//
#define DOWN_KEY			(0x0)
#define UP_KEY				(0x1)

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  OpenKbd
//* 功能描述:  打开键盘设备
//* 入口参数:
//*           -- dev:     - 键盘设备文件，如/dev/kbd0
//*           -- fd:      - 返回文件句柄
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int OpenKbd(char *dev,int *fd);


//*------------------------------------------------------------------------------------------------
//* 函数名称:  FlushKbd
//* 功能描述:  清空键盘缓冲区
//* 入口参数:
//*           -- fd:      - 文件句柄
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int FlushKbd(int fd);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GetKeyValue
//* 功能描述:  读取键盘值及对应状态
//* 入口参数:
//*           -- fd:         - 文件句柄
//*           -- keyValue:   - 返回按键值
//*           -- status:     - 返回按键的当前操作状态(1：UP/0：DOWN)
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int GetKeyValue(int fd, unsigned short *keyValue, unsigned char *status);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  CloseKbd
//* 功能描述:  关闭键盘设备，释放相关资源
//* 入口参数:
//*           -- fd:         - 文件句柄，由OpenKbd返回
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int CloseKbd(int fd);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_MISCS_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*               ID加密芯片设备相关部分
//*						需硬件支持
//*
//*-----------------------------------------------------------
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadSerialID
//* 功能描述:  读取串行ID(DI)设备的唯一ID号
//* 入口参数:
//*           -- id:        - 返回ID数据数组
//*           -- len:       - ID数组的长度
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int ReadSerialID(unsigned char* id, unsigned char* len);
#ifdef __cplusplus
}
#endif

//************************************************************
//*-----------------------------------------------------------
//*
//*               温湿度传感设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************
/******   TM control cmd   *********/
#define	EM_SET_SENSOR_RES 	    _IOWR(EMFUTURE_IOCTL_BASE, 30, int) // 设置采样分辨率
#define	EM_GET_SENSOR_RES 		_IOWR(EMFUTURE_IOCTL_BASE, 31, int) //  读取当前采样分辨率

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetSensorCfg
//* 功能描述:  配置温度传感器的工作参数
//* 入口参数:
//*           -- cmd:        - 用户命令码
//*                            EM_SET_SENSOR_RES 	    ―― 设置采样分辨率
//*                            EM_GET_SENSOR_RES 		―― 读取当前采样分辨率
//*           -- arg:        - 传入/传出设置参数
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注         :  无
//*------------------------------------------------------------------------------------------------
extern int SetSensorCfg(int cmd, unsigned short* arg);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  ReadSensorVal
//* 功能描述:  读取传感器的温度值
//* 入口参数:
//*           -- value:        - 返回温度值
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注      :  读取值=实际值*放大倍数(10000)
//*------------------------------------------------------------------------------------------------
extern int ReadSensorVal(unsigned int* value);

#ifdef __cplusplus
}
#endif

//************************************************************
//*-----------------------------------------------------------
//*
//**   蜂鸣器和运行LED设备相关部分
//*
//*-----------------------------------------------------------
//************************************************************

#ifdef __cplusplus
extern "C" {
#endif

//*------------------------------------------------------------------------------------------------
//* 函数名称:  IoctlBuzzer
//* 功能描述:  操作蜂鸣器设备
//* 入口参数:
//*           -- ulaston10ms:   - 鸣叫持续时间，单位10毫秒。
//*           -- ulastoff10ms:  - 响铃关闭时间，单位10毫秒。
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:  	  1、ulaston10ms与ulastoff10ms的单位均为10毫秒；
//*           2、ulaston10ms与ulastoff10ms的取值规则：
//*			     若ulaston10ms = 0 ----  关闭蜂鸣器
//*				 若ulaston10ms = 0xFFFF ---- 蜂鸣器持续鸣叫
//*				 若ulaston10ms >0 && ulastoff10ms=0  ------ 蜂鸣器鸣叫1次，持续时间ulaston10ms
//*				 若ulaston10ms >0 && ulastoff10ms>0  ------ 蜂鸣器周期性鸣叫
//*				 											鸣叫时间ulaston10ms，周期(ulaston10ms+ulastoff10ms)
//*------------------------------------------------------------------------------------------------
extern int IoctlBuzzer(unsigned short ulaston10ms, unsigned short ulastoff10ms);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  IoctlRunLed
//* 功能描述:  操作运行指示灯(Running LED)
//* 入口参数:
//*           -- ulaston10ms:   - 运行指示灯亮持续时间，单位10毫秒。
//*           -- ulastoff10ms:  - 运行指示灯关闭时间，单位10毫秒。
//*
//* 返回值    :  成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:  	  1、ulaston10ms与ulastoff10ms的单位均为10毫秒；
//*           2、ulaston10ms与ulastoff10ms的取值规则：
//*			     若ulaston10ms = 0 ----  关闭运行指示灯
//*				 若ulaston10ms = 0xFFFF ---- 运行指示灯持续亮
//*				 若ulaston10ms >0 && ulastoff10ms=0  ------ 运行指示灯闪烁1次，持续时间ulaston10ms
//*				 若ulaston10ms >0 && ulastoff10ms>0  ------ 运行指示灯周期性闪烁
//*				 											亮持续时间ulaston10ms，周期(ulaston10ms+ulastoff10ms)
//*------------------------------------------------------------------------------------------------
extern int IoctlRunLed(unsigned short ulaston10ms, unsigned short ulastoff10ms);

#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_GPIO_LIB
//************************************************************
//************************************************************
//*
//**   用户层的GPIO操作函数
//*
//************************************************************
//************************************************************


//GPIO端口定义
#define PORTA_GPIO(x)	x
#define PORT0_GPIO(x)	PORTA_GPIO(x)
#define PORTB_GPIO(x)	(32+x)
#define PORT1_GPIO(x)	PORTB_GPIO(x)
#define PORTC_GPIO(x)	(64+x)
#define PORT2_GPIO(x)	PORTC_GPIO(x)
#define PORTD_GPIO(x)	(96+x)
#define PORT3_GPIO(x)	PORTD_GPIO(x)
#define PORTE_GPIO(x)	(128+x)
#define PORT4_GPIO(x)	PORTE_GPIO(x)
#define PORTF_GPIO(x)	(160+x)
#define PORT5_GPIO(x)	PORTF_GPIO(x)

//端口类型
typedef enum {
	PIO_MODE_OUT = 0,
	PIO_MODE_IN,
} gpio_mode_t;


#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* 函数名称:  SetGPIOCfg
//* 功能描述:  设置GPIO的端口属性
//* 入口参数:
//*           -- gpio:         - gpio端口
//*           -- attr:         - 指定GPIO的属性（模式）
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int SetGPIOCfg(int gpio, gpio_mode_t attr);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GPIOOutValue
//* 功能描述:  GPIO端口输出
//* 入口参数:
//*           -- gpio:         - gpio端口
//*           -- val:          - 指定输出状态
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int GPIOOutValue(int gpio, int val);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  GPIOInValue
//* 功能描述:  读GPIO端口输入
//* 入口参数:
//*           -- gpio:         - gpio端口
//*           -- state:        - 返回GPIO输入状态
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int GPIOInValue(int gpio,int *state);

//*------------------------------------------------------------------------------------------------
//* 函数名称:  FreeGPIOCfg
//* 功能描述:  释放GPIO控制
//* 入口参数:
//*           -- gpio:         - gpio端口
//*
//* 返回值：        成功返回ERROR_OK(==0)；失败返回(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* 备注:      无
//*------------------------------------------------------------------------------------------------
extern int FreeGPIOCfg(int gpio);

#ifdef __cplusplus
}
#endif
#endif

//************************************************************
//************************************************************
//*
//**   其他辅助函数
//*
//************************************************************
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#if 0
#if defined(CPU_AT91SAM9263) || defined(CPU_AT91SAM9260) || defined(CPU_AT91SAM9G20) \
		|| defined(CPU_AT91SAM9G45) || defined(CPU_AT91SAM9X5) || defined(CPU_TIAM335X)

extern inline unsigned short _inw(unsigned long addr);
extern inline void _outw(unsigned long addr, unsigned short dat);
extern inline unsigned long _inl(unsigned long addr);
extern inline void _outl(unsigned long addr, unsigned long dat);
extern inline unsigned char _inb(unsigned long addr);
extern inline void _outb(unsigned long addr, unsigned char dat);

#endif
#endif

extern void _nops(unsigned long nums);
extern int _Readn(int fd, void *buf, int n);
extern int _Writen(int fd, const void *buf, int n);

#ifdef __cplusplus
}
#endif

#define DEBUG
#ifdef DEBUG
#define TRACE(str, args...)	printf(str, ## args)
#else
#define TRACE(str, args...)
#endif

#endif /* LIBS_EMFUTURE_ODM_H_ */
/*
 * libs_emfuture_odm.h
 *
 *  Created on: 2016-3-3
 *      Author: Administrator
 */

#ifndef LIBS_EMFUTURE_ODM_H_
#define LIBS_EMFUTURE_ODM_H_

#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <dirent.h>
#include <termios.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/rtc.h>
#include <sys/wait.h>
#include <memory.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <getopt.h>
#include <sys/file.h>
#include <signal.h>

//struct serial_rs485
#include <linux/serial.h>


#if defined(EM_AT91_PD)
#define CPU_AT91SAM9X5		1
#define CFG_USART_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_TIMER_LIB		1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_DAC_LIB			1
#define CFG_ADC_LIB			1
#elif defined(EM_AM335X_PD)
#define CPU_TIAM335X		1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#define CFG_DAC_LIB			1
#define CFG_ADC_LIB			1
#elif defined(EM_MCIMX6Q_PD)
#define CPU_MCIMX6Q			1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#define CFG_ADC_LIB			1
#define CFG_DAC_LIB			1
#elif defined(EM_LS1B_PD)
#define CPU_LOONGSON1B		1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#elif defined(EM_MPC83XX_PD)
#define CPU_MPC8315E		1
#define CFG_USART_LIB		1
#define CFG_GPIO_LIB		1
#define CFG_ENGPIO_LIB		1
#define CFG_DATETIM_LIB		1
#define CFG_TIMER_LIB		1
#define CFG_HIGHTIMER_LIB	1
#define CFG_WDG_LIB			1
#define CFG_MISCS_LIB		1
#endif

//´íÎóÂë¶¨Òå
#define ERROR_NOSUPPORT	-5		//µ±Ç°²»Ö§³Ö¸Ã¹¦ÄÜ
#define ERROR_SYS		-4		//ÏµÍ³API·µ»Ø´íÎó£¨ÏµÍ³µ÷ÓÃ·µ»Ø´íÎó£©
#define ERROR_INPARA	-3		//²ÎÊý´íÎó£¨Èç²ÎÊý²»ºÏ·¨¡¢»º³åÇø¹ýÐ¡µÈ£©
#define ERROR_TIMEOUT	-2		//²Ù×÷³¬Ê±
#define ERROR_FAIL		-1		//ÆäËû´íÎó£¬Ô­Òò²»Ã÷
#define ERROR_OK		0		//²Ù×÷³É¹¦

//ÃüÁî×Ö¶¨Òå
#define	EMFUTURE_IOCTL_BASE	'E'


//*-----------------------------------------------------------
/* IIO - useful set of util functionality
 *
 * Copyright (c) 2008 Jonathan Cameron
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

/* Made up value to limit allocation sizes */
#define IIO_MAX_NAME_LENGTH 30
#define FORMAT_SCAN_ELEMENTS_DIR "%s/scan_elements"
#define FORMAT_TYPE_FILE "%s_type"

extern const char *iio_dir;

/**
 * struct iio_channel_info - information about a given channel
 * @name: channel name
 * @generic_name: general name for channel type
 * @scale: scale factor to be applied for conversion to si units
 * @offset: offset to be applied for conversion to si units
 * @index: the channel index in the buffer output
 * @bytes: number of bytes occupied in buffer output
 * @mask: a bit mask for the raw output
 * @is_signed: is the raw value stored signed
 * @enabled: is this channel enabled
 **/
struct iio_channel_info {
	char *name;
	char *generic_name;
	float scale;
	float offset;
	unsigned index;
	unsigned bytes;
	unsigned bits_used;
	unsigned shift;
	unsigned mask;
	unsigned be;
	unsigned is_signed;
	unsigned enabled;
	unsigned location;
};

//*-----------------------------------------------------------



//ÏÂÃæÊÇ·Ö±ðÕë¶Ô¿âËùÖ§³Ö²¿¼þµÄº¯ÊýÃèÊö
#ifdef CFG_USART_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**               ´®¿ÚÉè±¸Ïà¹Ø²¿·Ö
//**
//**
//**  ×¢£ºÈôÔÚÍ¬Ò»¸ö½ø³ÌÖÐÐèÒª¶Ô¶à¸ö´®¿Ú½øÐÐÂÖÑ¯¶Á£¬  Îª´ïµ½¸üÓÅµÄ
//**      ×èÈûÐ§¹û£¬½¨ÒéÖ±½ÓÊ¹ÓÃselect+read»úÖÆ£¬
//**      ¶ø²»²ÉÓÃ¿âÖÐ¶ÁAPIº¯ÊýReadCom£¬ÆäËûº¯ÊýÈÔ¿ÉÊ¹ÓÃ¡£
//*
//*-----------------------------------------------------------
//************************************************************

//´®¿ÚµÄÄ¬ÈÏ²ÎÊý
#define COM_BAUDDEF		(115200)
#define	COM_DABITSDEF	(8)
#define COM_STBITSDEF	(1)
#define COM_PARITYDEF	('N')
#define COM_RTSCTSDEF	(0)
#define DEFCFGFORCOM 	"115200,8,1,N,0"

//¶ÁÐ´¿í¶È
typedef enum {
	RS232_MODE = 0,
	RS422_MODE,
	RS485_MODE
} com_type_t;

// #ifdef __cplusplus
// extern "C" {
// #endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OpenCom
//* ¹¦ÄÜÃèÊö:  ´ò¿ª²¢³õÊ¼»¯´®¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- dev:      - ´®¿ÚÉè±¸Ãû£¬Èç/dev/ttyS1
//*           -- configs:  - ³õÊ¼»¯Í¨Ñ¶²ÎÊý×Ö·û´®:²¨ÌØÂÊ,Êý¾ÝÎ»£¬Í£Ö¹Î»£¬Ð£ÑéÎ»£¬
//*                          Èç:   "115200,8,1,N"£¬ÈôÎªNULL£¬ÔòÊ¹ÓÃÄ¬ÈÏ²ÎÊý("115200,8,1,N")
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int OpenCom(char *dev, int* fd, char *configs);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetComCfg
//* ¹¦ÄÜÃèÊö:  ÉèÖÃ´®¿Ú²ÎÊý
//* Èë¿Ú²ÎÊý:
//*           -- fd:      - ´®¿ÚÉè±¸ÎÄ¼þ¾ä±ú
//*           -- configs: - Í¨Ñ¶²ÎÊý×Ö·û´®:²¨ÌØÂÊ,Êý¾ÝÎ»£¬Í£Ö¹Î»£¬Ð£ÑéÎ»
//*                         Èç:   "115200,8,1,N"
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int SetComCfg(int fd, char *configs);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetComMode
//* ¹¦ÄÜÃèÊö:  ÉèÖÃ´®¿Ú¹¤×÷Ä£Ê½(RS232/RS485/RS422)
//* Èë¿Ú²ÎÊý:
//*           -- dev:      - ´®¿ÚÉè±¸Ãû£¬Èç/dev/ttyS1
//*           -- mode: - ´®¿ÚµÄ¹¤×÷Ä£Ê½
//*                         Èç:   RS232_MODE --- RS232
//*                         	      RS485_MODE --- RS485
//*                         	      RS422_MODE --- RS422
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int SetComMode(char *dev, com_type_t mode);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadCom
//* ¹¦ÄÜÃèÊö:  ¶Á´®¿ÚÊý¾Ý
//* Èë¿Ú²ÎÊý:
//*           -- fd:         - Ö¸¶¨´®¿ÚÉè±¸µÄ¾ä±ú(¼´OpenCom()µÄ·µ»ØÖµ)
//*           -- buffer:     - ¶ÁÈ¡µÄÊý¾Ý´æ·Å»º³åÇø
//*           -- len:        - ¶ÁÈ¡µÄÊý¾Ý³¤¶È(Ö¸Õë)£¬Í¬Ê±·µ»ØÊµ¼Ê¶ÁÈ¡µÄÊý¾ÝÊýÄ¿
//*           -- timesoutus: - ×î´óµÄ×èÈû¶ÁÈ¡µÈ´ý¼ä¸ô£¬µ¥Î»: useconds(Î¢Ãë)
//*                            Èôtimesoutus<=0£¬×èÈû¶ÁÄ£Ê½£¬¼´Ö±µ½¶Á¹»Ö¸¶¨Êý¾Ý¸öÊýºóº¯Êý·µ»Ø.
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢·ÖÅä²ÉÑùÖµ´æ·Å»º³åÇøÊ±£¬Ò»¶¨Òª×ã¹»´æ·ÅÖ¸¶¨µÄÊý¾Ý¸öÊý£¬·ñÔò¿ÉÄÜÔì³É»º³åÇøÒç³ö£»
//*           2¡¢µ±ÖÃÎª×èÈû¶ÁÄ£Ê½Ê±£¬Ö¸¶¨µÄ¶ÁÊý¾Ý¸öÊýÐèÒªÇ¡µ±£¬·ñÔòÈôÃ»ÓÐÏàÓ¦¸öÊýÊý¾Ý£¬Ôò½ø³Ì»á×èÈû£»
//*           3¡¢³É¹¦¶ÁÈ¡Êý¾Ý´óÐ¡£¬ÓÉ²ÎÊýlen·µ»Ø£»
//*------------------------------------------------------------------------------------------------
extern int ReadCom(int fd, char *buffer, unsigned int *len, int timesoutus);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  WriteCom
//* ¹¦ÄÜÃèÊö:  Ïò´®¿ÚÐ´Êý¾Ý
//* Èë¿Ú²ÎÊý:
//*           -- fd:         - Ö¸¶¨´®¿ÚÉè±¸µÄ¾ä±ú(¼´OpenCom()µÄ·µ»ØÖµ)
//*           -- buffer:     - ¶ÁÈ¡µÄÊý¾Ý´æ·Å»º³åÇø
//*           -- len:        - ¶ÁÈ¡µÄÊý¾Ý³¤¶È(Ö¸Õë)£¬Í¬Ê±·µ»ØÊµ¼Ê¶ÁÈ¡µÄÊý¾ÝÊýÄ¿
//*           -- timesoutus: - ×î´óµÄ×èÈû¶ÁÈ¡µÈ´ý¼ä¸ô£¬µ¥Î»: useconds(Î¢Ãë)
//*                            Èôtimesoutus<=0£¬×èÈû¶ÁÄ£Ê½£¬¼´Ö±µ½¶Á¹»Ö¸¶¨Êý¾Ý¸öÊýºóº¯Êý·µ»Ø.
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢µ±ÖÃÎª×èÈûÐ´Ä£Ê½Ê±£¬Ö¸¶¨µÄÊý¾Ý¸öÊýÐèÒªÇ¡µ±£¬·ñÔòÈôÉè±¸µ±Ç°²»¿ÉÐ´µÄ»°£¬¿ÉÄÜ»áÔì³É½ø³Ì×èÈû;
//*           2¡¢Êµ¼ÊÐ´µÄÊý¾Ý´óÐ¡£¬ÓÉ²ÎÊýlen·µ»Ø;
//*------------------------------------------------------------------------------------------------
extern int WriteCom(int fd, char *buffer, unsigned int *len, int timesoutus);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  CloseCom
//* ¹¦ÄÜÃèÊö:  ¹Ø±Õ´®¿ÚÉè±¸
//* Èë¿Ú²ÎÊý:
//*           -- fd:  - Ö¸¶¨´®¿ÚÉè±¸µÄ¾ä±ú(¼´OpenCom()µÄ·µ»ØÖµ)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int CloseCom(int fd);

#ifdef __cplusplus
}
#endif
#endif



#ifdef CFG_FIFO_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**               FIFO Éè±¸Ïà¹Ø²¿·Ö
//**
//**
//**  ×¢£ºÈôÔÚÍ¬Ò»¸ö½ø³ÌÖÐÐèÒª¶Ô¶à¸ö´®¿Ú½øÐÐÂÖÑ¯¶Á£¬  Îª´ïµ½¸üÓÅµÄ
//**      ×èÈûÐ§¹û£¬½¨ÒéÖ±½ÓÊ¹ÓÃselect+read»úÖÆ£¬
//**      ¶ø²»²ÉÓÃ¿âÖÐ¶ÁAPIº¯ÊýReadFifo£¬ÆäËûº¯ÊýÈÔ¿ÉÊ¹ÓÃ¡£
//*
//*-----------------------------------------------------------
//************************************************************

//FIFO µÄÄ¬ÈÏ²ÎÊý

#define PORTDEV		"/dev/fifo0"

//portµÄioctlÖ¸Áî
#define FIFO_IOC_MAGIC       'p'
#define FIFO_RESET_IOCFG   _IOW(FIFO_IOC_MAGIC, 1, unsigned char)



#ifdef __cplusplus
extern "C" {
#endif

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadFifo
//* ¹¦ÄÜÃèÊö:  ¶ÁFIFO Êý¾Ý
//* Èë¿Ú²ÎÊý:
//*           -- fd:         - Ö¸¶¨fifo Éè±¸µÄ¾ä±ú(¼´Open()µÄ·µ»ØÖµ)
//*           -- buffer:     - ¶ÁÈ¡µÄÊý¾Ý´æ·Å»º³åÇø
//*           -- len:        - ¶ÁÈ¡µÄÊý¾Ý³¤¶È(Ö¸Õë)£¬Í¬Ê±·µ»ØÊµ¼Ê¶ÁÈ¡µÄÊý¾ÝÊýÄ¿
//*           -- timesoutus: - ×î´óµÄ×èÈû¶ÁÈ¡µÈ´ý¼ä¸ô£¬µ¥Î»: useconds(Î¢Ãë)
//*                            Èôtimesoutus<=0£¬×èÈû¶ÁÄ£Ê½£¬¼´Ö±µ½¶Á¹»Ö¸¶¨Êý¾Ý¸öÊýºóº¯Êý·µ»Ø.
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢·ÖÅä²ÉÑùÖµ´æ·Å»º³åÇøÊ±£¬Ò»¶¨Òª×ã¹»´æ·ÅÖ¸¶¨µÄÊý¾Ý¸öÊý£¬·ñÔò¿ÉÄÜÔì³É»º³åÇøÒç³ö£»
//*           2¡¢µ±ÖÃÎª×èÈû¶ÁÄ£Ê½Ê±£¬Ö¸¶¨µÄ¶ÁÊý¾Ý¸öÊýÐèÒªÇ¡µ±£¬·ñÔòÈôÃ»ÓÐÏàÓ¦¸öÊýÊý¾Ý£¬Ôò½ø³Ì»á×èÈû£»
//*           3¡¢³É¹¦¶ÁÈ¡Êý¾Ý´óÐ¡£¬ÓÉ²ÎÊýlen·µ»Ø£»
//*------------------------------------------------------------------------------------------------
extern int ReadFifo(int fd, char *buffer, unsigned int *len, int timesoutus);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  WriteFifo
//* ¹¦ÄÜÃèÊö:  Ïò´®¿ÚÐ´Êý¾Ý
//* Èë¿Ú²ÎÊý:
//*           -- fd:         - Ö¸¶¨fifo Éè±¸µÄ¾ä±ú(¼´Open()µÄ·µ»ØÖµ)
//*           -- buffer:     - ¶ÁÈ¡µÄÊý¾Ý´æ·Å»º³åÇø
//*           -- len:        - ¶ÁÈ¡µÄÊý¾Ý³¤¶È(Ö¸Õë)£¬Í¬Ê±·µ»ØÊµ¼Ê¶ÁÈ¡µÄÊý¾ÝÊýÄ¿
//*           -- timesoutus: - ×î´óµÄ×èÈû¶ÁÈ¡µÈ´ý¼ä¸ô£¬µ¥Î»: useconds(Î¢Ãë)
//*                            Èôtimesoutus<=0£¬×èÈû¶ÁÄ£Ê½£¬¼´Ö±µ½¶Á¹»Ö¸¶¨Êý¾Ý¸öÊýºóº¯Êý·µ»Ø.
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢µ±ÖÃÎª×èÈûÐ´Ä£Ê½Ê±£¬Ö¸¶¨µÄÊý¾Ý¸öÊýÐèÒªÇ¡µ±£¬·ñÔòÈôÉè±¸µ±Ç°²»¿ÉÐ´µÄ»°£¬¿ÉÄÜ»áÔì³É½ø³Ì×èÈû;
//*           2¡¢Êµ¼ÊÐ´µÄÊý¾Ý´óÐ¡£¬ÓÉ²ÎÊýlen·µ»Ø;
//*------------------------------------------------------------------------------------------------
extern int WriteFifo(int fd, char *buffer, unsigned int *len, int timesoutus);


#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_ADC_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*        ADCÉè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************

//¶¨Òå²ÉÑùÖµ¿í¶È(16-bit)
typedef unsigned short cyg_adc_sample;

//ADCÉè±¸µÄIoctlÃüÁî½Ó¿Ú¶¨Òå
//²»ÔÙÊ¹ÓÃ
#define EM_SETADC_START_IOCFG      _IOWR(EMFUTURE_IOCTL_BASE, 20, int)
#define EM_SETADC_STOP_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 21, int)
#define EM_SETADC_RATE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 22, int)
#define EM_SETADC_MODE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 23, int)
#define EM_SETADC_ENCHAN_IOCFG     _IOWR(EMFUTURE_IOCTL_BASE, 24, int)
#define EM_SETADC_DISCHAN_IOCFG    _IOWR(EMFUTURE_IOCTL_BASE, 25, int)
#define EM_GETADC_STAT_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 26, int)
#define EM_GETADC_RATE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 27, int)
#define EM_GETADC_MODE_IOCFG       _IOWR(EMFUTURE_IOCTL_BASE, 28, int)
#define EM_SETADC_SWITCHKEY        _IOWR(EMFUTURE_IOCTL_BASE, 29, int)

//ADCµÄ²ÉÑù´¥·¢·½Ê½¶¨Òå
//Ö§³ÖÄ£Ê½ÓëÓ²¼þÆ½Ì¨Ïà¹Ø
//²»ÔÙÊ¹ÓÃ
#define EM_ADCTRIGER_ONESHOT 		_IOWR(EMFUTURE_IOCTL_BASE, 20, char)
#define EM_ADCTRIGER_SOFTPERIOD		_IOWR(EMFUTURE_IOCTL_BASE, 21, char)
#define EM_ADCTRIGER_HARDPERIOD		_IOWR(EMFUTURE_IOCTL_BASE, 22, char)

//AD²ÉÑùÖµ±ä»»½á¹¹(cyg_adc_sample -> float)
//²»ÔÙÊ¹ÓÃ
struct __sample_to_float {
	cyg_adc_sample ivalue;
	float fvalue;
};

#ifdef __cplusplus
extern "C" {
#endif

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OpenAdc
//* ¹¦ÄÜÃèÊö:  ´ò¿ªADCÉè±¸
//* Èë¿Ú²ÎÊý:
//*           -- dev:        -²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Ö¸¶¨ÎªNULL£»
//*           -- keys:       -²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Ö¸¶¨ÎªNULL
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//*------------------------------------------------------------------------------------------------
extern int OpenAdc(char *dev, struct __sample_to_float *keys);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  CloseAdc
//* ¹¦ÄÜÃèÊö £º¿Õº¯Êý£¬Îª±£Ö¤¼æÈÝÐÔ
//* Èë¿Ú²ÎÊý:  ÎÞ
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int CloseAdc(void);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetAdcCfg
//* ¹¦ÄÜÃèÊö:  ¶ÔADCÉè±¸½øÐÐÅäÖÃ
//* ±¸×¢    :  ²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Îª¿Õº¯Êý
//*------------------------------------------------------------------------------------------------
extern int SetAdcCfg(int chan, int cmd, char *buffer, unsigned int len);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetAdcCfg
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ADCÉè±¸µÄµ±Ç°ÅäÖÃ
//* ±¸×¢    :  ²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Îª¿Õº¯Êý
//*------------------------------------------------------------------------------------------------
extern int GetAdcCfg(int chan, int cmd, char *buffer, unsigned int *len);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadAdcSampling
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ADCµÄ²ÉÑùÖµ
//* Èë¿Ú²ÎÊý:
//*           -- chan:     - ¶ÁÈ¡ADC²ÉÑùÍ¨µÀºÅ
//*                          Èôchan>0,  ¶ÁÈ¡Ö¸¶¨µÄÄ³µ¥Ò»Í¨µÀ
//*                          Èôchan==0, ¶ÁÈ¡ËùÓÐÍ¨µÀ
//*           -- buffer:   - ²ÉÑùÖµ»º³åÇø
//*           -- len:      - ¶ÁÈ¡²ÉÑù´ÎÊý£¬Í¬Ê±·µ»ØÊµ¼Ê¶ÁÈ¡µÄ²ÉÑùÖµÊýÄ¿¡£
//*                          Èôchan>0, len±íÊ¾¸ÃÍ¨µÀµÄ²ÉÑù´ÎÊý
//*                          Èôchan=0, len±íÊ¾Ã¿Ò»Í¨µÀµÄ²ÉÑù´ÎÊý
//*           -- type:     - Êý¾ÝÀàÐÍ
//*                          Èôtype=0,Ôò¶ÁÈ¡Îª×ª»»µÄÔ­Ê¼²ÉÑùÖµ
//*                          Èôtype=1,Ôò¶ÁÈ¡×ª»»ºóµÄÄ£ÄâÁ¿
//*           -- timesout£º - ¸Ã²ÎÊý²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Ö¸¶¨Îª0
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢·ÖÅä²ÉÑùÖµ´æ·Å»º³åÇøÊ±£¬Ò»¶¨Òª×ã¹»´æ·ÅÖ¸¶¨µÄ²ÉÑùÖµ¸öÊý£º
//*              Èôchan > 0Ê±£º Èô¶ÁÈ¡Ô­Ê¼Öµ£¬Ôò»º³åÇø´óÐ¡ >=  len * sizeof(cyg_adc_sample)
//*              				Èô¶ÁÈ¡×ª»»ºóµÄ¸¡µãÊý£¬Ôò»º³åÇø´óÐ¡ >=  len * sizeof(float)
//*              Èôchan = 0Ê±£º Èô¶ÁÈ¡Ô­Ê¼Öµ£¬Ôò»º³åÇø´óÐ¡ >=  ÓÐÐ§×ÜÍ¨µÀÊý * len * sizeof(cyg_adc_sample)
//*              				Èô¶ÁÈ¡×ª»»ºóµÄ¸¡µãÊý£¬Ôò»º³åÇø´óÐ¡ >=  ÓÐÐ§×ÜÍ¨µÀÊý * len * sizeof(float)
//*------------------------------------------------------------------------------------------------
extern int ReadAdcSampling(unsigned char chan, void* buffer, unsigned int *len, unsigned char type, int timesout);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadAdcMinScale
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ADC×îÐ¡²ÉÑù±ÈÀý³ßÊýÖµ
//* Èë¿Ú²ÎÊý:
//*           -- val:  - ADC×îÐ¡Êä³ö±ÈÀý³ßÁ¿£¬¸¡µãÊý(µ¥Î»:ºÁ·ümv)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢valÎªADCµ±Ç°²ÉÑù×îÐ¡±ÈÀý³ßÖµ£¬Ä£ÄâÁ¿£¨µ¥Î»:ºÁ·ümv£©
//*------------------------------------------------------------------------------------------------
extern int ReadAdcMinScale(float *val);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadAdcValidChans
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡Adcµ±Ç°ÓÐÐ§×ÜÍ¨µÀÊý
//* Èë¿Ú²ÎÊý:
//*           -- val:  - ·µ»ØAdcµ±Ç°ÓÐÐ§Í¨µÀÊý
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢      :  1¡¢valÎª°åÔØAdcÓÐÐ§Í¨µÀÊý
//*------------------------------------------------------------------------------------------------
extern int ReadAdcValidChans(int *val);

#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_DAC_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**   DAÉè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************

//Ä¬ÈÏDAÊä³ö¿í¶È(16-bit)
typedef unsigned short cyg_dac_out;

//DA²ÉÑùÖµ±ä»»½á¹¹(float -> cyg_dac_out)
//²»ÔÙÊ¹ÓÃ
struct __float_to_dacout {
	float fvalue;
	cyg_dac_out ivalue;
};

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OpenDac
//* ¹¦ÄÜÃèÊö:  ´ò¿ªDACÉè±¸
//* Èë¿Ú²ÎÊý:
//*           -- dev:        -²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Ö¸¶¨ÎªNULL£»
//*           -- keys:       -²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Ö¸¶¨ÎªNULL
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//*------------------------------------------------------------------------------------------------
extern int OpenDac(char *dev, struct __float_to_dacout *keys);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetDacCfg
//* ¹¦ÄÜÃèÊö:  ÉèÖÃDAC¸÷Í¨µÀ×ª»»²ÎÊý
//* ±¸×¢    :  ²»ÔÙÊ¹ÓÃ£¬Îª±£Ö¤¼æÈÝÐÔ£¬Îª¿Õº¯Êý
//*------------------------------------------------------------------------------------------------
extern int SetDacCfg(int chan, struct __float_to_dacout *keys);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  WriteDacRawVal
//* ¹¦ÄÜÃèÊö:  ÏòDACÊä³öÊý¾Ý£¬Êý¾ÝÎª×ª»»ºóµÄÕûÊýÖµ
//* Èë¿Ú²ÎÊý:
//*           -- chan:    - ÉèÖÃDACÍ¨µÀºÅ
//*                         Èôchan>0, ÔòÖ»Êä³öÄ³µ¥Ò»Í¨µÀ
//*                         Èôchan==0, ÔòËùÓÐÍ¨µÀÊä³ö
//*           -- intval:  - Êä³öÊý×ÖÁ¿
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢intvalÎªÄâÊä³öµÄ16Î»Êý×ÖÁ¿
//*------------------------------------------------------------------------------------------------
extern int WriteDacRawVal(int chan,unsigned short intval);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  WriteDacfloatVal
//* ¹¦ÄÜÃèÊö:  ÏòDACÊä³öÊý¾Ý£¬Êý¾ÝÎªÎ´¾­×ª»»µÄÄ£ÄâÁ¿
//* Èë¿Ú²ÎÊý:
//*           -- chan:    - ÉèÖÃDACÍ¨µÀºÅ
//*                         Èôchan>0, ÔòÖ»Êä³öÄ³µ¥Ò»Í¨µÀ
//*                         Èôchan==0, ÔòËùÓÐÍ¨µÀÊä³ö
//*           -- val:  - Êä³öÄ£ÄâÁ¿£¬¸¡µãÊý
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢valÎªÊä³öµÄÄ£ÄâÁ¿£¨¸¡µãÊý£©
//*------------------------------------------------------------------------------------------------
extern int WriteDacfloatVal(int chan,float val);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadDacRawVal
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡DACµ±Ç°Êä³öÖµ£¬Êý¾ÝÎª×ª»»ºóµÄÕûÊýÖµ
//* Èë¿Ú²ÎÊý:
//*           -- chan:    - ÉèÖÃDACÍ¨µÀºÅ
//*                         Èôchan>0, ÔòÖ¸Ä³µ¥Ò»Í¨µÀ
//*                         Èôchan==0, ´ú±íËùÓÐÍ¨µÀÊä³öÖµ
//*           -- intval:  - µ±Ç°Êä³öÊý×ÖÁ¿
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢intvalÎªÄâÊä³öµÄ16Î»Êý×ÖÁ¿
//*------------------------------------------------------------------------------------------------
extern int ReadDacRawVal(int chan,unsigned short *intval);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadDacfloatVal
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡DACµ±Ç°Êä³öÖµ£¬Êý¾ÝÎª×ª»»ºóµÄ¸¡µãÊýÖµ
//* Èë¿Ú²ÎÊý:
//*           -- chan:    - ÉèÖÃDACÍ¨µÀºÅ
//*                         Èôchan>0, ÔòÖ»Êä³öÄ³µ¥Ò»Í¨µÀ
//*                         Èôchan==0, ÔòËùÓÐÍ¨µÀÊä³ö
//*           -- val:  - µ±Ç°Êä³öÄ£ÄâÁ¿£¬¸¡µãÊý(µ¥Î»:ºÁ·ümv)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢valÎªµ±Ç°Êä³öµÄÄ£ÄâÁ¿£¨µ¥Î»:ºÁ·ümv£©
//*------------------------------------------------------------------------------------------------
extern int ReadDacfloatVal(int chan,float *val);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadDacMinScale
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡DAC×îÐ¡Êä³ö±ÈÀý³ßÊýÖµ
//* Èë¿Ú²ÎÊý:
//*           -- val:  - DAC×îÐ¡Êä³ö±ÈÀý³ßÁ¿£¬¸¡µãÊý(µ¥Î»:ºÁ·ümv)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢valÎªµ±Ç°Êä³öµÄ×îÐ¡Êä³ö±ÈÀýÊýÖµ£¬Ä£ÄâÁ¿£¨µ¥Î»:ºÁ·ümv£©
//*------------------------------------------------------------------------------------------------
extern int ReadDacMinScale(float *val);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadDacValidChans
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡DACµ±Ç°ÓÐÐ§Í¨µÀÊý
//* Èë¿Ú²ÎÊý:
//*           -- val:  - ·µ»ØDACµ±Ç°ÓÐÐ§Í¨µÀÊý
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢      :  1¡¢valÎª°åÔØDACÓÐÐ§Í¨µÀÊý
//*------------------------------------------------------------------------------------------------
extern int ReadDacValidChans(int *val);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  CloseDac
//* ¹¦ÄÜÃèÊö £º¿Õº¯Êý£¬Îª±£Ö¤¼æÈÝÐÔ
//* Èë¿Ú²ÎÊý:  ÎÞ
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int CloseDac(void);
#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_ENGPIO_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**   GPIOÉè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetEnIOOutState
//* ¹¦ÄÜÃèÊö:  ÉèÖÃGPIOÎªµçÆ½Êä³ö£¬²¢ÉèÖÃIOµÄÊä³ö×´Ì¬
//* Èë¿Ú²ÎÊý:
//*           -- gpio:    - IO¶Ë¿ÚºÅ
//*           -- state:   - IOÊä³ö×´Ì¬(0:Low/1:High)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :   ÎÞ
//*------------------------------------------------------------------------------------------------
extern int SetEnIOOutState(int gpio, int state);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetEnIOOutState
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ÎªµçÆ½Êä³öÀàÐÍIOµÄµ±Ç°Êä³ö×´Ì¬
//* Èë¿Ú²ÎÊý:
//*           -- gpio:     - IO¶Ë¿ÚºÅ
//*           -- curstate: - ·µ»ØIOµÄµ±Ç°Êä³ö×´Ì¬(0:Low/1:High)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :   ÎÞ
//*------------------------------------------------------------------------------------------------
extern int GetEnIOOutState(int gpio,int *curstate);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetEnIOPulseState
//* ¹¦ÄÜÃèÊö:  ÉèÖÃGPIOÎªÂö³åÊä³ö£¬²¢Ö¸¶¨Êä³ö²ÎÊý
//* Èë¿Ú²ÎÊý:
//*           -- gpio:       - IO¶Ë¿ÚºÅ
//*           -- hitime10us: - Ö¸¶¨Âö³å¸ßµçÆ½Êä³öÊ±¼ä£¬µ¥Î»10us
//*           -- period10us: - Ö¸¶¨Âö³åÊä³öÖÜÆÚ£¬µ¥Î»£º10us
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢hitime10usºÍperiod10usÖ¸¶¨µ¥Î»¾ùÎª10us
//*           2¡¢ÌØÊâÖµ£ºÈôhitime10us==0£¬ÔòÒ»Ö±Êä³öµÍµçÆ½
//*           3¡¢ÌØÊâÖµ£¬Èôhitime10us!=0,period10us==0,ÔòÖ»Êä³öÒ»´Îhitime10usÊ±¼äµÄ¸ßµçÆ½Âö³å
//*           4¡¢ÌØÊâÖµ£¬Èôhitime10us>=period10us>0,ÔòÒ»Ö±Êä³ö¸ßµçÆ½
//*------------------------------------------------------------------------------------------------
extern int SetEnIOPulseState(int gpio, int hitime10us, int period10us);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetEnIOPulseCfg
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ÎªÂö³åÊä³öÀàÐÍIOµÄµ±Ç°Âö³å²ÎÊý
//* Èë¿Ú²ÎÊý:
//*           -- gpio:       - IO¶Ë¿ÚºÅ
//*           -- hitime10us: - ·µ»ØÂö³åÊä³öµÄ¸ßµçÆ½Êä³öÊ±¼ä²ÎÊý£¬µ¥Î»10us
//*           -- period10us: - ·µ»ØÂö³åÊä³öµÄµ±Ç°Êä³öÖÜÆÚ²ÎÊý£¬µ¥Î»£º10us
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢hitime10usºÍperiod10usÖ¸¶¨µ¥Î»¾ùÎª10us
//*------------------------------------------------------------------------------------------------
extern int GetEnIOPulseCfg(int gpio, int *hitime10us, int *period10us);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetEnIOInValue
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ÎªµçÆ½ÊäÈëÀàÐÍIOµÄµ±Ç°ÊäÈëÖµ
//* Èë¿Ú²ÎÊý:
//*           -- gpio:     - IO¶Ë¿ÚºÅ
//*           -- curval:   - ·µ»ØIOµÄµ±Ç°ÊäÈë×´Ì¬(0:Low/1:High)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :   ÎÞ
//*------------------------------------------------------------------------------------------------
extern int GetEnIOInValue(int gpio,int *curval);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetEnIOEventCfg
//* ¹¦ÄÜÃèÊö:  ÉèÖÃGPIOÎªÊÂ¼þIOÊäÈë£¬²¢Ö¸¶¨ÊÂ¼þ´¥·¢²ÎÊý
//* Èë¿Ú²ÎÊý:
//*           -- gpio:       - IO¶Ë¿ÚºÅ
//*           -- func:     	 - IOÊÂ¼þ»Øµ÷Ö´ÐÐº¯Êý£¬¼´¼ì²âµ½IOÊÂ¼þ·¢Éú£¬¸Ãº¯Êý»á±»´¥·¢Ö´ÐÐ
//*           -- funcmode:   - »Øµ÷º¯ÊýÏìÓ¦Ä£Ê½£¬¼´Í¬Ê±¼ì²âµ½IOÊÂ¼þ·¢Éú£¬»Øµ÷º¯ÊýÖ´ÐÐÄ£Ê½
//*                            0: Ö´ÐÐÒ»´Î£»1£ºÖ´ÐÐ´ÎÊýÓë¼ì²âµ½IOÊÂ¼þÊýÁ¿ÏàÍ¬
//*           -- trigermode: - IOÊÂ¼þÀàÐÍ
//*                            0: HI_to_LOW£»1£ºLOW_to_HI£»2£ºALL
//*           -- filter10us:   - ·À¶¶ÂË²¨²ÎÊý£¬µ¥Î»£º10us,¼´ÓÐÐ§µçÆ½³ÖÐøÊ±¼ä
//*                          ÈôµçÆ½³ÖÐøÊ±¼ä<filter10us,Ôò¼Ù¶¨Îª¸ÉÈÅ,ÊÂ¼þ²»ÏìÓ¦
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢funcÎª»Øµ÷º¯ÊýÖ¸Õë£¬ÈôÎªNULL,ÔòÇå³ý¸ÃÊÂ¼þIO¡£
//*              Òò´Ëµ±ÐèÒª½«ÊÂ¼þIOÖØÖÃÎªÎªÕý³£ÊäÈëIOÊ±£¬ÐèÖ¸¶¨funcÎªNULL²¢µ÷ÓÃSetEventIOCfgº¯Êý
//*           2¡¢func»Øµ÷º¯ÊýµÄ²ÎÊý¼´Îªµ±Ç°gpio
//*------------------------------------------------------------------------------------------------
extern int SetEnIOEventCfg(int gpio,void(*func)(int gpio),int funcmode,int trigermode,int filter10us);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetEnIOEventCfg
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡ÊäÈëÀàÐÍÎªÊÂ¼þIOµÄ²ÎÊý
//* Èë¿Ú²ÎÊý:
//*           -- gpio:       - IO¶Ë¿ÚºÅ
//*           -- trigermode: - ·µ»ØIOÊÂ¼þ¼ì²âÀàÐÍ
//*                          0: HI_to_LOW£»1£ºLOW_to_HI£»2£ºALL
//*           -- filter10us: - ·µ»ØÂö³å·À¶¶²ÎÊý£¬µ¥Î»£º10us
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  Èô·µ»ØERROR_INPARA£¬ÔòËµÃ÷¸Ã¿ÉÄÜÃ»ÓÐÉè¶¨ÎªÊÂ¼þIO£¬»ò²ÎÊý´«Èë´íÎó
//*------------------------------------------------------------------------------------------------
extern int GetEnIOEventCfg(int gpio,int *trigermode,int *filter10us);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_DATETIM_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//**   RTCÊ±¼äÉèÖÃÏà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetRtcTime
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡RTCÊ±¼ä
//* Èë¿Ú²ÎÊý:
//*           -- data:        - ·µ»Øµ±Ç°RTCÊ±¼ä
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢Êý×édataÖÐÊ±¼ä¹æÔòÈçÏÂ£º
//*              data[0] --- year
//*              data[1] --- month
//*              data[2] --- day
//*              data[3] --- hour
//*              data[4] --- minute
//*              data[5] --- second
//*              data[6] --- wday
//*------------------------------------------------------------------------------------------------
extern int GetRtcTime(unsigned short* data);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetRtcTime
//* ¹¦ÄÜÃèÊö:  ÉèÖÃRTCÊ±¼ä
//* Èë¿Ú²ÎÊý:
//*           -- configs:     - ÄâÉè¶¨Ê±¼ä²ÎÊý
//*           -- syn:         - Í¬²½ÏµÍ³Ê±¼ä(0-NO/1-Yes)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢Êý×éconfigsÖÐÊ±¼ä¹æÔòÈçÏÂ£º
//*              configs[0] --- year
//*              configs[1] --- month
//*              configs[2] --- day
//*              configs[3] --- hour
//*              configs[4] --- minute
//*              configs[5] --- second
//*------------------------------------------------------------------------------------------------
extern int SetRtcTime (unsigned short* configs, unsigned char syn);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_TIMER_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*               ¶¨Ê±Æ÷Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************

//¶¨Ê±Æ÷Ö´ÐÐº¯ÊýÖ¸Õë¶¨Òå
typedef void (*FUNC) (int sig);

//¶¨Ê±Æ÷´¥·¢Ä£Ê½
#define	EMTIMER_SINSHOT_MODE 	    _IOWR(EMFUTURE_IOCTL_BASE, 40, int) // µ¥´Î´¥·¢
#define	EMTIMER_PERSHOT_MODE 		_IOWR(EMFUTURE_IOCTL_BASE, 41, int) // ÖÜÆÚÐÔ´¥·¢

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  CreateTimer
//* ¹¦ÄÜÃèÊö:  ´´½¨ÓÃ»§¶¨Ê±Æ÷
//* Èë¿Ú²ÎÊý:
//*           -- handler:    - ¶¨Ê±Æ÷µÄ´¥·¢º¯Êý¾ä±ú
//*           -- args:       - ´«µÝ¸ø´¥·¢º¯ÊýµÄ²ÎÊý
//*           -- usecs:      - ¶¨Ê±Æ÷µÄ¶¨Ê±¼ä¸ô£¬µ¥Î»: useconds(Î¢Ãë)
//*           -- mode :      - ¶¨Ê±Æ÷µÄ´¥·¢Ä£Ê½£º
//*                            EMTIMER_SINSHOT_MODE 	    ¡ª¡ª µ¥´Î´¥·¢
//*                            EMTIMER_PERSHOT_MODE 		¡ª¡ª ÖÜÆÚÐÔ´¥·¢
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int CreateTimer(FUNC handler, void *args, unsigned int usecs, int mode);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  UserusDelayms
//* ¹¦ÄÜÃèÊö:  ÓÃ»§²ãÑÓÊ±º¯Êý£¨Ïà¶Ô¾«È·µÄºÁÃë¼¶ÑÓÊ±£©
//* Èë¿Ú²ÎÊý:
//*           -- millsecs:    - ÑÓÊ±´óÐ¡(µ¥Î»£ººÁÃëms)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int UserusDelayms(unsigned int millsecs);
#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_HIGHTIMER_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*               ¸ß¾«¶È¶¨Ê±Æ÷Ïà¹Ø²¿·Ö
//*               £¨ÐèÒªÓ²¼þÖ§³Ö£©
//*-----------------------------------------------------------
//************************************************************

//¶¨Ê±Æ÷Ö´ÐÐº¯ÊýÖ¸Õë¶¨Òå
typedef void (*HFUNC) (int sig, siginfo_t *extra, void *args);

//¸ß¾«¶È¶¨Ê±Æ÷´¥·¢Ä£Ê½
#define	EMTIMER_HIGHSINSHOT_MODE 	    _IOWR(EMFUTURE_IOCTL_BASE, 42, int) // µ¥´Î´¥·¢
#define	EMTIMER_HIGHSPERSHOT_MODE 		_IOWR(EMFUTURE_IOCTL_BASE, 43, int) // ÖÜÆÚÐÔ´¥·¢

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GreateHighResTimer
//* ¹¦ÄÜÃèÊö:  ´´½¨ÓÃ»§¸ß¾«¶È¶¨Ê±Æ÷
//* Èë¿Ú²ÎÊý:
//*           -- handler:    - ¶¨Ê±Æ÷µÄ´¥·¢º¯Êý¾ä±ú
//*           -- timerid:    - ¸ß¾«¶È¶¨Ê±Æ÷¾ä±úÖ¸Õë£¨¹©É¾³ý¶¨Ê±Æ÷Ê±Ê¹ÓÃ£©
//*           -- nsecs:      - ¶¨Ê±Æ÷µÄ¶¨Ê±¼ä¸ô£¬µ¥Î»: neconds(ÄÉÃë)
//*           -- mode :      - ¶¨Ê±Æ÷µÄ´¥·¢Ä£Ê½£º
//*                            EMTIMER_HIGHSINSHOT_MODE 	  ¡ª¡ª µ¥´Î´¥·¢
//*                            EMTIMER_HIGHSPERSHOT_MODE      ¡ª¡ª ÖÜÆÚÐÔ´¥·¢
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢¸Ãº¯ÊýÐèÒªÆ½Ì¨Ö§³Ö¸ß¾«¶È¶¨Ê±Æ÷¹¦ÄÜ
//*           2¡¢¸ß¾«¶È¶¨Ê±Æ÷Ê¹ÓÃÍêºó£¬ÐèÒªµ÷ÓÃ¶¨Ê±Æ÷É¾³ýº¯Êý£¨²ÎÊýtimerid£©£¬·ñÔò¿ÉÄÜ»áÓ°ÏìÏµÍ³Ð§ÂÊ
//*           3¡¢µ±ÏîÄ¿Ê¹ÓÃ¸ß¾«¶È¶¨Ê±Æ÷ºó£¬ÐèÒªÔÚ±àÒëÁ´½Ó³ÌÐòÊ±Ö¸¶¨¡°-lrt¡±¡£
//*------------------------------------------------------------------------------------------------
extern int GreateHighResTimer(HFUNC handler, timer_t *timerid, unsigned int nsecs,int mode);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  DelHighResTimer
//* ¹¦ÄÜÃèÊö:  É¾³ý¸ß¾«¶È¶¨Ê±Æ÷
//* Èë¿Ú²ÎÊý:
//*           -- timerid:    - ¸ß¾«¶È¶¨Ê±Æ÷¾ä±úÖ¸Õë£¨ÓÉGreateHighResTimer·µ»Ø£©
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢²ÎÊý±ØÐëÊÇGreateHighResTimer·µ»ØµÄ¶¨Ê±Æ÷¾ä±ú
//*------------------------------------------------------------------------------------------------
extern int DelHighResTimer(timer_t timerid);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  UserusDelayus
//* ¹¦ÄÜÃèÊö:  ÓÃ»§²ãÑÓÊ±º¯Êý£¨Ïà¶Ô¾«È·µÄÎ¢Ãë¼¶ÑÓÊ±£©
//* Èë¿Ú²ÎÊý:
//*           -- usecs:    - ÑÓÊ±´óÐ¡(µ¥Î»£ºÎ¢Ãîus)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int UserusDelayus(unsigned int usecs);

#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_PORT_LIB
//************************************************************
//************************************************************
//*
//**   ¶Ë¿ÚÉè±¸Ïà¹Ø²¿·Ö
//*
//************************************************************
//************************************************************

//¶ÁÐ´¿í¶È
typedef enum {
	PORT_WIDTH_8 = 0,
	PORT_WIDTH_16,
	PORT_WIDTH_32
} port_wid_t;

//ºÏ·¨µÄBANK±àºÅ
typedef enum {
	PORT_BANK_2 = 0,
	PORT_BANK_4,
	PORT_BANK_5,
	PORT_BANK_6,
	PORT_BANK_7,
} port_bank_t;

//BANKÊôÐÔ
typedef struct {
	port_bank_t bank;
	port_wid_t width;
	short reserved;
} port_attr_t;

//
typedef struct {
	int regAddr;
	int regData;
} port_data_t;

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OpenPortDev
//* ¹¦ÄÜÃèÊö:  ´ò¿ª¶Ë¿ÚÉè±¸
//* Èë¿Ú²ÎÊý:
//*           -- dev:         - ¶Ë¿ÚÉè±¸Ãû³Æ£¬Èç/dev/port5
//*           -- fd:          - ·µ»ØÎÄ¼þ¾ä±ú
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int OpenPortDev(char *dev,int* fd);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ClosePortDev
//* ¹¦ÄÜÃèÊö:  ¹Ø±Õ¶Ë¿ÚÉè±¸
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int ClosePortDev(int fd);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetPortCfg
//* ¹¦ÄÜÃèÊö:  ÉèÖÃ¶Ë¿ÚÊôÐÔ
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- bank:        - ¶Ë¿ÚµÄBankÅäÖÃ
//*           -- width:       - ¶Ë¿ÚµÄ¶ÁÐ´¿í¶È(8/16/32),ÓëÓ²¼þÆ½Ì¨ÓÐ¹Ø
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  1¡¢¶Ë¿ÚµÄBank¼°¶ÁÐ´widthÓÉÓ²¼þÅäÖÃ¾ö¶¨
//*------------------------------------------------------------------------------------------------
extern int SetPortCfg(int fd, unsigned char bank, unsigned char width);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetPortCfg
//* ¹¦ÄÜÃèÊö:  »ñÈ¡¶Ë¿ÚÊôÐÔ
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- bank:        - ·µ»Ø¶Ë¿ÚµÄBankÅäÖÃ
//*           -- width:       - ·µ»Ø¶Ë¿ÚµÄ¶ÁÐ´¿í¶È(8/16/32)
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int GetPortCfg(int fd, unsigned char *bank, unsigned char *width);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  InPortw
//* ¹¦ÄÜÃèÊö:  16-bit¶Á¶Ë¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- addr:        - ¶ÁÈ¡µÄ¶Ë¿ÚÆ«²îµØÖ·
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»Ø¶ÁÈ¡Öµ£¬·ñÔò·µ»Ø-1
//* ±¸×¢         :  1¡¢addr±ØÐë2×Ö½Ú¶ÔÆë
//*------------------------------------------------------------------------------------------------
extern inline short InPortw(int fd, unsigned long addr);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OutPortw
//* ¹¦ÄÜÃèÊö:  16-bitÐ´¶Ë¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- addr:        - ¶Ë¿ÚµÄÆ«²îµØÖ·
//*           -- data:        - ÊýÖµ
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK£¬·ñÔò·µ»ØERROR_FAIL
//* ±¸×¢         :  1¡¢addr±ØÐë2×Ö½Ú¶ÔÆë
//*------------------------------------------------------------------------------------------------
extern inline int OutPortw(int fd, unsigned long addr, short data);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  InPortl
//* ¹¦ÄÜÃèÊö:  32-bit¶Á¶Ë¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- addr:        - ¶Ë¿ÚµÄÆ«²îµØÖ·
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»Ø¶ÁÈ¡Öµ£¬·ñÔò·µ»Ø-1
//* ±¸×¢         :  1¡¢addr±ØÐë4×Ö½Ú¶ÔÆë
//*------------------------------------------------------------------------------------------------
extern inline int InPortl(int fd, unsigned long addr);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OutPortl
//* ¹¦ÄÜÃèÊö:  32-bitÐ´¶Ë¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- addr:        - ¶Ë¿ÚµÄÆ«²îµØÖ·
//*           -- data:        - ÊýÖµ
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK£¬·ñÔò·µ»ØERROR_FAIL
//* ±¸×¢         :  1¡¢addr±ØÐë4×Ö½Ú¶ÔÆë
//*------------------------------------------------------------------------------------------------
extern inline int OutPortl(int fd, unsigned long addr, int data);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  InPortb
//* ¹¦ÄÜÃèÊö:  8-bit¶Á¶Ë¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- addr:        - ¶ÁÈ¡µÄ¶Ë¿ÚÆ«²îµØÖ·
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»Ø¶ÁÈ¡Öµ£¬·ñÔò·µ»Ø-1
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern inline char InPortb(int fd, unsigned long addr);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OutPortb
//* ¹¦ÄÜÃèÊö:  8-bitÐ´¶Ë¿Ú
//* Èë¿Ú²ÎÊý:
//*           -- fd:          - ÎÄ¼þ¾ä±ú(¼´OpenPortDev()µÄ·µ»ØÖµ)
//*           -- addr:        - ¶Ë¿ÚµÄÆ«²îµØÖ·
//*           -- data:        - ÊýÖµ
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK£¬·ñÔò·µ»ØERROR_FAIL
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern inline int OutPortb(int fd, unsigned long addr, char data);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_WDT_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*                ¿´ÃÅ¹·Éè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  EnableWtd
//* ¹¦ÄÜÃèÊö:  Ê¹ÄÜ¿´ÃÅ¹·Éè±¸
//* Èë¿Ú²ÎÊý:
//*           -- TimeOut:     - Ö¸¶¨¿´ÃÅ¹·µÄ×î´óÎ¹¹·¼ä¸ô£¨Ãë£©
//*
//* ·µ»ØÖµ£º      ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:     ÎÞ
//*------------------------------------------------------------------------------------------------
extern int EnableWtd(int TimeOut);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  FeedWtd
//* ¹¦ÄÜÃèÊö:  Î¹¹·²Ù×÷
//* Èë¿Ú²ÎÊý:  ÎÞ
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int FeedWtd(void);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  DisableWtd
//* ¹¦ÄÜÃèÊö:  ¹Ø±Õ¿´ÃÅ¹·
//* Èë¿Ú²ÎÊý:  ÎÞ
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int DisableWtd(void);
#ifdef __cplusplus
}
#endif
#endif

#ifdef CFG_KBD_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*                ¼üÅÌÉè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************

//Çø·Öµ¯Æð/°´ÏÂµÄãÐÖµ£¨ÐèÓëÇý¶¯¹ØÁª£©
#define UP_THRESHOLD_DOWN	(0x80)

//
#define DOWN_KEY			(0x0)
#define UP_KEY				(0x1)

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  OpenKbd
//* ¹¦ÄÜÃèÊö:  ´ò¿ª¼üÅÌÉè±¸
//* Èë¿Ú²ÎÊý:
//*           -- dev:     - ¼üÅÌÉè±¸ÎÄ¼þ£¬Èç/dev/kbd0
//*           -- fd:      - ·µ»ØÎÄ¼þ¾ä±ú
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int OpenKbd(char *dev,int *fd);


//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  FlushKbd
//* ¹¦ÄÜÃèÊö:  Çå¿Õ¼üÅÌ»º³åÇø
//* Èë¿Ú²ÎÊý:
//*           -- fd:      - ÎÄ¼þ¾ä±ú
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int FlushKbd(int fd);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GetKeyValue
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡¼üÅÌÖµ¼°¶ÔÓ¦×´Ì¬
//* Èë¿Ú²ÎÊý:
//*           -- fd:         - ÎÄ¼þ¾ä±ú
//*           -- keyValue:   - ·µ»Ø°´¼üÖµ
//*           -- status:     - ·µ»Ø°´¼üµÄµ±Ç°²Ù×÷×´Ì¬(1£ºUP/0£ºDOWN)
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int GetKeyValue(int fd, unsigned short *keyValue, unsigned char *status);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  CloseKbd
//* ¹¦ÄÜÃèÊö:  ¹Ø±Õ¼üÅÌÉè±¸£¬ÊÍ·ÅÏà¹Ø×ÊÔ´
//* Èë¿Ú²ÎÊý:
//*           -- fd:         - ÎÄ¼þ¾ä±ú£¬ÓÉOpenKbd·µ»Ø
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int CloseKbd(int fd);
#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_MISCS_LIB
//************************************************************
//*-----------------------------------------------------------
//*
//*               ID¼ÓÃÜÐ¾Æ¬Éè±¸Ïà¹Ø²¿·Ö
//*						ÐèÓ²¼þÖ§³Ö
//*
//*-----------------------------------------------------------
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadSerialID
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡´®ÐÐID(DI)Éè±¸µÄÎ¨Ò»IDºÅ
//* Èë¿Ú²ÎÊý:
//*           -- id:        - ·µ»ØIDÊý¾ÝÊý×é
//*           -- len:       - IDÊý×éµÄ³¤¶È
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int ReadSerialID(unsigned char* id, unsigned char* len);
#ifdef __cplusplus
}
#endif

//************************************************************
//*-----------------------------------------------------------
//*
//*               ÎÂÊª¶È´«¸ÐÉè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************
/******   TM control cmd   *********/
#define	EM_SET_SENSOR_RES 	    _IOWR(EMFUTURE_IOCTL_BASE, 30, int) // ÉèÖÃ²ÉÑù·Ö±æÂÊ
#define	EM_GET_SENSOR_RES 		_IOWR(EMFUTURE_IOCTL_BASE, 31, int) //  ¶ÁÈ¡µ±Ç°²ÉÑù·Ö±æÂÊ

#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetSensorCfg
//* ¹¦ÄÜÃèÊö:  ÅäÖÃÎÂ¶È´«¸ÐÆ÷µÄ¹¤×÷²ÎÊý
//* Èë¿Ú²ÎÊý:
//*           -- cmd:        - ÓÃ»§ÃüÁîÂë
//*                            EM_SET_SENSOR_RES 	    ¡ª¡ª ÉèÖÃ²ÉÑù·Ö±æÂÊ
//*                            EM_GET_SENSOR_RES 		¡ª¡ª ¶ÁÈ¡µ±Ç°²ÉÑù·Ö±æÂÊ
//*           -- arg:        - ´«Èë/´«³öÉèÖÃ²ÎÊý
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢         :  ÎÞ
//*------------------------------------------------------------------------------------------------
extern int SetSensorCfg(int cmd, unsigned short* arg);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  ReadSensorVal
//* ¹¦ÄÜÃèÊö:  ¶ÁÈ¡´«¸ÐÆ÷µÄÎÂ¶ÈÖµ
//* Èë¿Ú²ÎÊý:
//*           -- value:        - ·µ»ØÎÂ¶ÈÖµ
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢      :  ¶ÁÈ¡Öµ=Êµ¼ÊÖµ*·Å´ó±¶Êý(10000)
//*------------------------------------------------------------------------------------------------
extern int ReadSensorVal(unsigned int* value);

#ifdef __cplusplus
}
#endif

//************************************************************
//*-----------------------------------------------------------
//*
//**   ·äÃùÆ÷ºÍÔËÐÐLEDÉè±¸Ïà¹Ø²¿·Ö
//*
//*-----------------------------------------------------------
//************************************************************

#ifdef __cplusplus
extern "C" {
#endif

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  IoctlBuzzer
//* ¹¦ÄÜÃèÊö:  ²Ù×÷·äÃùÆ÷Éè±¸
//* Èë¿Ú²ÎÊý:
//*           -- ulaston10ms:   - Ãù½Ð³ÖÐøÊ±¼ä£¬µ¥Î»10ºÁÃë¡£
//*           -- ulastoff10ms:  - ÏìÁå¹Ø±ÕÊ±¼ä£¬µ¥Î»10ºÁÃë¡£
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:  	  1¡¢ulaston10msÓëulastoff10msµÄµ¥Î»¾ùÎª10ºÁÃë£»
//*           2¡¢ulaston10msÓëulastoff10msµÄÈ¡Öµ¹æÔò£º
//*			     Èôulaston10ms = 0 ----  ¹Ø±Õ·äÃùÆ÷
//*				 Èôulaston10ms = 0xFFFF ---- ·äÃùÆ÷³ÖÐøÃù½Ð
//*				 Èôulaston10ms >0 && ulastoff10ms=0  ------ ·äÃùÆ÷Ãù½Ð1´Î£¬³ÖÐøÊ±¼äulaston10ms
//*				 Èôulaston10ms >0 && ulastoff10ms>0  ------ ·äÃùÆ÷ÖÜÆÚÐÔÃù½Ð
//*				 											Ãù½ÐÊ±¼äulaston10ms£¬ÖÜÆÚ(ulaston10ms+ulastoff10ms)
//*------------------------------------------------------------------------------------------------
extern int IoctlBuzzer(unsigned short ulaston10ms, unsigned short ulastoff10ms);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  IoctlRunLed
//* ¹¦ÄÜÃèÊö:  ²Ù×÷ÔËÐÐÖ¸Ê¾µÆ(Running LED)
//* Èë¿Ú²ÎÊý:
//*           -- ulaston10ms:   - ÔËÐÐÖ¸Ê¾µÆÁÁ³ÖÐøÊ±¼ä£¬µ¥Î»10ºÁÃë¡£
//*           -- ulastoff10ms:  - ÔËÐÐÖ¸Ê¾µÆ¹Ø±ÕÊ±¼ä£¬µ¥Î»10ºÁÃë¡£
//*
//* ·µ»ØÖµ    :  ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:  	  1¡¢ulaston10msÓëulastoff10msµÄµ¥Î»¾ùÎª10ºÁÃë£»
//*           2¡¢ulaston10msÓëulastoff10msµÄÈ¡Öµ¹æÔò£º
//*			     Èôulaston10ms = 0 ----  ¹Ø±ÕÔËÐÐÖ¸Ê¾µÆ
//*				 Èôulaston10ms = 0xFFFF ---- ÔËÐÐÖ¸Ê¾µÆ³ÖÐøÁÁ
//*				 Èôulaston10ms >0 && ulastoff10ms=0  ------ ÔËÐÐÖ¸Ê¾µÆÉÁË¸1´Î£¬³ÖÐøÊ±¼äulaston10ms
//*				 Èôulaston10ms >0 && ulastoff10ms>0  ------ ÔËÐÐÖ¸Ê¾µÆÖÜÆÚÐÔÉÁË¸
//*				 											ÁÁ³ÖÐøÊ±¼äulaston10ms£¬ÖÜÆÚ(ulaston10ms+ulastoff10ms)
//*------------------------------------------------------------------------------------------------
extern int IoctlRunLed(unsigned short ulaston10ms, unsigned short ulastoff10ms);

#ifdef __cplusplus
}
#endif
#endif


#ifdef CFG_GPIO_LIB
//************************************************************
//************************************************************
//*
//**   ÓÃ»§²ãµÄGPIO²Ù×÷º¯Êý
//*
//************************************************************
//************************************************************


//GPIO¶Ë¿Ú¶¨Òå
#define PORTA_GPIO(x)	x
#define PORT0_GPIO(x)	PORTA_GPIO(x)
#define PORTB_GPIO(x)	(32+x)
#define PORT1_GPIO(x)	PORTB_GPIO(x)
#define PORTC_GPIO(x)	(64+x)
#define PORT2_GPIO(x)	PORTC_GPIO(x)
#define PORTD_GPIO(x)	(96+x)
#define PORT3_GPIO(x)	PORTD_GPIO(x)
#define PORTE_GPIO(x)	(128+x)
#define PORT4_GPIO(x)	PORTE_GPIO(x)
#define PORTF_GPIO(x)	(160+x)
#define PORT5_GPIO(x)	PORTF_GPIO(x)

//¶Ë¿ÚÀàÐÍ
typedef enum {
	PIO_MODE_OUT = 0,
	PIO_MODE_IN,
} gpio_mode_t;


#ifdef __cplusplus
extern "C" {
#endif
//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  SetGPIOCfg
//* ¹¦ÄÜÃèÊö:  ÉèÖÃGPIOµÄ¶Ë¿ÚÊôÐÔ
//* Èë¿Ú²ÎÊý:
//*           -- gpio:         - gpio¶Ë¿Ú
//*           -- attr:         - Ö¸¶¨GPIOµÄÊôÐÔ£¨Ä£Ê½£©
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int SetGPIOCfg(int gpio, gpio_mode_t attr);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GPIOOutValue
//* ¹¦ÄÜÃèÊö:  GPIO¶Ë¿ÚÊä³ö
//* Èë¿Ú²ÎÊý:
//*           -- gpio:         - gpio¶Ë¿Ú
//*           -- val:          - Ö¸¶¨Êä³ö×´Ì¬
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int GPIOOutValue(int gpio, int val);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  GPIOInValue
//* ¹¦ÄÜÃèÊö:  ¶ÁGPIO¶Ë¿ÚÊäÈë
//* Èë¿Ú²ÎÊý:
//*           -- gpio:         - gpio¶Ë¿Ú
//*           -- state:        - ·µ»ØGPIOÊäÈë×´Ì¬
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int GPIOInValue(int gpio,int *state);

//*------------------------------------------------------------------------------------------------
//* º¯ÊýÃû³Æ:  FreeGPIOCfg
//* ¹¦ÄÜÃèÊö:  ÊÍ·ÅGPIO¿ØÖÆ
//* Èë¿Ú²ÎÊý:
//*           -- gpio:         - gpio¶Ë¿Ú
//*
//* ·µ»ØÖµ£º        ³É¹¦·µ»ØERROR_OK(==0)£»Ê§°Ü·µ»Ø(ERROR_INPARA/ERROR_TIMEOUT/ERROR_SYS/ERROR_NOSUPPORT/ERROR_FAIL)
//* ±¸×¢:      ÎÞ
//*------------------------------------------------------------------------------------------------
extern int FreeGPIOCfg(int gpio);

#ifdef __cplusplus
}
#endif
#endif

//************************************************************
//************************************************************
//*
//**   ÆäËû¸¨Öúº¯Êý
//*
//************************************************************
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#if defined(CPU_AT91SAM9263) || defined(CPU_AT91SAM9260) || defined(CPU_AT91SAM9G20) \
		|| defined(CPU_AT91SAM9G45) || defined(CPU_AT91SAM9X5) || defined(CPU_TIAM335X)

extern inline unsigned short _inw(unsigned long addr);
extern inline void _outw(unsigned long addr, unsigned short dat);
extern inline unsigned long _inl(unsigned long addr);
extern inline void _outl(unsigned long addr, unsigned long dat);
extern inline unsigned char _inb(unsigned long addr);
extern inline void _outb(unsigned long addr, unsigned char dat);

#endif

extern void _nops(unsigned long nums);
extern int _Readn(int fd, void *buf, int n);
extern int _Writen(int fd, const void *buf, int n);

#ifdef __cplusplus
}
#endif

#define DEBUG
#ifdef DEBUG
#define TRACE(str, args...)	printf(str, ## args)
#else
#define TRACE(str, args...)
#endif

#endif /* LIBS_EMFUTURE_ODM_H_ */
