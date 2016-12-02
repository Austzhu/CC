# Makefile for ztcc or lamp
# Copyright (C) 2016-2020
# Author: Austzhu
# Mail: 153462902@.com
# For conditions of distribution and use, see copyright notice

CFLAGS =  -Wall -O2
#CFLAGS += -Wno-implicit-function-declaration
#关闭由-O2优化选项带来的"警告"提示
#CFLAGS += -Wno-strict-aliasing
#关闭数据包data数组越界的"警告"提示
#CFLAGS += -Wno-array-bounds
CFLAGS += -std=gnu99

#OBJS_PATH := $(shell find -maxdepth 1 -type d -name '*' | grep '[a-z]')
OBJS_PATH +=./Common/common ./hardware/ethernet ./Software/initstart \
./task/taskque  ./User  ./Software/process ./hardware/serial ./sqlite ./hardware/single \
./hardware/Meter ./Software/Warn ./Software/ztcc ./Software/lamp ./Software/operate

ifeq (${Config_bord},e6018)
CC := arm2012-linux-gcc
LDFLAGS = -L$(PWD)/library/e6018 -L$(PWD)/library/sql_armv7  -lsqlite3 -lEM_Middleware_Lib
target := ztcc
else ifeq (${Config_bord},e3100)
CC = armv5-linux-gcc
LDFLAGS = -L$(PWD)/library/e3100 -L$(PWD)/library/sql_armv5  -lsqlite3  -lEM_Middleware_Lib -ldl -lrt
target := lamp
endif

CC ?= gcc
target ?= none

LINKFLAGS = -lpthread
DFLAGS =

OBJS :=
output := output
Rootdir=$(PWD)
export output CFLAGS Rootdir



#包含子目录的工程文件
INCLUDE := $(addsuffix /Makefile, $(OBJS_PATH))
include $(INCLUDE)

DESTOBJS := $(addprefix ./$(output)/, $(notdir $(OBJS)))

all: Version ${target} Automountusb  Watchrsh

${target}: compile
	@$(CC)  $(DESTOBJS) -o ./Applications/$@ $(LINKFLAGS) $(LDFLAGS)
Version:
	@`./version/setlocalversion`

Automountusb:
	@$(CC) -o  ./Applications/$@  ./tools/AutoMountUsb.c
Watchrsh:
	@$(CC) -o ./Applications/$@ ./tools/Watch_ssh.c


#编译子目录的c文件
compile:
	@$(foreach N,$(OBJS_PATH),make -C $(N) CC=$(CC);)

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
	@rm ./bin/test_App -f

