#!/bin/sh

dbname=cc_corl.db
Table_Single=db_light
Table_Coordi=db_coordinator
Table_Task=db_task
Table_Tasklist=db_tasklist
Table_Warn=db_warn
Table_info=db_info_light
Table_tunnel=db_tunnel_info
Table_pwm_Index=db_index_pwm
Table_group_info=db_group_info

#开启外键支持
sqlite3 ${dbname} "PRAGMA foreign_keys = ON;"

#创建协调器记录表
sqlite3 ${dbname}  "drop table if exists ${Table_Coordi} ;"
sqlite3 ${dbname} \
"create table ${Table_Coordi}(
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
);"

#建立单灯记录表
sqlite3 ${dbname}  "drop table if exists ${Table_Single} ;"
sqlite3 ${dbname} \
"create table ${Table_Single}(
	id 			integer,
	Wl_Addr 	integer NOT NULL,
	Base_Addr 	integer NOT NULL,
	lt_gid 		integer NOT NULL,
	Coor_id	integer NOT NULL,
	Map_Addr	integer NOT NULL,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	FOREIGN KEY(Coor_id) REFERENCES ${Table_Coordi}(Base_Addr) ON DELETE CASCADE ON UPDATE CASCADE
);"

sqlite3 ${dbname}  "drop table if exists ${Table_info} ;"
sqlite3 ${dbname} \
"create table ${Table_info}(
	id 				integer,
	Base_Addr 		integer ,
	operate_flags 	integer default 0,
	Warn_flags 	integer default 0,
	Rate_v 			integer default 230,
	Rate_p 			integer default 0,
	Rate_PF 		integer default 0,
	light_status 	integer default 0,
	light_val  		integer default 0,
	light_E 			integer default 0,
	light_P 			integer default 0,
	light_V 			integer default 0,
	light_D 			integer default 0,
	rtime 			integer default 0,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	FOREIGN KEY(Base_Addr) REFERENCES $Table_Single(Base_Addr) ON DELETE CASCADE ON UPDATE CASCADE
);"

#建立任务表
sqlite3 ${dbname}  "drop table if exists ${Table_Task} ;"
sqlite3 ${dbname} \
"create table ${Table_Task}(
	id 			integer,
	Name 		text NOT NULL,
	Priority 	integer NOT NULL,
	Start_Date 	integer NOT NULL,
	End_Date 	integer NOT NULL,
	Run_Time	integer NOT NULL,
	Inter_Time	integer NOT NULL,
	Type 		integer NOT NULL,
	State 		integer NOT NULL,
	PRIMARY KEY (id)
);"

#建立任务明细表
sqlite3 ${dbname} "PRAGMA foreign_keys = ON; drop table if exists ${Table_Tasklist} ;\
create table ${Table_Tasklist}(
	id 			integer,
	Tk_id 		integer NOT NULL,
	Rank 		integer NOT NULL,
	Cmd	 	text 	NOT NULL,
	Wait_time	integer NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY(Tk_id) REFERENCES ${Table_Task}(id) ON DELETE CASCADE ON UPDATE CASCADE
);"

#建立报警表
sqlite3 ${dbname}  "drop table if exists ${Table_Warn} ;"
sqlite3 ${dbname} \
"create table ${Table_Warn}(
	id 			integer,
	Add_time 	integer NOT NULL,
	Type 		integer NOT NULL,
	Grade	 	integer NOT NULL,
	State		integer NOT NULL,
	Addr 		integer NOT NULL,
	Remark 	text NOT NULL,
	PRIMARY KEY (id)
);"

#隧道信息表
sqlite3 ${dbname}  "drop table if exists ${Table_tunnel} ;"
sqlite3 ${dbname} \
"create table ${Table_tunnel}(
	id 				integer,
	tun_bothway 	integer default 0,
	tun_speed   	integer default 80,
	tun_length   	integer default 200,
	tun_sensor   	integer default 0,
	PRIMARY KEY (id)
);"

sqlite3 ${dbname}  "drop table if exists ${Table_pwm_Index} ;"
sqlite3 ${dbname} \
"create table ${Table_pwm_Index}(
	id 			integer,
	level 		integer NOT NULL,
	light_min	integer NOT NULL,
	light_max	integer NOT NULL,
	PRIMARY KEY (id)
);"

sqlite3 ${dbname}  "drop table if exists ${Table_group_info} ;"
sqlite3 ${dbname} \
"create table ${Table_group_info}(
	id 			integer,
	gid 			integer NOT NULL,
	pwm_value	integer default 0,
	PRIMARY KEY (id)
);"



#建立唯一性约束索引(升序)
sqlite3 ${dbname} "CREATE UNIQUE INDEX ${Table_Coordi}_index_id ON ${Table_Coordi}(id ASC);"
sqlite3 ${dbname} "CREATE UNIQUE INDEX ${Table_Single}_index_id ON ${Table_Single}(id ASC);"
sqlite3 ${dbname} "CREATE UNIQUE INDEX ${Table_info}_index_id ON ${Table_info}(Base_Addr ASC);"
sqlite3 ${dbname} "CREATE UNIQUE INDEX ${Table_Task}_index_id ON ${Table_Task}(id ASC);"
sqlite3 ${dbname} "CREATE UNIQUE INDEX ${Table_Tasklist}_index_id ON ${Table_Tasklist}(id ASC);"
sqlite3 ${dbname} "CREATE UNIQUE INDEX ${Table_Warn}_index_id ON ${Table_Warn}(id ASC);"


echo "Create database success!"
