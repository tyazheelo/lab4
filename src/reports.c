#include "../include/reports.h"
#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

static int callback_report(void* data, int argc, char** argv, char** azColName) {
    ReportData* report = (ReportData*)data;

    // Reallocate for new row
    report->rows = (char**)realloc(report->rows, sizeof(char*) * (report->row_count + 1));

    // Build row string
    char* row_str = malloc(1024);
    row_str[0] = '\0';

    for (int i = 0; i < argc; i++) {
        char col[256];
        if (argv[i]) {
            snprintf(col, sizeof(col), "%s: %s | ", azColName[i], argv[i]);
        }
        else {
            snprintf(col, sizeof(col), "%s: NULL | ", azColName[i]);
        }
        strcat(row_str, col);
    }
    // Remove trailing " | "
    if (strlen(row_str) > 3) {
        row_str[strlen(row_str) - 3] = '\0';
    }

    report->rows[report->row_count] = row_str;
    report->row_count++;
    report->col_count = argc;

    return 0;
}

int reports_jockey_races(sqlite3* db, int jockey_id, ReportData* report) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT r.race_date, r.race_number, h.nickname as horse_name, rhj.position "
        "FROM RACES_HORSES_JOCKEYS rhj "
        "JOIN RACES r ON rhj.race_id = r.id "
        "JOIN HORSES h ON rhj.horse_id = h.id "
        "WHERE rhj.jockey_id = %d "
        "ORDER BY r.race_date DESC;", jockey_id);

    report->rows = NULL;
    report->row_count = 0;
    report->col_count = 0;

    return sqlite3_exec(db, sql, callback_report, report, NULL);
}

int reports_owner_horses_races(sqlite3* db, int owner_id, ReportData* report) {
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "SELECT h.nickname as horse_name, r.race_date, r.race_number, "
        "COALESCE(rhj.position, 0) as position "
        "FROM OWNERS o "
        "JOIN HORSES h ON o.id = h.owner_id "
        "LEFT JOIN RACES_HORSES_JOCKEYS rhj ON h.id = rhj.horse_id "
        "LEFT JOIN RACES r ON rhj.race_id = r.id "
        "WHERE o.id = %d "
        "ORDER BY h.nickname, r.race_date DESC;", owner_id);

    report->rows = NULL;
    report->row_count = 0;
    report->col_count = 0;

    return sqlite3_exec(db, sql, callback_report, report, NULL);
}

int reports_horse_most_wins(sqlite3* db, ReportData* report) {
    const char* sql =
        "SELECT h.id, h.nickname, h.age, h.experience_years, "
        "o.last_name || ' ' || o.name as owner_name, "
        "COUNT(*) as wins, "
        "GROUP_CONCAT(DISTINCT r.race_date) as race_dates, "
        "GROUP_CONCAT(DISTINCT j.last_name) as jockeys "
        "FROM HORSES h "
        "JOIN OWNERS o ON h.owner_id = o.id "
        "JOIN RACES_HORSES_JOCKEYS rhj ON h.id = rhj.horse_id "
        "JOIN RACES r ON rhj.race_id = r.id "
        "JOIN JOCKEYS j ON rhj.jockey_id = j.id "
        "WHERE rhj.position = 1 "
        "GROUP BY h.id "
        "ORDER BY wins DESC LIMIT 1;";

    report->rows = NULL;
    report->row_count = 0;
    report->col_count = 0;

    return sqlite3_exec(db, sql, callback_report, report, NULL);
}

int reports_jockey_most_participations(sqlite3* db, ReportData* report) {
    const char* sql =
        "SELECT j.id, j.last_name, j.experience_years, j.birth_year, j.address, "
        "COUNT(*) as total_races "
        "FROM JOCKEYS j "
        "JOIN RACES_HORSES_JOCKEYS rhj ON j.id = rhj.jockey_id "
        "GROUP BY j.id "
        "ORDER BY total_races DESC LIMIT 1;";

    report->rows = NULL;
    report->row_count = 0;
    report->col_count = 0;

    return sqlite3_exec(db, sql, callback_report, report, NULL);
}

int reports_races_by_period(sqlite3* db, const char* start_date, const char* end_date, ReportData* report) {
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "SELECT r.id, r.race_date, r.race_number, r.prize_fund, "
        "h.nickname as horse_name, j.last_name as jockey_name, rhj.position "
        "FROM RACES r "
        "JOIN RACES_HORSES_JOCKEYS rhj ON r.id = rhj.race_id "
        "JOIN HORSES h ON rhj.horse_id = h.id "
        "JOIN JOCKEYS j ON rhj.jockey_id = j.id "
        "WHERE r.race_date BETWEEN '%s' AND '%s' "
        "ORDER BY r.race_date, r.race_number, rhj.position;",
        start_date, end_date);

    report->rows = NULL;
    report->row_count = 0;
    report->col_count = 0;

    return sqlite3_exec(db, sql, callback_report, report, NULL);
}

int reports_jockey_earnings_by_period(sqlite3* db, int jockey_id, const char* start_date, const char* end_date, double* earnings) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT COALESCE(SUM(pd.prize_amount), 0) "
        "FROM PRIZE_DISTRIBUTION pd "
        "JOIN RACES_HORSES_JOCKEYS rhj ON pd.race_id = rhj.race_id AND pd.horse_id = rhj.horse_id "
        "JOIN RACES r ON pd.race_id = r.id "
        "WHERE rhj.jockey_id = %d AND r.race_date BETWEEN '%s' AND '%s';",
        jockey_id, start_date, end_date);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *earnings = sqlite3_column_double(stmt, 0);
    }
    else {
        *earnings = 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int reports_prize_distribution(sqlite3* db, int race_id, ReportData* report) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT h.nickname as horse_name, pd.position, pd.prize_amount, pd.distribution_date "
        "FROM PRIZE_DISTRIBUTION pd "
        "JOIN HORSES h ON pd.horse_id = h.id "
        "WHERE pd.race_id = %d "
        "ORDER BY pd.position;", race_id);

    report->rows = NULL;
    report->row_count = 0;
    report->col_count = 0;

    return sqlite3_exec(db, sql, callback_report, report, NULL);
}

void reports_free(ReportData* report) {
    if (report) {
        for (int i = 0; i < report->row_count; i++) {
            if (report->rows[i]) {
                free(report->rows[i]);
            }
        }
        if (report->rows) {
            free(report->rows);
        }
        report->rows = NULL;
        report->row_count = 0;
        report->col_count = 0;
    }
}