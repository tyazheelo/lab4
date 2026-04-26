#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string.h>

extern "C" {
#include "../include/horses.h"
#include "../include/owners.h"
#include "../include/database.h"
#include "../include/auth.h"
#include "../include/config.h"
}

class HorsesTest : public ::testing::Test {
protected:
    sqlite3* db;
    int test_owner_id;

    void SetUp() override {
        config_set_db_path("test_horses.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));

        owners_init(db);
        horses_init(db);

        // Create test owner
        auth_init(db);
        auth_create_user(db, NULL, "test_owner", "pass", ROLE_OWNER);

        sqlite3_stmt* stmt;
        const char* get_user = "SELECT id FROM USERS WHERE username='test_owner';";
        if (sqlite3_prepare_v2(db, get_user, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int user_id = sqlite3_column_int(stmt, 0);
                owners_create(db, "John", "Doe", NULL, "123 Main St", "555-1234", user_id);
            }
            sqlite3_finalize(stmt);
        }

        // Get owner id
        const char* get_owner = "SELECT id FROM OWNERS WHERE last_name='Doe';";
        if (sqlite3_prepare_v2(db, get_owner, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                test_owner_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
    }

    void TearDown() override {
        if (db) {
            db_close(db);
        }
        remove("test_horses.db");
    }
};

TEST_F(HorsesTest, CreateHorse) {
    ASSERT_TRUE(horses_create(db, "Thunder", 5, 3, test_owner_id));
}

TEST_F(HorsesTest, ReadHorse) {
    horses_create(db, "Lightning", 4, 2, test_owner_id);

    Horse horse;
    ASSERT_TRUE(horses_read(db, 1, &horse));
    ASSERT_STREQ("Lightning", horse.nickname);
    ASSERT_EQ(4, horse.age);
    ASSERT_EQ(2, horse.experience_years);
    ASSERT_EQ(test_owner_id, horse.owner_id);
}

TEST_F(HorsesTest, ReadHorseNotFound) {
    Horse horse;
    ASSERT_FALSE(horses_read(db, 999, &horse));
}

TEST_F(HorsesTest, UpdateHorse) {
    horses_create(db, "Spirit", 6, 4, test_owner_id);

    ASSERT_TRUE(horses_update(db, 1, "Spirit Updated", 7, 5));

    Horse horse;
    horses_read(db, 1, &horse);
    ASSERT_STREQ("Spirit Updated", horse.nickname);
    ASSERT_EQ(7, horse.age);
    ASSERT_EQ(5, horse.experience_years);
}

TEST_F(HorsesTest, UpdateHorseNotFound) {
    ASSERT_FALSE(horses_update(db, 999, "Ghost", 5, 3));
}

TEST_F(HorsesTest, DeleteHorse) {
    horses_create(db, "DeleteMe", 3, 1, test_owner_id);

    ASSERT_TRUE(horses_delete(db, 1));

    Horse horse;
    ASSERT_FALSE(horses_read(db, 1, &horse));
}

TEST_F(HorsesTest, DeleteHorseNotFound) {
    ASSERT_FALSE(horses_delete(db, 999));
}

TEST_F(HorsesTest, GetHorsesByOwner) {
    horses_create(db, "Horse1", 4, 2, test_owner_id);
    horses_create(db, "Horse2", 5, 3, test_owner_id);
    horses_create(db, "Horse3", 6, 4, test_owner_id);

    Horse* horses = NULL;
    int count = 0;
    ASSERT_TRUE(horses_get_by_owner(db, test_owner_id, &horses, &count));
    ASSERT_EQ(3, count);

    horses_free(&horses, count);
}

TEST_F(HorsesTest, GetHorsesByOwnerEmpty) {
    Horse* horses = NULL;
    int count = 0;
    ASSERT_TRUE(horses_get_by_owner(db, 999, &horses, &count));
    ASSERT_EQ(0, count);
    ASSERT_EQ(NULL, horses);
}

TEST_F(HorsesTest, GetAllHorses) {
    horses_create(db, "All1", 4, 2, test_owner_id);
    horses_create(db, "All2", 5, 3, test_owner_id);

    Horse* horses = NULL;
    int count = 0;
    ASSERT_TRUE(horses_get_all(db, &horses, &count));
    ASSERT_EQ(2, count);

    horses_free(&horses, count);
}

TEST_F(HorsesTest, SearchHorses) {
    horses_create(db, "FastHorse", 4, 2, test_owner_id);
    horses_create(db, "SlowHorse", 5, 3, test_owner_id);
    horses_create(db, "Different", 6, 4, test_owner_id);

    Horse* horses = NULL;
    int count = 0;
    ASSERT_TRUE(horses_search(db, "Horse", &horses, &count));
    ASSERT_EQ(2, count);

    horses_free(&horses, count);
}

TEST_F(HorsesTest, GetHorseStats) {
    // Need races table for this test
    races_init(db);

    horses_create(db, "StatsHorse", 4, 2, test_owner_id);
    int horse_id = 1;

    int total_races, wins;
    double prize;
    ASSERT_TRUE(horses_get_stats(db, horse_id, &total_races, &wins, &prize));
    // Initially should be zero
    ASSERT_EQ(0, total_races);
    ASSERT_EQ(0, wins);
    ASSERT_DOUBLE_EQ(0.0, prize);
}