/********************************************************************
	> File Name:		database.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年03月30日 星期三 10时38分53秒
 *******************************************************************/
#include "database.h"

#define Database_Path 	"./cc_corl.db"




struct Errmsg{
	int errnum;
	char *Message;
};
struct Errmsg ERRMessage[]={
	[0]   = {0,"Successful result"},
	[1]   = {1,"SQL error or missing database"},
	[2]   = {2,"Internal logic error in SQLite "},
	[3]   = {3,"Access permission denied"},
	[4]   = {4,"Callback routine requested an abort "},
	[5]   = {5,"The database file is locked "},
	[6]   = {6,"A table in the database is locked"},
	[7]   = {7,"A malloc() failed"},
	[8]   = {8,"Attempt to write a readonly database "},
	[9]   = {9,"Operation terminated by sqlite3_interrupt()"},
	[10] = {10,"Some kind of disk I/O error occurred"},
	[11] = {11,"The database disk image is malformed"},
	[12] = {12,"Unknown opcode in sqlite3_file_control() "},
	[13] = {13,"Insertion failed because database is full "},
	[14] = {14,"Unable to open the database file "},
	[15] = {15,"Database lock protocol error"},
	[16] = {16,"Database is empty"},
	[17] = {17,"The database schema changed "},
	[18] = {18,"String or BLOB exceeds size limit "},
	[19] = {19,"Abort due to constraint violation"},
	[20] = {20,"Data type mismatch "},
	[21] = {21,"Library used incorrectly"},
	[22] = {22,"Uses OS features not supported on host "},
	[23] = {23,"Authorization denied"},
	[24] = {24,"Auxiliary database format error"},
	[25] = {25,"2nd parameter to sqlite3_bind out of range"},
	[26] = {26,"File opened that is not a database file"},
	[27] = {27,"Notifications from sqlite3_log()"},
	[28] = {28,"Warnings from sqlite3_log()"},
	[29] = {100,"sqlite3_step() has another row ready"},
	[30] = {101,"sqlite3_step() has finished executing "},
};


void ErrorMessage(int err)
{
	int i=0;
	if(err <= 28){
		printf("Error Message:%s\n",ERRMessage[err].Message);
	}else{
		i=29;
		while(i<sizeof(ERRMessage)/sizeof(ERRMessage[0])){
			if(ERRMessage[i].errnum == err){
				printf("Error Message:%s\n",ERRMessage[i].Message);
				break;
			}++i;
		}
		if(i == sizeof(ERRMessage)/sizeof(ERRMessage[0]) ){
			printf("Error Message:Unknown error Message!\n");
		}
	}
}

/**
 *  往数据库里插入一条数据
 *  cmd：向数据库的哪张表插入，后面跟的是对应的那张表的结构体数据
 *  返回值：
 *  	成功 返回SUCCESS  失败  FAIL
 *  eg:	Insert_Table(Cmd_coordi, &Coordi);
 *  注意：除了主键id不用赋值，其他成员必须要赋值
 */
