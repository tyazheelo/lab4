#include "../include/database.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int db_init(sqlite3** db) {
    int rc = sqlite3_open(DB_PATH, db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
        return rc;
    }

    // Enable foreign keys
    char* err_msg = NULL;
    rc = sqlite3_exec(*db, "PRAGMA foreign_keys = ON;", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to enable foreign keys: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }

    return SQLITE_OK;
}

int db_close(sqlite3* db) {
    if (db) {
        return sqlite3_close(db);
    }
    return SQLITE_OK;
}

int db_execute(sqlite3* db, const char* sql) {
    char* err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    return rc;
}

long long db_last_insert_rowid(sqlite3* db) {
    return sqlite3_last_insert_rowid(db);
}

int db_begin_transaction(sqlite3* db) {
    return db_execute(db, "BEGIN TRANSACTION;");
}

int db_commit_transaction(sqlite3* db) {
    return db_execute(db, "COMMIT;");
}

int db_rollback_transaction(sqlite3* db) {
    return db_execute(db, "ROLLBACK;");
}

void db_log_error(sqlite3* db, int error_code, const char* context) {
    fprintf(stderr, "[ERROR] %s: code=%d, msg=%s\n",
        context, error_code, sqlite3_errmsg(db));
}