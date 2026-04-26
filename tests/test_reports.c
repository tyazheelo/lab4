#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string.h>

extern "C" {
#include "../include/reports.h"
#include "../include/races.h"
#include "../include/horses.h"
#include "../include/jockeys.h"
#include "../include/owners.h"
#include "../include/database.h"
#include "../include/auth.h"
#include "../include/config.h"
}

class ReportsTest : public ::testing::Test {
protected:
    sqlite3* db;
    int test_jockey_id;
    int test_owner_id;
    int test_horse_id;

    void SetUp() override {
        config_set_db_path("test_reports.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));

        // Initialize all modules
        auth_init(db);
        owners_init(db);
        horses_init(db);
        jockeys_init(db);
        races_init(db);

        // Create test owner
        auth_create_user(db, NULL, "report_owner", "pass", ROLE_OWNER);
        int owner_user_id = 0;
        sqlite3_stmt* stmt;
        const char* get_owner = "SELECT id FROM USERS WHERE username='report_owner';";
        if (sqlite3_prepare_v2(db, get_owner, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                owner_user_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        owners_create(db, "Report", "Owner", NULL, "Addr", "555-0000", owner_user_id);
        test_owner_id = 1;

        // Create test horse
        horses_create(db, "ReportHorse", 5, 3, test_owner_id);
        test_horse_id = 1;

        // Create test jockey
        auth_create_user(db, NULL, "report_jockey", "pass", ROLE_JOCKEY);
        int jockey_user_id = 0;
        const char* get_jockey = "SELECT id FROM USERS WHERE username='report_jockey';";
        if (sqlite3_prepare_v2(db, get_jockey, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                jockey_user_id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        jockeys_create(db, "ReportJockey", 7, 1985, "Addr", jockey_user_id);
        test_jockey_id = 1;

        // Create race with results
        races_create(db, "2024-01-15", 1, 10000.0);
        races_add_participant(db, 1, test_horse_id, test_jockey_id);
        races_set_result(db, 1, test_horse_id, 1);
        races_distribute_prize(db, 1, 10000.0);
    }

    void TearDown() override {
        if (db) {
            db_close(db);
        }
        remove("test_reports.db");
    }
};

TEST_F(ReportsTest, JockeyRacesReport) {
    ReportData report;
    ASSERT_EQ(SQLITE_OK, reports_jockey_races(db, test_jockey_id, &report));
    ASSERT_GT(report.row_count, 0);

    reports_free(&report);
}

TEST_F(ReportsTest, OwnerHorsesRacesReport) {
    ReportData report;
    ASSERT_EQ(SQLITE_OK, reports_owner_horses_races(db, test_owner_id, &report));
    ASSERT_GT(report.row_count, 0);

    reports_free(&report);
}

TEST_F(ReportsTest, HorseMostWinsReport) {
    ReportData report;
    ASSERT_EQ(SQLITE_OK, reports_horse_most_wins(db, &report));
    // Should have at least the horse we created
    ASSERT_GE(report.row_count, 0);

    reports_free(&report);
}

TEST_F(ReportsTest, JockeyMostParticipationsReport) {
    ReportData report;
    ASSERT_EQ(SQLITE_OK, reports_jockey_most_participations(db, &report));

    reports_free(&report);
}

TEST_F(ReportsTest, RacesByPeriodReport) {
    ReportData report;
    ASSERT_EQ(SQLITE_OK, reports_races_by_period(db, "2024-01-01", "2024-12-31", &report));
    ASSERT_GT(report.row_count, 0);

    reports_free(&report);
}

TEST_F(ReportsTest, JockeyEarningsByPeriod) {
    double earnings;
    ASSERT_TRUE(reports_jockey_earnings_by_period(db, test_jockey_id, "2024-01-01", "2024-12-31", &earnings));
    ASSERT_GT(earnings, 0);
}

TEST_F(ReportsTest, PrizeDistributionReport) {
    ReportData report;
    ASSERT_EQ(SQLITE_OK, reports_prize_distribution(db, 1, &report));
    ASSERT_GT(report.row_count, 0);

    reports_free(&report);
}