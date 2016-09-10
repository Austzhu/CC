#########################################################################
# File Name: create_database.sh
# Author: Austzhu
# mail: 153462902@.com
# Created Time: 2016年05月17日 星期二 11时29分08秒
#########################################################################
#!/bin/sh
dbname=cc_corl.db
Table_Single=db_light
Table_Coordi=db_coordinator
Table_Task=db_task
Table_Tasklist=db_tasklist
Table_Warn=db_warn

#创建协调器记录表
sqlite3 $dbname  "drop table if exists $Table_Coordi ;"
sqlite3 $dbname \
"create table $Table_Coordi(
	id 		integer,
	Wl_Addr 	integer NOT NULL,
	Base_Addr 	integer NOT NULL,
	Coor_gid 	integer NOT NULL,
	CC_id		text NOT NULL,
	Map_Addr 	integer NOT NULL,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	CHECK (Base_Addr < 0xff AND Base_Addr > 0x20)
);"

#开启外键支持
sqlite3 $dbname "PRAGMA foreign_keys = ON;"

#建立单灯记录表
sqlite3 $dbname  "drop table if exists $Table_Single ;"
sqlite3 $dbname \
"create table $Table_Single(
	id 		integer,
	Wl_Addr 	integer NOT NULL,
	Base_Addr 	integer NOT NULL,
	lt_gid 		integer NOT NULL,
	Coor_id	integer NOT NULL,
	Map_Addr	integer NOT NULL,
	PRIMARY KEY (id),
	UNIQUE (Base_Addr),
	FOREIGN KEY(Coor_id) REFERENCES $Table_Coordi(Base_Addr) ON DELETE CASCADE ON UPDATE CASCADE
);"

#建立任务表
sqlite3 $dbname  "drop table if exists $Table_Task ;"
sqlite3 $dbname \
"create table $Table_Task(
	id 		integer,
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
sqlite3 $dbname "PRAGMA foreign_keys = ON; drop table if exists $Table_Tasklist ;\
create table $Table_Tasklist(
	id 		integer,
	Tk_id 		integer NOT NULL,
	Rank 		integer NOT NULL,
	Cmd	 	text 	NOT NULL,
	Wait_time	integer NOT NULL,
	PRIMARY KEY (id),
	FOREIGN KEY(Tk_id) REFERENCES $Table_Task(id) ON DELETE CASCADE ON UPDATE CASCADE
);"

#建立报警表
#建立任务明细表
sqlite3 $dbname  "drop table if exists $Table_Warn ;"
sqlite3 $dbname \
"create table $Table_Warn(
	id 		integer,
	Add_time 	integer NOT NULL,
	Type 		integer NOT NULL,
	Grade	 	integer NOT NULL,
	State		integer NOT NULL,
	Remark 	text NOT NULL,
	PRIMARY KEY (id)
);"


#协调器演示数据
#id,    wl_addr,    base_addr,    coor_gid,    cc_id,    map_addr
echo "Coordinator tables..."
#sqlite3 $dbname "insert into $Table_Coordi values(0,0,0x70,1,'00000000c121',0x01);"
#sqlite3 $dbname "insert into $Table_Coordi values(1,1,0x71,2,'00000000c121',0x02);"
#sqlite3 $dbname "insert into $Table_Coordi values(2,2,3,2,'00000000b2c5',0x84);"
#sqlite3 $dbname "insert into $Table_Coordi values(3,3,4,3,'00000000b2c5',0x85);"
#sqlite3 $dbname "select * from $Table_Coordi;"
#单灯演示数据
#id,    wl_addr,    base_addr,    lt_gid,    map_addr,    coor_id
echo  "Single light tables..."
#sqlite3 $dbname "insert into $Table_Single values(  0,   0,0x010B,1,0x70,0x01);"
#sqlite3 $dbname "insert into $Table_Single values(  1,   1,0x00AB,2,0x70,0x02);"
#sqlite3 $dbname "insert into $Table_Single values(  2,   2,0x0224, 2,0x70,0x03);"
#sqlite3 $dbname "insert into $Table_Single values(  3,   3,0x016B, 2,0x70,0x04);"
#sqlite3 $dbname "insert into $Table_Single values(  4,   4,0x020D, 2,0x70,0x05);"
#sqlite3 $dbname "insert into $Table_Single values(  5,   5,0x023D, 1,0x70,0x06);"
#sqlite3 $dbname "insert into $Table_Single values(  6,   6,0x0204, 3,0x70,0x07);"
#sqlite3 $dbname "insert into $Table_Single values(  7,   7,0x0052, 3,0x70,0x08);"
#sqlite3 $dbname "insert into $Table_Single values(  8,   8,0x0098, 3,0x70,0x09);"
#sqlite3 $dbname "insert into $Table_Single values(  9,   9,0x01F3, 3,0x70,0x0A);"
#sqlite3 $dbname "insert into $Table_Single values(10,10,0x0296, 3,0x70,0x0B);"
#sqlite3 $dbname "insert into $Table_Single values(11,11,0x01AC,4,0x70,0x0C);"
#sqlite3 $dbname "insert into $Table_Single values(12,12,0x022F, 4,0x70,0x0D);"
#sqlite3 $dbname "insert into $Table_Single values(13,13,0x0071, 4,0x70,0x0E);"
#sqlite3 $dbname "insert into $Table_Single values(14,14,0x0063, 4,0x70,0x0F);"
#sqlite3 $dbname "insert into $Table_Single values(15,15,0x023C, 4,0x71,0x01);"
#sqlite3 $dbname "select * from $Table_Single;"

#任务表中的演示数据
echo  "Task tables..."
#sqlite3 $dbname "insert into $Table_Task values(0,'tast01',1,201606,201609,201608,55,1,1);"
#sqlite3 $dbname "insert into $Table_Task values(1,'tast02',2,201606,201609,201608,80,2,1);"
#sqlite3 $dbname "select * from $Table_Task;"
echo "Task list tables..."
#sqlite3 $dbname "insert into $Table_Tasklist values(0,0,1,'0303420f0e',0);"
#sqlite3 $dbname "insert into $Table_Tasklist values(1,0,2,'a2080310031b030f0305',0);"
#sqlite3 $dbname "insert into $Table_Tasklist values(2,0,3,'a4020586',0);"

#sqlite3 $dbname "insert into $Table_Tasklist values(3,1,1,'0303420f0e',0);"
#sqlite3 $dbname "insert into $Table_Tasklist values(4,1,2,'a2080310031b030f0305',0);"
#sqlite3 $dbname "insert into $Table_Tasklist values(5,2,3,'a4020586',0);"
#sqlite3 $dbname "select * from $Table_Tasklist;"

#建立唯一性约束索引(升序)
sqlite3 $dbname "CREATE UNIQUE INDEX ${Table_Coordi}_index_id ON $Table_Coordi(id ASC);"
sqlite3 $dbname "CREATE UNIQUE INDEX ${Table_Single}_index_id ON $Table_Single(id ASC);"
sqlite3 $dbname "CREATE UNIQUE INDEX ${Table_Task}_index_id ON $Table_Task(id ASC);"
sqlite3 $dbname "CREATE UNIQUE INDEX ${Table_Tasklist}_index_id ON $Table_Tasklist(id ASC);"
sqlite3 $dbname "CREATE UNIQUE INDEX ${Table_Warn}_index_id ON $Table_Warn(id ASC);"
