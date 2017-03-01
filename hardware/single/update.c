/******************************************************************
** 文件名:	update.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.11
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "update.h"
#include "serial.h"
#include "Interface.h"

static int update_slave(struct update_t *this)
{
	assert_param(this,FAIL);

	this->update_send_package(this);
	while(this->info.update_timeout){
		switch(this->update_recv_onebyte(this)){
			case ACK:
				this->info.update_repeat = 5;
				this->info.update_timeout = 5;
				if(++this->info.update_cframe >= this->info.update_frame){
					this->info.update_ok = 1;
					debug(DEBUG_update,"Done!\n"\
						"this->info.update_cframe=%d\n",this->info.update_cframe);
					return SUCCESS;
				}
				printf("#");fflush(stdout);
				this->update_send_package(this);
				break;
			case NAK:
				this->update_send_package(this);
				--this->info.update_repeat;
				break;
			default:
				debug(DEBUG_update,"Timeout Send package agin!\n");
				if(--this->info.update_timeout)
					this->update_send_package(this);
				break;
		}
		if(this->info.update_ok)  return SUCCESS;
		if(this->info.update_repeat <= 0){
			this->info.update_pend = !0;
			return Update_Pend;
		}
	}
	return TIME_OUT;
}

static int update_start(struct update_t *this,int addr,char Image)
{
	assert_param(this,FAIL);

	char update_file[32] = {0};
	this->info.update_isCoordi = ((0xffff&addr) == 0)?!0:0;
	this->package.Coor_addr = 0xff&(addr>>16);
	this->package.single_addr[0] = 0xff&(addr>>8);
	this->package.single_addr[1] = 0xff&addr;
	debug(DEBUG_update,"update:Coordi_addr=0x%02x,Single_addr=0x%02x%02x.\n",\
		this->package.Coor_addr,this->package.single_addr[0],this->package.single_addr[1]);
	switch(Image){
		case 'a': case 'A':
		case 'b': case 'B':
			Image = toupper(Image);  break;
		default:
			Image = 'N'; break;
	}
	sprintf(update_file,"./update/%s_%c.bin",this->info.update_isCoordi ?"Coordinate":"Single",Image);
	debug(DEBUG_update,"update: \"%s\"\n",update_file);
	if(access(update_file,F_OK)){
		err_Print(DEBUG_update,"Update file not exist!\n");
		return FAIL;
	}
	this->info.update_fd = open(update_file,O_RDONLY);
	if(this->info.update_fd < 0){
		err_Print(DEBUG_update,"Open update file failed!\n");
		return Open_FAIL;
	}
	struct stat   update_stat;
	if(-1 == fstat(this->info.update_fd, &update_stat)){
		err_Print(DEBUG_update,"Get update info Fail!\n");
		return FAIL;
	}
	this->info.update_size = update_stat.st_size;
	this->info.update_frame = (this->info.update_size+UPACKSIZE-1)/UPACKSIZE;
	this->info.update_isImageA = (Image=='A')?1:0;
	this->info.update_cframe = 0;
	this->info.update_repeat = 5;
	this->info.update_timeout = 5;
	debug(DEBUG_update,"update size:%d.\nIn total %d frame.\n"\
		"Start Update Coordinate 0r Single...\n",this->info.update_size,this->info.update_frame);
	return update_slave(this);
}

static u8 update_recv_onebyte(struct update_t *this)
{
	assert_param(this,FAIL);
	u8 res = 0;
	serial_t *p_serial = this->subclass->topuser->Serial;
	if(!p_serial) return FAIL;
	return SUCCESS == p_serial->serial_recv(\
		p_serial,CFG_COM485,(s8*)&res,1,2000000)?res:FAIL;
}

static int update_send_package(struct update_t *this)
{
	assert_param(this,FAIL);

	int readsize = 0;
	appitf_t *topuser = this->subclass->topuser;
	if(!topuser || !topuser->Serial) {
		debug(DEBUG_update,"Serial API is undefine!\n");
		return FAIL;
	}
	this->package.header = (this->info.update_cframe >= this->info.update_frame-1)?EOT:SOH;
	this->package.frame[0] = this->info.update_cframe;
	this->package.frame[1] = ~this->info.update_cframe;
	if(lseek(this->info.update_fd,UPACKSIZE*this->info.update_cframe,SEEK_SET) < 0){
		err_Print(DEBUG_update,"lseek error!\n");
		return FAIL;
	}
	readsize = read(this->info.update_fd, this->package.data,UPACKSIZE);
	if(readsize < 0){
		err_Print(DEBUG_update,"Read update file error!\n");
		return FAIL;
	}else if(readsize < UPACKSIZE){
		//debug(DEBUG_update,"Fill databuf size %d\n",UPACKSIZE-readsize);
		memset(this->package.data+readsize,CTRLZ,UPACKSIZE-readsize);
	}
	crc_hight(this->package.CRC16,this->package.data,UPACKSIZE);
	//this->subclass->Display("\npackage:",&this->package,sizeof(package_t));
	return topuser->Serial->serial_send(topuser->Serial,\
		CFG_COM485,(s8*)&this->package,sizeof(package_t),2000000);
}

static void update_release(struct update_t *this)
{
	assert_param(this,;);
	FREE(this);
}

update_t *update_init(struct Single_t *subclass)
{
	update_t *update = malloc(sizeof(update_t));
	if(!update)  return NULL;
	bzero(update,sizeof(update_t));

	update->subclass = subclass;
	update->update_start = update_start;
	update->update_release = update_release;
	update->update_send_package = update_send_package;
	update->update_recv_onebyte = update_recv_onebyte;
	update->update_slave = update_slave;

	return update;
}


