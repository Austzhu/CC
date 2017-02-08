# Makefile for ztcc or lamp
# Copyright (C) 2016-2020
# Author: Austzhu
# Mail: 153462902@.com
# For conditions of distribution and use, see copyright notice
#
CFLAGS =  -Wall -O2
LINKFLAGS = -lpthread
DFLAGS = -std=gnu99
OBJS :=
ROOTDIR=${PWD}
output := output

OBJS_PATH := ${ROOTDIR}/Common/common
OBJS_PATH +=${ROOTDIR}/Common/crc16
OBJS_PATH +=${ROOTDIR}/User
OBJS_PATH +=${ROOTDIR}/task/taskque
OBJS_PATH +=${ROOTDIR}/task/timertask
OBJS_PATH +=${ROOTDIR}/sqlite
OBJS_PATH +=${ROOTDIR}/hardware/ethernet
OBJS_PATH +=${ROOTDIR}/hardware/serial
OBJS_PATH +=${ROOTDIR}/hardware/single
OBJS_PATH +=${ROOTDIR}/hardware/Meter
OBJS_PATH +=${ROOTDIR}/Software/initstart
OBJS_PATH +=${ROOTDIR}/Software/Warn
OBJS_PATH +=${ROOTDIR}/Software/ztcc
OBJS_PATH +=${ROOTDIR}/Software/operate
OBJS_PATH +=${ROOTDIR}/Software/auto_control
OBJS_PATH +=${ROOTDIR}/Software/process
OBJS_PATH +=${ROOTDIR}/Software/kalman

CMD := $(shell find ${ROOTDIR}/ -type d | egrep -v '.git|output|library|tools')
IFLAGS = $(addprefix -I,${CMD})

ifeq (${Config_bord},e6018)
CC := arm2012-linux-gcc
LDFLAGS = -L$(PWD)/library/e6018 -L$(PWD)/library/sql_armv7  -lsqlite3 -lEM_Middleware_Lib
IFLAGS += -I$(PWD)/library/e6018
target := zts210
else ifeq (${Config_bord},e3100)
CC = armv5-linux-gcc
LDFLAGS = -L$(PWD)/library/e3100 -L$(PWD)/library/sql_armv5  -lsqlite3  -lEM_Middleware_Lib -ldl -lrt
IFLAGS += -I$(PWD)/library/e3100
target := lamp
endif

CC ?= gcc
target ?= none

export output CFLAGS ROOTDIR CC  DFLAGS IFLAGS


#包含子目录的工程文件
#INCLUDE := $(addsuffix /Makefile, $(OBJS_PATH))
#-include $(INCLUDE)
#DESTOBJS := $(addprefix ./$(output)/, $(notdir $(OBJS)))
DESTOBJS := ${ROOTDIR}/output/*.o


all: autoconf.mk Version ${target} Automountusb  Watchrsh

${target}: compile
	$(CC) $(CFLAGS) $(DFLAGS) $(LINKFLAGS) $(LDFLAGS)  -o ${ROOTDIR}/Applications/$@  $(DESTOBJS)

Version:
	@`./version/setlocalversion`

Automountusb:
	@$(CC) -o  ./Applications/$@  ./tools/AutoMountUsb.c
Watchrsh:
	@$(CC) -o ./Applications/$@ ./tools/Watch_ssh.c
autoconf.mk: ${ROOTDIR}/config/config.h
	@${CC} -E -dM $< | grep "Config_" | sed -n -f ${ROOTDIR}/tools/define2mk >$@

#编译子目录的c文件
compile:
	@$(foreach N,$(OBJS_PATH),make --no-print-directory -C  $(N);)

#将可程序拷贝到nfs目录下
install:
	@mkdir -p $(PRJROOT)/nfs/${target}/config $(PRJROOT)/nfs/${target}/update \
			$(PRJROOT)/nfs/${target}/Logfile $(PRJROOT)/nfs/${target}/tools
	@cp -frd ./Applications/${target} $(PRJROOT)/nfs/${target}
	@cp -frd ./Applications/Automountusb  ./Applications/Watchrsh  $(PRJROOT)/nfs/${target}/tools
	@cp -frd ./config/Create_Database.sh $(PRJROOT)/nfs/${target}/config
	@cp -frd ./config/fileparam.ini $(PRJROOT)/nfs/${target}/config

.PHONY: clean
clean:
	@find -type f \( -name '*.o' \) -print | xargs rm -f
distclean:clean
	@rm ./Applications/* -f



