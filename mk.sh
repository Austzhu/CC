#########################################################################
# File Name: mk.sh
# Author: Austzhu
# mail: 153462902@.com
# Created Time: 2016年03月29日 星期二 14时46分19秒
#########################################################################
#!/bin/bash

if [ $# -lt 1 ];then
	echo "use $0 e6180 || $0 e3100"
	exit 0
else
	Config_bord=$1
fi

CURDIR=${PWD}
COMDIR=${CURDIR}/Common
HARDIR=${CURDIR}/hardware
SOFTDIR=${CURDIR}/Software

STAGING_DIR=/
#增加c语言的头文件搜索路径
CPATH=${COMDIR}:${COMDIR}/common:${COMDIR}/crc16
CPATH=${CPATH}:${CURDIR}/config:${CURDIR}/sqlite:${CURDIR}/task/taskque:${CURDIR}/version
CPATH=${CPATH}:${HARDIR}/ethernet:${HARDIR}/serial:${HARDIR}/single:${HARDIR}/Meter
CPATH=${CPATH}:${SOFTDIR}/process:${SOFTDIR}/initstart:${SOFTDIR}/Log:${SOFTDIR}/Warn
CPATH=${CPATH}:${SOFTDIR}/ztcc:${SOFTDIR}/lamp:${SOFTDIR}/operate:${SOFTDIR}/auto_control
CPATH=${CPATH}:${SOFTDIR}/kalman

case ${Config_bord} in
	e6018)
		CPATH=${CURDIR}/library/e6018:${CURDIR}/library/sql_armv7:${CPATH}
		;;
	e3100)
		CPATH=${CURDIR}/library/e3100:${CURDIR}/library/sql_armv5:${CPATH}
		;;
	*)
		echo "error:\"Config_bord=${Config_bord} not define\""
		exit -1
		;;
esac

export CPATH Config_bord STAGING_DIR
#env | grep 'CPATH' ; echo "" ; echo ""
echo "*****board is ${Config_bord}*****"
make distclean
make && make install && echo "******make install success!******"
