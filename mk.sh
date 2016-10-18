#########################################################################
# File Name: mk.sh
# Author: Austzhu
# mail: 153462902@.com
# Created Time: 2016年03月29日 星期二 14时46分19秒
#########################################################################
#!/bin/bash

Topdir=$PWD
DirCom=$Topdir/Common
DirHard=$Topdir/hardware
DirSoft=$Topdir/Software

#增加c语言的头文件搜索路径
CPATH=$Topdir/Common:$Topdir/Common/My_lib:$Topdir/Common/common:$Topdir/config:$DirHard/coordinate:\
$DirHard/ethernet:$DirHard/gprs_ppp:$DirHard/serial:$DirSoft/analysis:$DirSoft/burglar_alarm:$DirSoft/cc_param:\
$DirSoft/cmd_process:$DirSoft/communication:$DirSoft/info_output:$DirSoft/initstart:$DirSoft/link_method:$DirHard/Zt_Meter:\
$DirSoft/Log:$DirSoft/meter_crc:$Topdir/sqlite:$Topdir/task/tasklist:$Topdir/task/taskque:$Topdir/task/timertask:$DirHard/update:\
$Topdir/User:$Topdir/version:$CPATH

export CPATH

#env | grep 'PATH'

if [ ! -f ./sqlite/lib/libsqlite3.so ];then
	ln -s libsqlite3.so.0.8.6 ./sqlite/lib/libsqlite3.so && echo "Create soft link sqlite3.so suceess!"
fi
if [ ! -f ./sqlite/lib/libsqlite3.so.0 ];then
	ln -s libsqlite3.so.0.8.6 ./sqlite/lib/libsqlite3.so.0 && echo "Create soft link sqlite3.so.0 suceess!"
fi

make distclean
make && make install && echo "******make install success!******"
