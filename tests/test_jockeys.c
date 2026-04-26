#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string.h>

extern "C" {
#include "../include/jockeys.h"
#include "../include/database.h"
#include "../include/auth.h"
#include "../include/config.h"
}

class JockeysTest : public ::testing::Test {
protected:
    sqlite3* db;
    int test_user_id;

    void SetUp() override {
        config_set_db_path("test_jockeys.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));

        auth_init(db);
        jockeys_init(db);

        // Create test user
        auth_create_user(db, NULL, "test_jockey_user", "pass", ROLE_JOCKEY);

        sqlite3_stmt* stmt;
        const char* get_user = "SELECT id FROM USERS WHERE username='test_jockey_user';";
        if (sqlite3_prepare_v2(db, get_user, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                test_user_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
    }

    void TearDown() override {
        if (db) {
            db_close(db);
        }
        remove("test_jockeys.db");
    }
};

TEST_F(JockeysTest, CreateJockey) {
    ASSERT_TRUE(jockeys_create(db, "Smith", 5, 1990, "123 Race St", test_user_id));
}

TEST_F(JockeysTest, ReadJockey) {
    jockeys_create(db, "Johnson", 8, 1985, "456 Track Ave", test_user_id);

    Jockey jockey;
    ASSERT_TRUE(jockeys_read(db, 1, &jockey));
    ASSERT_STREQ("Johnson", jockey.last_name);
    ASSERT_EQ(8, jockey.experience_years);
    ASSERT_EQ(1985, jockey.birth_year);
}

TEST_F(JockeysTest, ReadJockeyNotFound) {
    Jockey jockey;
    ASSERT_FALSE(jockeys_read(db, 999, &jockey));
}

TEST_F(JockeysTest, UpdateJockey) {
    jockeys_create(db, "Williams", 3, 1995, "789 Derby Ln", test_user_id);

    ASSERT_TRUE(jockeys_update(db, 1, "Williams Updated", 4, 1994, "Updated Address"));

    Jockey jockey;
    jockeys_read(db, 1, &jockey);
    ASSERT_STREQ("Williams Updated", jockey.last_name);
    ASSERT_EQ(4, jockey.experience_years);
    ASSERT_EQ(1994, jockey.birth_year);
    ASSERT_STREQ("Updated Address", jockey.address);
}

TEST_F(JockeysTest, DeleteJockey) {
    // Create a separate user for this jockey
    auth_create_user(db, NULL, "delete_jockey_user", "pass", ROLE_JOCKEY);
    sqlite3_stmt* stmt;
    int new_user_id = 0;
    const char* get_user = "SELECT id FROM USERS WHERE username='delete_jockey_user';";
    if (sqlite3_prepare_v2(db, get_user, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            new_user_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    jockeys_create(db, "DeleteMe", 2, 2000, "Address", new_user_id);

    ASSERT_TRUE(jockeys_delete(db, 1));

    Jockey jockey;
    ASSERT_FALSE(jockeys_read(db, 1, &jockey));
}

TEST_F(JockeysTest, GetJockeyByUserId) {
    jockeys_create(db, "FindMe", 7, 1988, "Find Address", test_user_id);

    Jockey jockey;
    ASSERT_TRUE(jockeys_get_by_user_id(db, test_user_id, &jockey));
    ASSERT_STREQ("FindMe", jockey.last_name);
}

TEST_F(JockeysTest, GetAllJockeys) {
    // Create another user
    auth_create_user(db, NULL, "jockey2_user", "pass", ROLE_JOCKEY);
    int user2_id = 0;
    sqlite3_stmt* stmt;
    const char* get_user = "SELECT id FROM USERS WHERE username='jockey2_user';";
    if (sqlite3_prepare_v2(db, get_user, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user2_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    jockeys_create(db, "Jockey1", 5, 1990, "Addr1", test_user_id);
    jockeys_create(db, "Jockey2", 3, 1992, "Addr2", user2_id);

    Jockey* jockeys = NULL;
    int count = 0;
    ASSERT_TRUE(jockeys_get_all(db, &jockeys, &count));
    ASSERT_EQ(2, count);

    jockeys_free(&jockeys, count);
}

TEST_F(JockeysTest, GetJockeyStats) {
    races_init(db);
    jockeys_create(db, "StatsJockey", 5, 1990, "Addr", test_user_id);

    int total_races, wins, top3;
    ASSERT_TRUE(jockeys_get_stats(db, 1, &total_races, &wins, &top3));
    // Initially should be zero
    ASSERT_EQ(0, total_races);
    ASSERT_EQ(0, wins);
    ASSERT_EQ(0, top3);
}

TEST_F(JockeysTest, GetJockeyEarnings) {
    jockeys_create(db, "EarnJockey", 5, 1990, "Addr", test_user_id);

    double earnings;
    ASSERT_TRUE(jockeys_get_earnings(db, 1, &earnings));
    ASSERT_DOUBLE_EQ(0.0, earnings);
}