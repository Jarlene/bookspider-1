#ifndef _dbclient_h_
#define _dbclient_h_

#include <string>

class DBQueryResult
{
	DBQueryResult(const DBQueryResult&){}
	DBQueryResult& operator=(const DBQueryResult&){ return *this; }

public:
	DBQueryResult();
	~DBQueryResult();

	int GetRows() const;
	int GetCols() const;

	int FetchRow();

	int GetValue(int column, int& value);
	int GetValue(int column, bool& value);
	int GetValue(int column, double& value);
	int GetValue(int column, std::string& value);
	int GetValue(int column, char* value, int valueLen);

	int GetValue(const char* name, int& value);
	int GetValue(const char* name, bool& value);
	int GetValue(const char* name, double& value);
	int GetValue(const char* name, std::string& value);
	int GetValue(const char* name, char* value, int valueLen);

private:
	friend int db_query(void* db, const char* sql, DBQueryResult& result);
	int StoreResult(void* db);
	int FindColumn(const char* name) const;

private:
	void* m_db;
	void* m_result;
	void* m_row;
};

int db_init();
int db_fini();

void* db_connect(const char* ip, int port, const char* db, const char* username, const char* password);
int db_disconnect(void* db);

int db_query(void* db, const char* sql, DBQueryResult& result);
int db_insert(void* db, const char* sql);
int db_delete(void* db, const char* sql);
int db_update(void* db, const char* sql);

#endif /* !_dbclient_h_ */