s32 Insert_Table(u32 cmd, ...)
{
	char  sql[200]={0};
	u8      strtemp[24] = {0};
	char *Errmsg;
	int     res = -1;
	void*Arguments = NULL;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	va_list 	arg_ptr;
	va_start(arg_ptr, cmd);

	if( SQLITE_OK != sqlite3_open(Database_Path,&db) ){
		sqlite3_close(db);
		debug(DEBUG_sqlite3,"In %s %d:Open Sqlite fail!\n",__func__,__LINE__);
		Write_log(err,Asprintf("In %s %d:Open Sqlite fail!",__func__,__LINE__)  );
		goto out;
	}

	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,&Errmsg);


	memset(sql,0,sizeof(sql));
	switch(cmd){
		case Cmd_light:
			Arguments =  va_arg(arg_ptr, void*);
			va_end(arg_ptr);
			/* 数据库中主键自动生成 */
			sprintf(sql,"insert into %s(%s)  values(%d,%d,%d,%d,%d);",Table_Single,   Column_Single,
			((TableSingle_t*)Arguments)->Wl_Addr,   ((TableSingle_t*)Arguments)->Base_Addr,   ((TableSingle_t*)Arguments)->lt_gid,
								((TableSingle_t*)Arguments)->Coor_id,   ((TableSingle_t*)Arguments)->Map_Addr);
			break;
		case Cmd_coordi:
			Arguments = va_arg(arg_ptr, void*);
			va_end(arg_ptr);
			memset(strtemp,0,sizeof(strtemp));
			HexToStr_v3(strtemp, (u8*)( ((TableCoordi_t*)Arguments)->CC_id ),6);
			sprintf(sql,"insert into %s(%s)  values(%d,%d,%d,'%s',%d);",Table_Coordi,   Column_Coordi,
			((TableCoordi_t*)Arguments)->Wl_Addr,   ((TableCoordi_t*)Arguments)->Base_Addr,   ((TableCoordi_t*)Arguments)->Coor_gid,
			 									strtemp,   ((TableCoordi_t*)Arguments)->Map_Addr);
			break;
		case Cmd_task:
			Arguments = va_arg(arg_ptr, void*);
			va_end(arg_ptr);
			sprintf(sql,"insert into %s(%s)  values('%s',%d,%ld,%ld,%ld,%d,%d,%d);",Table_Task,   Column_Task,  ((TableTask_t*)Arguments)->Name,((TableTask_t*)Arguments)->Priority,
					((TableTask_t*)Arguments)->Start_Date, ((TableTask_t*)Arguments)->End_Date, ((TableTask_t*)Arguments)->Run_Time,
						((TableTask_t*)Arguments)->Inter_Time, ((TableTask_t*)Arguments)->Type,  ((TableTask_t*)Arguments)->State );
			break;
		case Cmd_tasklist:
			Arguments = va_arg(arg_ptr, void*);
			va_end(arg_ptr);
			sprintf(sql,"insert into %s(%s)  values(%d,%d,'%s',%d);",Table_Tasklist,   Column_Tasklist,  ((TableTasklist_t*)Arguments)->Tk_id,
					((TableTasklist_t*)Arguments)->Rank,((TableTasklist_t*)Arguments)->Cmd,((TableTasklist_t*)Arguments)->Wait_time);
			break;
		case Cmd_warn:
			Arguments = va_arg(arg_ptr, void*);
			va_end(arg_ptr);
			sprintf(sql,"insert into %s(%s)  values(%d,%d,%d,%d,%s);",Table_Warn,   Column_Warn,
			 ((Tablewarn_t*)Arguments)->Add_time,((Tablewarn_t*)Arguments)->Type,((Tablewarn_t*)Arguments)->Grade,
			 					((Tablewarn_t*)Arguments)->State,((Tablewarn_t*)Arguments)->Remark);
			break;
		default:break;
	}

	debug(DEBUG_sqlite3,"Sql: %s\n",sql);

	/* 准备对象 */
	int rr=-1;
	if( SQLITE_OK !=  (rr= sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL)) ){
		debug(DEBUG_sqlite3,"return values %d\n",rr);
		//debug(DEBUG_sqlite3,"ErrMessage:%s\n",sqlit3_errmsg(db));
		if (stmt){sqlite3_finalize(stmt);}
		sqlite3_close(db);
		debug(DEBUG_sqlite3,"**%s %d:Prepare Sqlite fail!\n",__func__,__LINE__);
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	if (SQLITE_DONE != (res=sqlite3_step(stmt)) ) {
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		debug(DEBUG_sqlite3,"**%s %d:Sqlite3_step fail,errno is %d\n",__func__,__LINE__,res);
		ErrorMessage(res);
		goto out;
	 }

	sqlite3_finalize(stmt);

	debug(DEBUG_sqlite3,"Insert table Success!\n");

	sqlite3_close(db);
	return SUCCESS;
 out:
	sqlite3_close(db);
	return FAIL;
}

