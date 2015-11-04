#pragma once

#include <sqlite3/sqlite3.h>
#include "ESException.h"

class DBException : public ESException {};

// super simple RAII wrapper for sqlite3
// prepared statement, can be used just like an sqlite3_stmt* thanks to overloaded operator
class SQLPreparedStmt
{
public:
	SQLPreparedStmt(sqlite3* db, const char* stmt) : mDB(db), mStmt(NULL) {
		if(sqlite3_prepare_v2(db, stmt, strlen(stmt), &mStmt, NULL))
			throw DBException() << "Error creating prepared stmt \"" << stmt << "\".\n\t" << sqlite3_errmsg(db);
	}

	SQLPreparedStmt(sqlite3* db, const std::string& stmt) : SQLPreparedStmt(db, stmt.c_str()) {};

	int step() { return sqlite3_step(mStmt); }

	void step_expected(int expected) {
		int result = step();
		if(result != expected)
			throw DBException() << "Step failed (got " << result << ", expected " << expected << "!\n\t" << sqlite3_errmsg(mDB);
	}

	void reset() {
		if(sqlite3_reset(mStmt))
			throw DBException() << "Error resetting statement!\n\t" << sqlite3_errmsg(mDB);
	}

	~SQLPreparedStmt() {
		if(mStmt)
			sqlite3_finalize(mStmt);
	}

	operator sqlite3_stmt*() { return mStmt; }

private:
	sqlite3* mDB; // used for error messages
	sqlite3_stmt* mStmt;
};

// encapsulates a transaction that cannot outlive the lifetime of this object
class SQLTransaction
{
public:
	SQLTransaction(sqlite3* db) : mDB(db) {
		if(sqlite3_exec(mDB, "BEGIN TRANSACTION", NULL, NULL, NULL))
			throw DBException() << "Error beginning transaction.\n\t" << sqlite3_errmsg(mDB);
	}

	void commit() {
		if(!mDB)
			throw DBException() << "Transaction already committed!";

		if(sqlite3_exec(mDB, "COMMIT TRANSACTION", NULL, NULL, NULL))
			throw DBException() << "Error committing transaction.\n\t" << sqlite3_errmsg(mDB);

		mDB = NULL;
	}

	~SQLTransaction() {
		if(mDB)
			commit();
	}

private:
	sqlite3* mDB;
};
