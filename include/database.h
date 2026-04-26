#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

// Initialize database connection
int db_init(sqlite3** db);

// Close database connection
int db_close(sqlite3* db);

// Execute query without returning data
int db_execute(sqlite3* db, const char* sql);

// Get last inserted row ID
long long db_last_insert_rowid(sqlite3* db);

// Begin transaction
int db_begin_transaction(sqlite3* db);

// Commit transaction
int db_commit_transaction(sqlite3* db);

// Rollback transaction
int db_rollback_transaction(sqlite3* db);

// Error handling
void db_log_error(sqlite3* db, int error_code, const char* context);

#endif // DATABASE_H