/********************************************************************
	> File Name:	database.c
	> Author:		Austzhu
	> Mail:			153462902@qq.com.com
	> Created Time:	2016年03月30日 星期三 10时38分53秒
 *******************************************************************/
#include "database.h"

/**
 * [ 往数据库里插入一条数据]
 * @param  sql [sql语句]
 * @return     [成功 返回SUCCESS  失败  FAIL]
 * eg:  Insert_Table_v2( Asprintf("insert into db_info_light(l_id,Base_Addr) values(%d,%d); ",55,0x1234) );
 */
static int sql_Insert(const char *sql)
{
	assert_param(sql,FAIL);
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	int res = sqlite3_open(CFG_DB_NAME,&db);
	if( SQLITE_OK != res ){
		debug(DEBUG_sqlite3,"[In %s %d] Open Sqlite fail!\n",__func__,__LINE__);
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);
	/* 准备对象 */
	res = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if( SQLITE_OK !=  res ){
		debug(DEBUG_sqlite3,"[In %s %d] Prepare Sqlite fail!\n",__FILE__,__LINE__);
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	res = sqlite3_step(stmt);
	if (SQLITE_DONE != res ) {
		debug(DEBUG_sqlite3,"[In %s %d] Sqlite3_step fail!\n",__FILE__,__LINE__);
		goto out;
	 }
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return SUCCESS;
 out:
 	debug(DEBUG_sqlite3,"Insert error number is %d\nSql: %s\n",res,sql);
	if(stmt)	sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	return FAIL;
}

static int sql_delete(const char *sql)
{
	assert_param(sql,FAIL);

	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	int res =  sqlite3_open(CFG_DB_NAME,&db);
	if( SQLITE_OK != res){
		debug(DEBUG_sqlite3,"[In %s %s %d] Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);
	/* 准备对象 */
	res = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL) ;
	if( SQLITE_OK != res ){
		debug(DEBUG_sqlite3,"[In %s %s %d] Prepare Sqlite fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	res = sqlite3_step(stmt);
	if (SQLITE_DONE != res ) {
		debug(DEBUG_sqlite3,"[In %s %s %d] Sqlite3_step fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	sqlite3_finalize(stmt);
 	sqlite3_close(db);
	return 	SUCCESS;
 out:
 	debug(DEBUG_sqlite3,"delete error number is %d\nSql: %s\n",res,sql);
	if (stmt)sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	return FAIL;
}

/**
 *  更新表中的一个字段
 *  table    对应的表
 *  Condition：更新的条件，字符串
 *  返回值：
 *  	成功   SUCCESS   失败    FAIL
 *  eg： Update_Table("db_light","set Wl_Addr=0x88 where id=6");
 */
static int sql_update(const char *table,const char *Condition)
{
	assert_param(Condition,FAIL);
	assert_param(table,FAIL);

	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	int32_t res = -1;
	char* const sql = calloc(strlen(table) + strlen(Condition) + 12 , sizeof(char));
	if(!sql) goto out;
	res = sqlite3_open(CFG_DB_NAME,&db);
	if( SQLITE_OK != res ){
		debug(DEBUG_sqlite3,"In %s %s %d:Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	sprintf(sql,"update %s %s;",table,Condition);
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);	/* 开启外键约束 */
	/* 准备对象 */
	res = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if( SQLITE_OK != res ){
		debug(DEBUG_sqlite3,"In %s %s %d:Prepare Sqlite fail!\n",__FILE__,__func__,__LINE__) ;
		goto out;
	}
	/* 执行操作，向数据库里插入数据 */
	res = sqlite3_step(stmt);
	if (SQLITE_DONE != res  ) {
		debug(DEBUG_sqlite3,"In %s %s %d:Sqlite3_step fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	 }
	free(sql);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return SUCCESS;
 out:
 	debug(DEBUG_sqlite3,"update error number is %d\nSql: %s\n",res,sql);
 	if(sql) free(sql);
	if(stmt) sqlite3_finalize(stmt);
	if(db)  sqlite3_close(db);
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
 * 	sql_select( sql,  (char*)buf,   sizeof(buf[0]),   sizeof(buf)/sizeof(buf[0]),  0);
 */
static int sql_select(const char *sql, char *buf,int RowSize,int ColSize,int strcount,...)
{
	assert_param(sql,FAIL);
	assert_param(buf,FAIL);

	uint32_t i=0,j=0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	uint32_t strsize[10] = {0};		//记入字符串的缓存空间大小
	s8 *pResult = buf;
	va_list 	arg_ptr;

	if(strcount > 0 && strcount < 9){
		va_start(arg_ptr, strcount);
		for(i=0;i< strcount;++i){
			strsize[i] = va_arg(arg_ptr,int);
		}va_end(arg_ptr);
	}
	int res = sqlite3_open(CFG_DB_NAME,&db);
	if( SQLITE_OK != res ){
		debug(DEBUG_sqlite3,"[In %s-%s-%d] Open Sqlite fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	/* 开启外键约束 */
	sqlite3_exec(db,"PRAGMA foreign_keys = ON;", NULL, NULL,NULL);
	/* 准备对象 */
	res = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if( SQLITE_OK !=  res){
		debug(DEBUG_sqlite3,"[In %s-%s-%d] Prepare Sqlite fail!\n",__FILE__,__func__,__LINE__);
		goto out;
	}
	int Col_cnt = sqlite3_column_count(stmt);
	if(Col_cnt <= 0) goto out;

	while(1){
		if(!ColSize--) break;  	//如果超出元素的个数，表示buf满了
		res = sqlite3_step(stmt);
		if(res == SQLITE_ROW ){
			pResult = buf;
			i = 0, j = 0;
			while(i<Col_cnt){
				switch(sqlite3_column_type(stmt,i)){
					case SQLITE_INTEGER:
						*((int*)pResult) = sqlite3_column_int(stmt,i);
						pResult += sizeof(int);	break;
					case SQLITE_FLOAT:
						*((double*)pResult) = sqlite3_column_double(stmt,i);
						pResult += sizeof(double);	break;
					case SQLITE_TEXT:
						strcpy(pResult,(const char*)sqlite3_column_text(stmt,i) );
						pResult += strsize[j++];  break;
					case SQLITE_NULL:
						debug(DEBUG_sqlite3,"Select value is NULL.\n");
						goto out1;
					default:	break;
				}	++i;
			}	buf += RowSize;	//指向数组的下一个元素
		}else if(res == SQLITE_DONE){
			break;
		}else{
			debug(DEBUG_sqlite3,"[In %s-%s-%d] Select Failed!\n",__FILE__,__func__,__LINE__);
			goto out;
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return SUCCESS;
 out:
 	debug(DEBUG_sqlite3,"select error number is %d\nSql: %s\n",res,sql);
 out1:
 	if(stmt)	sqlite3_finalize(stmt);
	if(db)	sqlite3_close(db);
	return FAIL;
}

static int sql_Isexist(const char *table, const char *condition)
{
	if( !table || !condition) return FALSE;
	int32_t  id = FALSE;
	char buf[12] = {0};
	strcpy(buf,condition);
	sql_select(Asprintf("select id from %s where %s;",table,buf),(char*)&id,sizeof(int),1,0);
	return id == FALSE ? FALSE : TRUE;
}

static void inline sql_release(struct sql_t*this)
{
	FREE(this);
}

sql_t *sql_Init(sql_t *this)
{
	sql_t *sql = this;
	if(!this){
		sql = malloc(sizeof(sql_t));
		if(!sql) return NULL;
	}
	bzero(sql,sizeof(sql_t));
	sql->sql_insert = sql_Insert;
	sql->sql_delete = sql_delete;
	sql->sql_update = sql_update;
	sql->sql_select = sql_select;
	sql->sql_Isexist = sql_Isexist;
	sql->sql_release = sql_release;
	return sql;
}
