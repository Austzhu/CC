#!/bin/sh

DB=cc_corl.db
TB_COOR=db_coordinator
TB_SINGLE=db_light
TB_TASK=db_task
TB_TASKLIST=db_tasklist
TB_WARN=db_warn
TB_INFO=db_info_light
TB_TUNNEL=db_tunnel_info
TB_PWM=db_index_pwm
TB_GRPINFO=db_group_info

check_errno(){
	if [ $1 -eq 1 ] ; then
		echo "create table" $2 "error!"
		exit 0
	fi
}

#创建协调器记录表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_COOR} ;
create table ${TB_COOR}(
	id 			integer,
	Wl_Addr 	integer NOT NULL,
	Base_Addr 	integer NOT NULL,
	Coor_gid 	integer NOT NULL,
	CC_id		text 	NOT NULL,
	Map_Addr 	integer NOT NULL,
	Warn_flags	integer default 0,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	CHECK (Base_Addr < 0xff AND Base_Addr > 0x20)
);
EOF
check_errno $?  ${TB_COOR}

#建立单灯记录表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_SINGLE} ;
create table ${TB_SINGLE}(
	id 			integer,
	Wl_Addr 	integer NOT NULL,
	Base_Addr 	integer NOT NULL,
	lt_gid 		integer NOT NULL,
	Coor_id		integer NOT NULL,
	Map_Addr	integer NOT NULL,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	FOREIGN KEY(Coor_id) REFERENCES ${TB_COOR}(Base_Addr) ON DELETE CASCADE ON UPDATE CASCADE
);
EOF
check_errno $?  ${TB_SINGLE}
#单灯信息表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_INFO} ;
create table ${TB_INFO}(
	id 				integer,
	Base_Addr 		integer ,
	operate_flags 	integer default 0,
	Warn_flags		integer default 0,
	Rate_v 			integer default 230,
	Rate_p 			integer default 0,
	Rate_PF 		integer default 0,
	light_status 	integer default 0,
	light_val  		integer default 0,
	light_E 		integer default 0,
	light_P 		integer default 0,
	light_V 		integer default 0,
	light_D 		integer default 0,
	rtime 			integer default 0,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	FOREIGN KEY(Base_Addr) REFERENCES ${TB_SINGLE}(Base_Addr) ON DELETE CASCADE ON UPDATE CASCADE
);
EOF
check_errno $?  ${TB_INFO}
#建立任务表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_TASK} ;
create table ${TB_TASK}(
	id			integer,
	Name		text NOT NULL,
	Priority	integer NOT NULL,
	Start_Date	integer NOT NULL,
	End_Date	integer NOT NULL,
	Run_Time	integer NOT NULL,
	Inter_Time	integer NOT NULL,
	Type		integer NOT NULL,
	State		integer NOT NULL,
	PRIMARY KEY (id)
);
EOF
check_errno $?  ${TB_TASK}
#建立任务明细表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_TASKLIST} ;
create table ${TB_TASKLIST}(
	id			integer,
	Tk_id		integer NOT NULL,
	Rank		integer NOT NULL,
	Cmd			text 	NOT NULL,
	Wait_time	integer NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY(Tk_id) REFERENCES ${TB_TASK}(id) ON DELETE CASCADE ON UPDATE CASCADE
);
EOF
check_errno $? ${TB_TASKLIST}
#建立报警表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_WARN} ;
create table ${TB_WARN}(
	id 			integer,
	Add_time 	integer NOT NULL,
	Type 		integer NOT NULL,
	Grade	 	integer NOT NULL,
	State		integer NOT NULL,
	Addr 		integer NOT NULL,
	Remark 	text NOT NULL,
	PRIMARY KEY (id)
);
EOF
check_errno $? ${TB_WARN}
#隧道信息表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_TUNNEL} ;
create table ${TB_TUNNEL}(
	id 				integer,
	tun_bothway 	integer default 0,
	tun_speed   	integer default 80,
	tun_length   	integer default 200,
	tun_sensor   	integer default 0,
	PRIMARY KEY (id)
);
EOF
check_errno $? ${TB_TUNNEL}
#PWM对照表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_PWM} ;
create table ${TB_PWM}(
	id 			integer,
	level 		integer NOT NULL,
	light_min	integer NOT NULL,
	light_max	integer NOT NULL,
	PRIMARY KEY (id)
);
EOF
check_errno $? ${TB_PWM}
#隧道组信息表
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
drop table if exists ${TB_GRPINFO} ;
create table ${TB_GRPINFO}(
	id 			integer,
	gid 		integer NOT NULL,
	pwm_value	integer default 0,
	PRIMARY KEY (id)
);
EOF
check_errno $? ${TB_GRPINFO}
#建立唯一性约束索引(升序)
sqlite3 ${DB} << EOF
CREATE UNIQUE INDEX ${TB_COOR}_index_id ON ${TB_COOR}(id ASC);
CREATE UNIQUE INDEX ${TB_SINGLE}_index_id ON ${TB_SINGLE}(id ASC);
CREATE UNIQUE INDEX ${TB_INFO}_index_id ON ${TB_INFO}(Base_Addr ASC);
CREATE UNIQUE INDEX ${TB_TASK}_index_id ON ${TB_TASK}(id ASC);
CREATE UNIQUE INDEX ${TB_TASKLIST}_index_id ON ${TB_TASKLIST}(id ASC);
CREATE UNIQUE INDEX ${TB_WARN}_index_id ON ${TB_WARN}(id ASC);
EOF

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 ; do
sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
insert into ${TB_PWM} values($i ,$i,($i -1)*1000,($i -1) *1000+999);
EOF
done

sqlite3 ${DB} << EOF
PRAGMA foreign_keys = ON;
insert into ${TB_TUNNEL} values(1,0,80,400,1);
EOF

if [ $? ];then
	echo "Create database success!"
else
	echo "Create database error!"
fi
