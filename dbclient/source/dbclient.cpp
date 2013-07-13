#include "dbclient.h"
#include "cstringext.h"
#include <stdio.h>
#include <assert.h>
#include <WinSock2.h>
extern "C"
{
#include "mysql.h"
}

int db_init()
{
	static char *server_args[] = {
		"webspider",       /* this string is not used */
		"--datadir=.",
		"--key_buffer_size=32M"
	};
	static char *server_groups[] = {
		"embedded",
		"server",
		"webspider_SERVER",
		(char *)NULL
	};

	return mysql_library_init(sizeof(server_args) / sizeof(char *), server_args, server_groups);
}

int db_fini()
{
	mysql_library_end();
	return 0;
}

void* db_connect(const char* ip, int port, const char* db, const char* username, const char* password)
{
	MYSQL* mysql = mysql_init(NULL);
	if(!mysql)
		return NULL;

	if(!mysql_real_connect(mysql, ip, username, password, db, port, NULL, 0))
	{
		printf("mysql connect error: %s\n", mysql_error(mysql));
		db_disconnect(mysql);
		return NULL;
	}

	return mysql;
}

int db_disconnect(void* db)
{
	MYSQL* mysql = (MYSQL*)db;
	mysql_close(mysql);
	return 0;
}

int db_query(void* db, const char* sql, DBQueryResult& result)
{
	MYSQL* mysql = (MYSQL*)db;
	if(mysql_real_query(mysql, sql, strlen(sql)))
		return (int)mysql_errno(mysql);
	return result.StoreResult(db);
}

int db_insert(void* db, const char* sql)
{
	MYSQL* mysql = (MYSQL*)db;
	if(mysql_real_query(mysql, sql, strlen(sql)))
		return (int)mysql_errno(mysql);

	MYSQL_RES* result = mysql_store_result(mysql);
	int r = (int)mysql_errno(mysql);
	if(result)
		mysql_free_result(result);
	return r;
}

int db_delete(void* db, const char* sql)
{
	MYSQL* mysql = (MYSQL*)db;
	if(mysql_real_query(mysql, sql, strlen(sql)))
		return (int)mysql_errno(mysql);

	MYSQL_RES* result = mysql_store_result(mysql);
	int r = (int)mysql_errno(mysql);
	if(result)
		mysql_free_result(result);
	return r;
}

int db_update(void* db, const char* sql)
{
	MYSQL* mysql = (MYSQL*)db;
	if(mysql_real_query(mysql, sql, strlen(sql)))
		return (int)mysql_errno(mysql);

	MYSQL_RES* result = mysql_store_result(mysql);
	int r = (int)mysql_errno(mysql);
	if(result)
		mysql_free_result(result);
	return r;
}

DBQueryResult::DBQueryResult()
{
	m_db = NULL;
	m_row = NULL;
	m_result = NULL;
}

DBQueryResult::~DBQueryResult()
{
	if(m_result)
		mysql_free_result((MYSQL_RES*)m_result);
}

int DBQueryResult::StoreResult(void* db)
{
	assert(db);
	MYSQL* mysql = (MYSQL*)db;

	m_db = db;
	m_result = mysql_store_result(mysql);
	if(m_result)
		return 0;
	assert(0==mysql_field_count(mysql));
	return (int)mysql_errno(mysql);
}

int DBQueryResult::GetRows() const
{
	if(!m_result)
		return 0;
	return (int)mysql_num_rows((MYSQL_RES*)m_result);
}

int DBQueryResult::GetCols() const
{
	if(!m_result)
		return 0;
	return (int)mysql_num_fields((MYSQL_RES*)m_result);
}

int DBQueryResult::FetchRow()
{
	if(!m_result)
		return -1;

	m_row = (void*)mysql_fetch_row((MYSQL_RES*)m_result);
	return m_row?0:-1;
}

int DBQueryResult::GetValue(int column, int& value)
{
	std::string v;
	int r = GetValue(column, v);
	if(r)
		return r;
	value = atoi(v.c_str());
	return 0;
}

int DBQueryResult::GetValue(int column, bool& value)
{
	std::string v;
	int r = GetValue(column, v);
	if(r)
		return r;
	value = strieq(v.c_str(), "True");
	return 0;
}

int DBQueryResult::GetValue(int column, double& value)
{
	std::string v;
	int r = GetValue(column, v);
	if(r)
		return r;
	value = atof(v.c_str());
	return 0;
}

int DBQueryResult::GetValue(int column, std::string& value)
{
	MYSQL_ROW row = (MYSQL_ROW)m_row;
	if(!row)
		return -1;

	if(column > GetCols())
		return -1;

	value.assign(row[column]?row[column]:"");
	return 0;
}

int DBQueryResult::GetValue(int column, char* value, int valueLen)
{
	MYSQL_ROW row = (MYSQL_ROW)m_row;
	if(!row)
		return -1;

	if(column > GetCols())
		return -1;

	value[0] = 0;
	if(row[column])
	{
		int n = strlen(row[column]);
		strncpy(value, row[column], n>valueLen?valueLen:n);
		value[n>valueLen?valueLen:n] = 0;
	}
	return 0;
}

int DBQueryResult::GetValue(const char* name, int& value)
{
	int column = FindColumn(name);
	if(column < 0)
		return -1;
	return GetValue(column, value);
}

int DBQueryResult::GetValue(const char* name, bool& value)
{
	int column = FindColumn(name);
	if(column < 0)
		return -1;
	return GetValue(column, value);
}

int DBQueryResult::GetValue(const char* name, double& value)
{
	int column = FindColumn(name);
	if(column < 0)
		return -1;
	return GetValue(column, value);
}

int DBQueryResult::GetValue(const char* name, std::string& value)
{
	int column = FindColumn(name);
	if(column < 0)
		return -1;
	return GetValue(column, value);
}

int DBQueryResult::GetValue(const char* name, char* value, int valueLen)
{
	int column = FindColumn(name);
	if(column < 0)
		return -1;
	return GetValue(column, value, valueLen);
}

int DBQueryResult::FindColumn(const char* name) const
{
	unsigned int num = mysql_num_fields((MYSQL_RES*)m_result);
	MYSQL_FIELD* fields = mysql_fetch_fields((MYSQL_RES*)m_result);
	for(unsigned int i=0; i<num; i++)
	{
		if(streq(name, fields[i].name))
			return (int)i;
	}
	return -1;
}
