#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string.h>

extern "C" {
#include "../include/races.h"
#include "../include/horses.h"
#include "../include/jockeys.h"
#include "../include/owners.h"
#include "../include/database.h"
#include "../include/auth.h"
#include "../include/config.h"
}

class RacesTest : public ::testing::Test {
protected:
    sqlite3* db;
    int test_horse_id;
    int test_jockey_id;
    int test_owner_id;

    void SetUp() override {
        config_set_db_path("test_races.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));

        // Initialize all modules
        auth_init(db);
        owners_init(db);
        horses_init(db);
        jockeys_init(db);
        races_init(db);

        // Create test owner
        auth_create_user(db, NULL, "race_owner", "pass", ROLE_OWNER);
        int owner_user_id = 0;
        sqlite3_stmt* stmt;
        const char* get_owner = "SELECT id FROM USERS WHERE username='race_owner';";
        if (sqlite3_prepare_v2(db, get_owner, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                owner_user_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        owners_create(db, "Race", "Owner", NULL, "Addr", "555-0000", owner_user_id);
        test_owner_id = 1;

        // Create test horse
        horses_create(db, "RaceHorse", 5, 3, test_owner_id);
        test_horse_id = 1;

        // Create test jockey
        auth_create_user(db, NULL, "race_jockey", "pass", ROLE_JOCKEY);
        int jockey_user_id = 0;
        const char* get_jockey = "SELECT id FROM USERS WHERE username='race_jockey';";
        if (sqlite3_prepare_v2(db, get_jockey, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                jockey_user_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        jockeys_create(db, "RaceJockey", 7, 1985, "Addr", jockey_user_id);
        test_jockey_id = 1;
    }

    void TearDown() override {
        if (db) {
            db_close(db);
        }
        remove("test_races.db");
    }
};

TEST_F(RacesTest, CreateRace) {
    ASSERT_TRUE(races_create(db, "2024-01-15", 1, 10000.0));
}

TEST_F(RacesTest, ReadRace) {
    races_create(db, "2024-01-15", 1, 10000.0);

    Race race;
    ASSERT_TRUE(races_read(db, 1, &race));
    ASSERT_STREQ("2024-01-15", race.race_date);
    ASSERT_EQ(1, race.race_number);
    ASSERT_DOUBLE_EQ(10000.0, race.prize_fund);
}

TEST_F(RacesTest, UpdateRace) {
    races_create(db, "2024-01-15", 1, 10000.0);

    ASSERT_TRUE(races_update(db, 1, "2024-01-16", 2, 15000.0));

    Race race;
    races_read(db, 1, &race);
    ASSERT_STREQ("2024-01-16", race.race_date);
    ASSERT_EQ(2, race.race_number);
    ASSERT_DOUBLE_EQ(15000.0, race.prize_fund);
}

TEST_F(RacesTest, DeleteRace) {
    races_create(db, "2024-01-15", 1, 10000.0);

    ASSERT_TRUE(races_delete(db, 1));

    Race race;
    ASSERT_FALSE(races_read(db, 1, &race));
}

TEST_F(RacesTest, AddParticipant) {
    races_create(db, "2024-01-15", 1, 10000.0);

    ASSERT_TRUE(races_add_participant(db, 1, test_horse_id, test_jockey_id));
}

TEST_F(RacesTest, AddParticipantInvalidHorse) {
    races_create(db, "2024-01-15", 1, 10000.0);

    ASSERT_FALSE(races_add_participant(db, 1, 999, test_jockey_id));
}

TEST_F(RacesTest, SetRaceResult) {
    races_create(db, "2024-01-15", 1, 10000.0);
    races_add_participant(db, 1, test_horse_id, test_jockey_id);

    ASSERT_TRUE(races_set_result(db, 1, test_horse_id, 1));
}

TEST_F(RacesTest, GetAllRaces) {
    races_create(db, "2024-01-15", 1, 10000.0);
    races_create(db, "2024-01-16", 2, 20000.0);

    Race* races = NULL;
    int count = 0;
    ASSERT_TRUE(races_get_all(db, &races, &count));
    ASSERT_EQ(2, count);

    races_free(&races, count);
}

TEST_F(RacesTest, GetRacesByDateRange) {
    races_create(db, "2024-01-10", 1, 10000.0);
    races_create(db, "2024-01-15", 2, 15000.0);
    races_create(db, "2024-01-20", 3, 20000.0);

    Race* races = NULL;
    int count = 0;
    ASSERT_TRUE(races_get_by_date_range(db, "2024-01-12", "2024-01-18", &races, &count));
    ASSERT_EQ(1, count);

    races_free(&races, count);
}

TEST_F(RacesTest, GetParticipants) {
    races_create(db, "2024-01-15", 1, 10000.0);
    races_add_participant(db, 1, test_horse_id, test_jockey_id);

    RaceParticipation* participants = NULL;
    int count = 0;
    ASSERT_TRUE(races_get_participants(db, 1, &participants, &count));
    ASSERT_EQ(1, count);

    races_free_participants(&participants, count);
}

TEST_F(RacesTest, GetWinners) {
    races_create(db, "2024-01-15", 1, 10000.0);
    races_add_participant(db, 1, test_horse_id, test_jockey_id);
    races_set_result(db, 1, test_horse_id, 1);

    RaceParticipation* winners = NULL;
    int count = 0;
    ASSERT_TRUE(races_get_winners(db, 1, &winners, &count));
    ASSERT_EQ(1, count);

    races_free_participants(&winners, count);
}

TEST_F(RacesTest, DistributePrize) {
    races_create(db, "2024-01-15", 1, 10000.0);
    races_add_participant(db, 1, test_horse_id, test_jockey_id);
    races_set_result(db, 1, test_horse_id, 1);

    ASSERT_TRUE(races_distribute_prize(db, 1, 10000.0));
}

TEST_F(RacesTest, ValidateParticipants) {
    ASSERT_TRUE(races_validate_participants(db, test_horse_id, test_jockey_id));
    ASSERT_FALSE(races_validate_participants(db, 999, test_jockey_id));
    ASSERT_FALSE(races_validate_participants(db, test_horse_id, 999));
}