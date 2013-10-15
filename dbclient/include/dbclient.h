#ifndef _dbclient_h_
#define _dbclient_h_

#include "dllexport.h"
#include <string>

#ifdef DBCLIENT_EXPORTS
	#define DBCLIENT_API DLL_EXPORT_API
#else
	#define DBCLIENT_API DLL_IMPORT_API
#endif

class DBCLIENT_API DBQueryResult
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

	int StoreResult(void* db);

private:
	int FindColumn(const char* name) const;

private:
	void* m_db;
	void* m_result;
	void* m_row;
};

DBCLIENT_API int db_init();
DBCLIENT_API int db_fini();

DBCLIENT_API void* db_connect(const char* ip, int port, const char* db, const char* username, const char* password);
DBCLIENT_API int db_disconnect(void* db);

/// @return: <0-sql error, =0-ok
DBCLIENT_API int db_query(void* db, const char* sql, DBQueryResult& result);
DBCLIENT_API int db_query_int(void* db, const char* sql, int* value);
DBCLIENT_API int db_query_string(void* db, const char* sql, char* value, int bytes);

/// @return: <0-sql error, >0-insert rows, =0-no matched
DBCLIENT_API int db_insert(void* db, const char* sql);

/// @return: <0-sql error, >0-delete rows, =0-no matched
DBCLIENT_API int db_delete(void* db, const char* sql);

/// @return: <0-sql error, >0-update rows, =0-no matched
DBCLIENT_API int db_update(void* db, const char* sql);

#endif /* !_dbclient_h_ */
