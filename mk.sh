#########################################################################
# File Name: mk.sh
# Author: Austzhu
# mail: 153462902@.com
# Created Time: 2016年03月29日 星期二 14时46分19秒
#########################################################################
#!/bin/bash

if [ $# -lt 1 ];then
	echo "use ./mk.sh e6180 || ./mk.sh e3100"
	exit 0
fi
Config_bord=$1
Topdir=$PWD
DirCom=$Topdir/Common
DirHard=$Topdir/hardware
DirSoft=$Topdir/Software

STAGING_DIR=/
#增加c语言的头文件搜索路径
CPATH=$Topdir/Common:$Topdir/Common/common:$Topdir/config:$DirHard/ethernet:\
$DirSoft/process:$DirSoft/initstart:$DirSoft/Log:$Topdir/sqlite:$Topdir/task/taskque:\
$Topdir/User:$Topdir/version:$DirHard/serial:$DirHard/single:$DirHard/Meter:$DirSoft/Warn:\
$DirSoft/ztcc:$DirSoft/lamp:$CPATH

create_lib(){
	if [ ! -f $1/libsqlite3.so ];then
		ln -s libsqlite3.so.0.8.6 $1/libsqlite3.so && echo "Create soft link sqlite3.so suceess!"
	fi
	if [ ! -f $1/libsqlite3.so.0 ];then
		ln -s libsqlite3.so.0.8.6 $1/libsqlite3.so.0 && echo "Create soft link sqlite3.so.0 suceess!"
	fi
}

if [ ${Config_bord} == e6018 ] ; then
	echo "*****board is e6018*****"
	CPATH=${Topdir}/library/e6018:${Topdir}/library/sql_armv7:$CPATH
	create_lib ${Topdir}/library/sql_armv7
elif [ ${Config_bord} == e3100 ] ; then
	echo "*****board is e3100*****"
	CPATH=${Topdir}/library/e3100:${Topdir}/library/sql_armv5:$CPATH
	create_lib ${Topdir}/library/sql_armv5
fi

export CPATH Config_bord STAGING_DIR

#env | grep 'CPATH'

make distclean
make && make install && echo "******make install success!******"
