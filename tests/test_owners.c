#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string.h>

extern "C" {
#include "../include/owners.h"
#include "../include/database.h"
#include "../include/auth.h"
#include "../include/config.h"
}

class OwnersTest : public ::testing::Test {
protected:
    sqlite3* db;
    int test_user_id;

    void SetUp() override {
        config_set_db_path("test_owners.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));

        auth_init(db);
        owners_init(db);

        // Create test user
        auth_create_user(db, NULL, "test_owner_user", "pass", ROLE_OWNER);

        sqlite3_stmt* stmt;
        const char* get_user = "SELECT id FROM USERS WHERE username='test_owner_user';";
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
        remove("test_owners.db");
    }
};

TEST_F(OwnersTest, CreateOwner) {
    ASSERT_TRUE(owners_create(db, "John", "Doe", "James", "123 Main St", "555-0100", test_user_id));
}

TEST_F(OwnersTest, ReadOwner) {
    owners_create(db, "Jane", "Smith", "Ann", "456 Oak Ave", "555-0200", test_user_id);

    Owner owner;
    ASSERT_TRUE(owners_read(db, 1, &owner));
    ASSERT_STREQ("Jane", owner.name);
    ASSERT_STREQ("Smith", owner.last_name);
    ASSERT_STREQ("Ann", owner.middle_name);
}

TEST_F(OwnersTest, ReadOwnerNotFound) {
    Owner owner;
    ASSERT_FALSE(owners_read(db, 999, &owner));
}

TEST_F(OwnersTest, UpdateOwner) {
    owners_create(db, "Bob", "Jones", "Lee", "789 Pine Rd", "555-0300", test_user_id);

    ASSERT_TRUE(owners_update(db, 1, "Robert", "Jones", "L.", "Updated Address", "555-9999"));

    Owner owner;
    owners_read(db, 1, &owner);
    ASSERT_STREQ("Robert", owner.name);
    ASSERT_STREQ("Updated Address", owner.address);
    ASSERT_STREQ("555-9999", owner.phone);
}

TEST_F(OwnersTest, DeleteOwner) {
    // Create separate user
    auth_create_user(db, NULL, "delete_owner_user", "pass", ROLE_OWNER);
    int new_user_id = 0;
    sqlite3_stmt* stmt;
    const char* get_user = "SELECT id FROM USERS WHERE username='delete_owner_user';";
    if (sqlite3_prepare_v2(db, get_user, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            new_user_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    owners_create(db, "Delete", "Me", NULL, "Addr", "555-0000", new_user_id);

    ASSERT_TRUE(owners_delete(db, 1));

    Owner owner;
    ASSERT_FALSE(owners_read(db, 1, &owner));
}

TEST_F(OwnersTest, GetOwnerByUserId) {
    owners_create(db, "Find", "Owner", NULL, "Addr", "555-1111", test_user_id);

    Owner owner;
    ASSERT_TRUE(owners_get_by_user_id(db, test_user_id, &owner));
    ASSERT_STREQ("Find", owner.name);
    ASSERT_STREQ("Owner", owner.last_name);
}

TEST_F(OwnersTest, GetAllOwners) {
    // Create another user
    auth_create_user(db, NULL, "owner2_user", "pass", ROLE_OWNER);
    int user2_id = 0;
    sqlite3_stmt* stmt;
    const char* get_user = "SELECT id FROM USERS WHERE username='owner2_user';";
    if (sqlite3_prepare_v2(db, get_user, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user2_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    owners_create(db, "Owner1", "One", NULL, "Addr1", "555-0001", test_user_id);
    owners_create(db, "Owner2", "Two", NULL, "Addr2", "555-0002", user2_id);

    Owner* owners = NULL;
    int count = 0;
    ASSERT_TRUE(owners_get_all(db, &owners, &count));
    ASSERT_EQ(2, count);

    owners_free(&owners, count);
}

TEST_F(OwnersTest, GetOwnerHorses) {
    horses_init(db);
    owners_create(db, "HorseOwner", "Test", NULL, "Addr", "555-7777", test_user_id);

    int owner_id = 1;

    // Create some horses
    horses_create(db, "HorseA", 5, 3, owner_id);
    horses_create(db, "HorseB", 4, 2, owner_id);

    Horse* horses = NULL;
    int count = 0;
    ASSERT_TRUE(owners_get_horses(db, owner_id, &horses, &count));
    ASSERT_EQ(2, count);

    horses_free(&horses, count);
}