/**
 * [ 往数据库里插入一条数据]
 * @param  sql [sql语句]
 * @return     [成功 返回SUCCESS  失败  FAIL]
 * eg:  Insert_Table_v2( Asprintf("insert into db_info_light(l_id,Base_Addr) values(%d,%d); ",55,0x1234) );
 */
s32 Insert_Table_v2(const char *sql)
{
	assert_param(sql,NULL,FAIL);

	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;

	if( SQLITE_OK != sqlite3_open(Database_Path,&db) ){
		debug(DEBUG_sqlite3,"[In %s %d] Open Sqlite fail!\n",__func__,__LINE__);
		Write_log(err,Asprintf("[In %s %d] Open Sqlite fail!",__func__,__LINE__)  );
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);
	debug(DEBUG_sqlite3,"Sql: %s\n",sql);
	/* 准备对象 */
	int res = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if( SQLITE_OK !=  res ){
		debug(DEBUG_sqlite3,"[In %s %d] Prepare Sqlite fail,return values%d\n",__FILE__,__LINE__,res);
		Write_log(err,Asprintf("[In %s %d] Prepare Sqlite fail,return values%d\n",__FILE__,__LINE__,res));
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	res = sqlite3_step(stmt);
	if (SQLITE_DONE != res ) {
		debug(DEBUG_sqlite3,"[In %s %d] Sqlite3_step fail,errno is %d\n",__FILE__,__LINE__,res);
		Write_log(err,Asprintf("[In %s %d] Sqlite3_step fail,errno is %d\n",__FILE__,__LINE__,res));
		goto out;
	 }

	if(stmt)	sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	return SUCCESS;
 out:
	if(stmt)	sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	debug(DEBUG_sqlite3,"Insert table fail!\n");
	return FAIL;
}

/**
 *  删除表中的一条
 *  cmd： 对应的表枚举
 *  Condition：删除的条件，字符串， 后面的后面的可变参数，可以根据cmd内容进行扩展
 *  返回值：
 *  	成功   SUCCESS   失败    FAIL
 *  eg： Delete_Table(Cmd_coordi,"where id=2");
 */
s32 Delete_Table(u32 cmd,const char *Condition)
{
	assert_param(Condition,NULL,FAIL);

	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;

	const int sqlsize = strlen(Condition) + 64;
	char* const sql = (char*)malloc(sqlsize);
	if(!sql){
		debug(DEBUG_sqlite3,"[In %s %s %d] malloc for sql fail!\n",__FILE__,__func__,__LINE__);
		return FAIL;
	}

	if( SQLITE_OK != sqlite3_open(Database_Path,&db) ){
		debug(DEBUG_sqlite3,"[In %s %s %d] Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		Write_log(err,Asprintf("[In %s %s %d] Open Sqlite fail!",__FILE__,__func__,__LINE__)  );
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);

	memset(sql,0,sqlsize);
	switch(cmd){
		case Cmd_light:	snprintf(sql,sqlsize,"delete from %s %s;",Table_Single,Condition);break;
		case Cmd_coordi:	snprintf(sql,sqlsize,"delete from %s %s;",Table_Coordi,Condition);break;
		case Cmd_task:		snprintf(sql,sqlsize,"delete from %s %s;",Table_Task,Condition);break;
		case Cmd_tasklist:	snprintf(sql,sqlsize,"delete from %s %s;",Table_Tasklist,Condition);break;
		case Cmd_warn:	snprintf(sql,sqlsize,"delete from %s %s;",Table_Warn,Condition);break;
		case Cmd_Info: 	snprintf(sql,sqlsize,"delete from %s %s;",Table_Info,Condition);break;
		default:			debug(DEBUG_sqlite3,"[%s]Can not find the table!\n",__func__);
			goto out;
	} 		debug(DEBUG_sqlite3,"Sql: %s\n",sql);
	/* 准备对象 */
	if( SQLITE_OK != sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL) ){
		debug(DEBUG_sqlite3,"[In %s %s %d] Prepare Sqlite fail!\n",__FILE__,__func__,__LINE__);
		Write_log(err,Asprintf("[In %s %s %d] Prepare Sqlite fail!",__FILE__,__func__,__LINE__)  );
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	int res = sqlite3_step(stmt);
	if (SQLITE_DONE != res ) {
		debug(DEBUG_sqlite3,"[In %s %s %d] Sqlite3_step fail,errno is %d\n",__FILE__,__func__,__LINE__,res);
		Write_log(err,Asprintf("[In %s %s %d] Sqlite3_step fail,errno is %d",__FILE__,__func__,__LINE__,res)  );
		goto out;
	}

	if(sql)	free(sql);
 	if (stmt)sqlite3_finalize(stmt);
 	if(db)	sqlite3_close(db);
	return 	SUCCESS;
 out:
 	if(sql)	free(sql);
	if (stmt)sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	debug(DEBUG_sqlite3,"Delete_Table fail!\n");
	return FAIL;
}

/**
 *  更新表中的一个字段
 *  cmd    对应的表枚举
 *  Condition：更新的条件，字符串
 *  返回值：
 *  	成功   SUCCESS   失败    FAIL
 *  eg： Update_Table(Cmd_coordi,"set Wl_Addr=0x88 where id=6");
 */
s32 Update_Table(u32 cmd,const char *Condition)
{
	assert_param(Condition,NULL,FAIL);

	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;

	const int sqlsize = strlen(Condition) + 64;
	char* const sql = (char*)malloc(sqlsize);
	if(!sql){
		debug(DEBUG_sqlite3,"[In %s %s %d] malloc for sql fail!\n",__FILE__,__func__,__LINE__);
		return FAIL;
	}

	if( SQLITE_OK != sqlite3_open(Database_Path,&db) ){
		debug(DEBUG_sqlite3,"In %s %s %d:Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		Write_log(err,Asprintf("In %s %s %d:Open Sqlite fail!",__FILE__,__func__,__LINE__)  );
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);

	memset(sql,0,sqlsize);
	switch(cmd){
		case Cmd_light:	snprintf(sql,sqlsize,"update %s %s;",Table_Single,Condition);break;
		case Cmd_coordi:	snprintf(sql,sqlsize,"update %s %s;",Table_Coordi,Condition);break;
		case Cmd_task:		snprintf(sql,sqlsize,"update %s %s;",Table_Task,Condition);break;
		case Cmd_tasklist:	snprintf(sql,sqlsize,"update %s %s;",Table_Tasklist,Condition);break;
		case Cmd_warn:	snprintf(sql,sqlsize,"update %s %s;",Table_Warn,Condition);break;
		case Cmd_Info: 	snprintf(sql,sqlsize,"update %s %s;",Table_Info,Condition);break;
		default:			debug(DEBUG_sqlite3,"[%s]Can not find the table!\n",__func__);
			goto out;
	}		debug(DEBUG_sqlite3,"Sql: %s\n",sql);
	/* 准备对象 */
	if( SQLITE_OK != sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL) ){
		debug(DEBUG_sqlite3,"In %s %s %d:Prepare Sqlite fail!\n",__FILE__,__func__,__LINE__) ;
		Write_log(err,Asprintf("In %s %s %d:Prepare Sqlite fail!",__FILE__,__func__,__LINE__)  );
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	int res = sqlite3_step(stmt);
	if (SQLITE_DONE != res  ) {
		debug(DEBUG_sqlite3,"In %s %s %d:Sqlite3_step fail,errno is %d\n",__FILE__,__func__,__LINE__,res);
		Write_log(err,Asprintf("In %s %s %d:Sqlite3_step fail,errno is %d",__FILE__,__func__,__LINE__,res)  );
		goto out;
	 }

	if(sql)	free(sql);
	if (stmt)sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	debug(DEBUG_sqlite3,"Update_Table success!\n");
	return SUCCESS;
 out:
 	if(sql)	free(sql);
	if (stmt)sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	debug(DEBUG_sqlite3,"Update_Table fail!\n");
	return FAIL;
}

/**
 *  查询表内容
 *  cmd    	对应的表枚举
 *  Column    	查询字段的内容
 *  Condition    	查询的条件
 *  后面跟的是查询内容的结构体，和结构体的大小
 *  eg：		struct {u32 Wl_Addr;u32 Map_Addr;} Res[10];
 *		Select_Table(Cmd_light ,"Wl_Addr,Map_Addr","where id=1",Res,sizeof(Res[0]));
 */
s32 Select_Table(u32 cmd,const char *Column,const char*Condition,...)
{
	u32 i = 0;
	char sql[128]={0};
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	void *Result = NULL;
	u32 Resultlen = 0;
	u32 Res_size = 0;

	 va_list 	arg_ptr;
	 va_start(arg_ptr, Condition);
	 Result = va_arg(arg_ptr,void*);
	 Resultlen = va_arg(arg_ptr,int);
	 Res_size = va_arg(arg_ptr,int);
	 va_end(arg_ptr);

	if( SQLITE_OK != sqlite3_open(Database_Path,&db) ){
		debug(DEBUG_sqlite3,"**%s %d:Open Sqlite fail!\n",__func__,__LINE__);
		goto err;
	}

	memset(sql,0,sizeof(sql));
	switch(cmd){
		case Cmd_light:	sprintf(sql,"select %s from %s %s;",Column,Table_Single,Condition);break;
		case Cmd_coordi:	sprintf(sql,"select %s from %s %s;",Column,Table_Coordi,Condition);break;
		case Cmd_task: 	sprintf(sql,"select %s from %s %s;",Column,Table_Task,Condition);break;
		case Cmd_tasklist:	sprintf(sql,"select %s from %s %s;",Column,Table_Tasklist,Condition);break;
		case Cmd_warn:	sprintf(sql,"select %s from %s %s;",Column,Table_Warn,Condition);break;
		default:break;
	}

	debug(DEBUG_sqlite3,"Sql: %s\n",sql);
	/* 准备对象 */
	if( SQLITE_OK != sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL) ){
		if (stmt){sqlite3_finalize(stmt);}
		debug(DEBUG_sqlite3,"**%s %d:Prepare Sqlite fail!\n",__func__,__LINE__);
		goto err;
	}

	int Col_cnt = sqlite3_column_count(stmt);

	while(1){
		if(!Res_size--){
			debug(DEBUG_sqlite3,"Result Buf Full!\n");
			break;
		}
		char *pchar = NULL;
		int res = sqlite3_step(stmt);
		if(res == SQLITE_ROW ){
			s8 *pResult = (s8*)Result;
			for(i=0;i<Col_cnt;++i){
				switch(sqlite3_column_type(stmt,i)){
					case SQLITE_INTEGER:
						*((int*)pResult) = sqlite3_column_int(stmt,i);
						//debug(DEBUG_sqlite3,"pResult = %d\n",*((int*)pResult));
						pResult += sizeof(int);break;
					case SQLITE_FLOAT:
						*((double*)pResult) = sqlite3_column_double(stmt,i);
						pResult += sizeof(double);break;
					case SQLITE_TEXT:
						pchar = (char*)sqlite3_column_text(stmt,i);
						strcpy(pResult,pchar);
						pResult += strlen(pchar);
						*pResult++ = 0;
						break;
					case SQLITE_NULL:	//建表的时候把每个字段都设置为非空
						debug(DEBUG_sqlite3,"The value is NULL.");
						break;
					default:break;
				}
			}
			Result += Resultlen;	//指向数组的下一个元素
		}else if(res == SQLITE_DONE){
			debug(DEBUG_sqlite3,"**%s %d:Select Finished.\n",__func__,__LINE__);
			break;
		}else{
			debug(DEBUG_sqlite3,"**%s %d:Select Failed.\n",__func__,__LINE__);
			sqlite3_finalize(stmt);
			goto err;
		}
	}

	 debug(DEBUG_sqlite3,"Select Data Success!\n");

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return SUCCESS;
 err:
	sqlite3_close(db);
	return FAIL;
}

/**
 * 	查询表内容
 * 	sql 对应的sql语句
 * 	buf 查询出来的内容存储缓冲区
 * 	RowSize 一个数组元素占多少字节
 * 	ColSize 数组元素的个数
 * 	如果包含字符串的话：
 * 		后面的可变参数要输入查询语句包含多少个字符串，紧接着依次写入每个字符串的缓存大小，
 * 		如果没有字符串，可变参数传入0即可
 * 	成功返回SUCCESS ，失败返回FAIL
 * eg:	struct {int CoordiAddr;int SingleAddr;}buf[1000];
 * 	Select_Table_V2( sql,  (char*)buf,   sizeof(buf[0]),   sizeof(buf)/sizeof(buf[0]),  0);
 */
s32 Select_Table_V2(const char *sql, char *buf,int RowSize,int ColSize,int strcount,...)
{
	assert_param(sql,NULL,FAIL);
	assert_param(buf,NULL,FAIL);

	u32 i=0,j=0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	u32 strsize[10] = {0};		//记入字符串的缓存空间大小
	s8 *pResult = buf;
	va_list 	arg_ptr;

	if(strcount > 0 && strcount < 9){
		va_start(arg_ptr, strcount);
		for(i=0;i< strcount;++i){
			strsize[i] = va_arg(arg_ptr,int);
		}va_end(arg_ptr);
	}

	if( SQLITE_OK != sqlite3_open(Database_Path,&db) ){
		debug(DEBUG_sqlite3,"[In %s-%s-%d] Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		Write_log(err,Asprintf("[In %s-%s-%d] Open Sqlite fail!",__FILE__,__func__,__LINE__)  );
		goto out;
	}

	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);
	debug(DEBUG_sqlite3,"Sql: %s\n",sql);

	/* 准备对象 */
	int res = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if( SQLITE_OK !=  res){
		debug(DEBUG_sqlite3,"[In %s-%s-%d] Prepare Sqlite fail,errno=%d\n",__FILE__,__func__,__LINE__,res);
		Write_log(err,Asprintf("[In %s-%s-%d] Prepare Sqlite fail!",__FILE__,__func__,__LINE__)   );
		goto out;
	}

	int Col_cnt = sqlite3_column_count(stmt);
	if(Col_cnt < 0) goto out;

	while(1){
		if(!ColSize--){  break;  }		//如果超出元素的个数，表示buf满了
		//char *pchar = NULL;

		res = sqlite3_step(stmt);
		if(res == SQLITE_ROW ){
			pResult = buf;
			i = 0; j = 0;
			while(i<Col_cnt){
				switch(sqlite3_column_type(stmt,i)){
					case SQLITE_INTEGER:  *((int*)pResult) = sqlite3_column_int(stmt,i);
						pResult += sizeof(int);break;
					case SQLITE_FLOAT:  *((double*)pResult) = sqlite3_column_double(stmt,i);
						pResult += sizeof(double);break;
					case SQLITE_TEXT:  strcpy(pResult,(const char*)sqlite3_column_text(stmt,i) );
						pResult += strsize[j++];  break;
					case SQLITE_NULL:
						debug(DEBUG_sqlite3,"Select value is NULL.\n");
						Write_log(warring,Asprintf("[In %s-%s-%d] Select value is NULL.",__FILE__,__func__,__LINE__) );
						goto out;
					default:break;
				} ++i;
			}
			buf += RowSize;	//指向数组的下一个元素
		}else if(res == SQLITE_DONE){
			break;
		}else{
			debug(DEBUG_sqlite3,"[In %s-%s-%d] Select Failed,errno is %d\n",__FILE__,__func__,__LINE__,res);
			Write_log(err,Asprintf("[In %s-%s-%d] Select Failed,errno is %d",__FILE__,__func__,__LINE__,res)   );
			goto out;
		}
	}

	if(stmt)	sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	return SUCCESS;
 out:
 	if(stmt)	sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	debug(DEBUG_sqlite3,"Select Data fail!\n");
	return FAIL;
}
