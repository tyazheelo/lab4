#include <gtest/gtest.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "../include/database.h"
#include "../include/config.h"
}

// Test fixture for database tests
class DatabaseTest : public ::testing::Test {
protected:
    sqlite3* db;

    void SetUp() override {
        // Use test database
        config_set_db_path("test.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));
    }

    void TearDown() override {
        if (db) {
            db_close(db);
        }
        // Cleanup test database
        remove("test.db");
    }
};

TEST_F(DatabaseTest, InitDatabase) {
    ASSERT_NE(db, nullptr);
}

TEST_F(DatabaseTest, ExecuteQuery) {
    const char* sql = "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);";
    ASSERT_EQ(SQLITE_OK, db_execute(db, sql));
}

TEST_F(DatabaseTest, TransactionBeginCommit) {
    ASSERT_EQ(SQLITE_OK, db_begin_transaction(db));

    const char* sql = "CREATE TABLE test_transaction (id INTEGER);";
    ASSERT_EQ(SQLITE_OK, db_execute(db, sql));

    ASSERT_EQ(SQLITE_OK, db_commit_transaction(db));

    // Verify table exists
    sqlite3_stmt* stmt;
    const char* check = "SELECT name FROM sqlite_master WHERE type='table' AND name='test_transaction';";
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db, check, -1, &stmt, NULL));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
}

TEST_F(DatabaseTest, TransactionRollback) {
    ASSERT_EQ(SQLITE_OK, db_begin_transaction(db));

    const char* sql = "CREATE TABLE test_rollback (id INTEGER);";
    ASSERT_EQ(SQLITE_OK, db_execute(db, sql));

    ASSERT_EQ(SQLITE_OK, db_rollback_transaction(db));

    // Verify table does NOT exist
    sqlite3_stmt* stmt;
    const char* check = "SELECT name FROM sqlite_master WHERE type='table' AND name='test_rollback';";
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db, check, -1, &stmt, NULL));
    ASSERT_EQ(SQLITE_DONE, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
}

TEST_F(DatabaseTest, LastInsertRowId) {
    const char* sql = "CREATE TABLE test_ids (id INTEGER PRIMARY KEY AUTOINCREMENT, value TEXT);";
    db_execute(db, sql);

    const char* insert = "INSERT INTO test_ids (value) VALUES ('test');";
    ASSERT_EQ(SQLITE_OK, db_execute(db, insert));

    long long id = db_last_insert_rowid(db);
    ASSERT_GT(id, 0);
}

TEST_F(DatabaseTest, ErrorLogging) {
    // This test just ensures the function doesn't crash
    db_log_error(db, SQLITE_ERROR, "Test error context");
}