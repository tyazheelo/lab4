#include "../include/races.h"
#include "../include/database.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int races_init(sqlite3* db) {
    const char* sql_races =
        "CREATE TABLE IF NOT EXISTS RACES ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "race_date DATETIME NOT NULL,"
        "race_number INTEGER NOT NULL,"
        "prize_fund DECIMAL(10,2) NOT NULL"
        ");";

    const char* sql_rhj =
        "CREATE TABLE IF NOT EXISTS RACES_HORSES_JOCKEYS ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "race_id INTEGER NOT NULL,"
        "horse_id INTEGER NOT NULL,"
        "jockey_id INTEGER NOT NULL,"
        "position INTEGER,"
        "UNIQUE(race_id, horse_id),"
        "FOREIGN KEY (race_id) REFERENCES RACES(id),"
        "FOREIGN KEY (horse_id) REFERENCES HORSES(id),"
        "FOREIGN KEY (jockey_id) REFERENCES JOCKEYS(id)"
        ");";

    const char* sql_prize =
        "CREATE TABLE IF NOT EXISTS PRIZE_DISTRIBUTION ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "race_id INTEGER NOT NULL,"
        "horse_id INTEGER NOT NULL,"
        "position INTEGER NOT NULL CHECK(position IN (1,2,3)),"
        "prize_amount DECIMAL(10,2) NOT NULL,"
        "distribution_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (race_id) REFERENCES RACES(id),"
        "FOREIGN KEY (horse_id) REFERENCES HORSES(id)"
        ");";

    int rc = db_execute(db, sql_races);
    if (rc != SQLITE_OK) return rc;
    rc = db_execute(db, sql_rhj);
    if (rc != SQLITE_OK) return rc;
    return db_execute(db, sql_prize);
}

int races_create(sqlite3* db, const char* race_date, int race_number, double prize_fund) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO RACES (race_date, race_number, prize_fund) VALUES ('%s', %d, %.2f);",
        race_date, race_number, prize_fund);
    return (db_execute(db, sql) == SQLITE_OK);
}

int races_read(sqlite3* db, int race_id, Race* race) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, race_date, race_number, prize_fund FROM RACES WHERE id=%d;", race_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        race->id = sqlite3_column_int(stmt, 0);
        strncpy(race->race_date, (const char*)sqlite3_column_text(stmt, 1), sizeof(race->race_date) - 1);
        race->race_number = sqlite3_column_int(stmt, 2);
        race->prize_fund = sqlite3_column_double(stmt, 3);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int races_update(sqlite3* db, int race_id, const char* race_date, int race_number, double prize_fund) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "UPDATE RACES SET race_date='%s', race_number=%d, prize_fund=%.2f WHERE id=%d;",
        race_date, race_number, prize_fund, race_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

int races_delete(sqlite3* db, int race_id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM RACES WHERE id=%d;", race_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

int races_add_participant(sqlite3* db, int race_id, int horse_id, int jockey_id) {
    // First check if horse and jockey exist
    if (!races_validate_participants(db, horse_id, jockey_id)) {
        return 0;
    }

    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO RACES_HORSES_JOCKEYS (race_id, horse_id, jockey_id) VALUES (%d, %d, %d);",
        race_id, horse_id, jockey_id);
    return (db_execute(db, sql) == SQLITE_OK);
}

int races_set_result(sqlite3* db, int race_id, int horse_id, int position) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "UPDATE RACES_HORSES_JOCKEYS SET position=%d WHERE race_id=%d AND horse_id=%d;",
        position, race_id, horse_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

static int callback_collect_races(void* data, int argc, char** argv, char** azColName) {
    Race** races = (Race**)data;
    int* count = (int*)((void**)data)[1];
    Race* race = &(*races)[*count];

    race->id = atoi(argv[0]);
    strncpy(race->race_date, argv[1], sizeof(race->race_date) - 1);
    race->race_number = atoi(argv[2]);
    race->prize_fund = atof(argv[3]);

    (*count)++;
    return 0;
}

int races_get_all(sqlite3* db, Race** races, int* count) {
    const char* sql = "SELECT id, race_date, race_number, prize_fund FROM RACES ORDER BY race_date DESC;";

    sqlite3_stmt* stmt;
    int row_count = 0;

    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM RACES;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *races = NULL;
        *count = 0;
        return 1;
    }

    *races = (Race*)malloc(sizeof(Race) * row_count);
    if (!*races) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_races, (void*)(&(*races)), NULL);

    if (rc != SQLITE_OK) {
        free(*races);
        *races = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

int races_get_by_date_range(sqlite3* db, const char* start_date, const char* end_date, Race** races, int* count) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT id, race_date, race_number, prize_fund FROM RACES "
        "WHERE race_date BETWEEN '%s' AND '%s' ORDER BY race_date DESC;",
        start_date, end_date);

    sqlite3_stmt* stmt;
    char count_sql[512];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    int row_count = 0;

    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *races = NULL;
        *count = 0;
        return 1;
    }

    *races = (Race*)malloc(sizeof(Race) * row_count);
    if (!*races) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_races, (void*)(&(*races)), NULL);

    if (rc != SQLITE_OK) {
        free(*races);
        *races = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

int races_get_participants(sqlite3* db, int race_id, RaceParticipation** participants, int* count) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT rhj.race_id, rhj.horse_id, rhj.jockey_id, rhj.position, "
        "h.nickname as horse_name, j.last_name as jockey_name "
        "FROM RACES_HORSES_JOCKEYS rhj "
        "JOIN HORSES h ON rhj.horse_id = h.id "
        "JOIN JOCKEYS j ON rhj.jockey_id = j.id "
        "WHERE rhj.race_id = %d;", race_id);

    sqlite3_stmt* stmt;
    int row_count = 0;

    char count_sql[512];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *participants = NULL;
        *count = 0;
        return 1;
    }

    *participants = (RaceParticipation*)malloc(sizeof(RaceParticipation) * row_count);
    if (!*participants) return 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(*participants);
        return 0;
    }

    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RaceParticipation* p = &(*participants)[*count];
        p->race_id = sqlite3_column_int(stmt, 0);
        p->horse_id = sqlite3_column_int(stmt, 1);
        p->jockey_id = sqlite3_column_int(stmt, 2);
        p->position = sqlite3_column_int(stmt, 3);
        strncpy(p->horse_nickname, (const char*)sqlite3_column_text(stmt, 4), sizeof(p->horse_nickname) - 1);
        strncpy(p->jockey_name, (const char*)sqlite3_column_text(stmt, 5), sizeof(p->jockey_name) - 1);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int races_get_winners(sqlite3* db, int race_id, RaceParticipation** winners, int* count) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT rhj.race_id, rhj.horse_id, rhj.jockey_id, rhj.position, "
        "h.nickname as horse_name, j.last_name as jockey_name "
        "FROM RACES_HORSES_JOCKEYS rhj "
        "JOIN HORSES h ON rhj.horse_id = h.id "
        "JOIN JOCKEYS j ON rhj.jockey_id = j.id "
        "WHERE rhj.race_id = %d AND rhj.position IN (1,2,3) "
        "ORDER BY rhj.position;", race_id);

    sqlite3_stmt* stmt;
    int row_count = 0;

    char count_sql[512];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *winners = NULL;
        *count = 0;
        return 1;
    }

    *winners = (RaceParticipation*)malloc(sizeof(RaceParticipation) * row_count);
    if (!*winners) return 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(*winners);
        return 0;
    }

    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RaceParticipation* p = &(*winners)[*count];
        p->race_id = sqlite3_column_int(stmt, 0);
        p->horse_id = sqlite3_column_int(stmt, 1);
        p->jockey_id = sqlite3_column_int(stmt, 2);
        p->position = sqlite3_column_int(stmt, 3);
        strncpy(p->horse_nickname, (const char*)sqlite3_column_text(stmt, 4), sizeof(p->horse_nickname) - 1);
        strncpy(p->jockey_name, (const char*)sqlite3_column_text(stmt, 5), sizeof(p->jockey_name) - 1);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int races_distribute_prize(sqlite3* db, int race_id, double prize_fund) {
    // Get the winners
    RaceParticipation* winners = NULL;
    int winner_count = 0;

    if (!races_get_winners(db, race_id, &winners, &winner_count)) {
        return 0;
    }

    if (winner_count == 0) {
        races_free_participants(&winners, winner_count);
        return 0;
    }

    double amounts[3] = { 0 };
    amounts[0] = prize_fund * PRIZE_FIRST;
    amounts[1] = prize_fund * PRIZE_SECOND;
    amounts[2] = prize_fund * PRIZE_THIRD;

    db_begin_transaction(db);

    for (int i = 0; i < winner_count && i < 3; i++) {
        char sql[512];
        snprintf(sql, sizeof(sql),
            "INSERT INTO PRIZE_DISTRIBUTION (race_id, horse_id, position, prize_amount) "
            "VALUES (%d, %d, %d, %.2f);",
            race_id, winners[i].horse_id, winners[i].position, amounts[i]);
        db_execute(db, sql);
    }

    db_commit_transaction(db);
    races_free_participants(&winners, winner_count);

    return 1;
}

int races_get_all_with_details(sqlite3* db, char** report, int* count) {
    const char* sql =
        "SELECT r.id, r.race_date, r.race_number, r.prize_fund, "
        "COUNT(DISTINCT rhj.horse_id) as participants "
        "FROM RACES r "
        "LEFT JOIN RACES_HORSES_JOCKEYS rhj ON r.id = rhj.race_id "
        "GROUP BY r.id "
        "ORDER BY r.race_date DESC;";

    sqlite3_stmt* stmt;
    int row_count = 0;

    char count_sql[512];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *report = NULL;
        *count = 0;
        return 1;
    }

    *report = (char**)malloc(sizeof(char*) * row_count);
    if (!*report) return 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(*report);
        return 0;
    }

    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char line[512];
        int race_id = sqlite3_column_int(stmt, 0);
        const char* date = (const char*)sqlite3_column_text(stmt, 1);
        int race_num = sqlite3_column_int(stmt, 2);
        double prize = sqlite3_column_double(stmt, 3);
        int participants = sqlite3_column_int(stmt, 4);

        snprintf(line, sizeof(line), "Race #%d: Date=%s, Prize=%.2f, Participants=%d",
            race_num, date ? date : "unknown", prize, participants);

        (*report)[*count] = (char*)malloc(strlen(line) + 1);
        if ((*report)[*count]) {
            strcpy((*report)[*count], line);
        }
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int races_validate_participants(sqlite3* db, int horse_id, int jockey_id) {
    sqlite3_stmt* stmt;
    int horse_exists = 0, jockey_exists = 0;

    const char* horse_sql = "SELECT 1 FROM HORSES WHERE id = ?;";
    if (sqlite3_prepare_v2(db, horse_sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, horse_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            horse_exists = 1;
        }
        sqlite3_finalize(stmt);
    }

    const char* jockey_sql = "SELECT 1 FROM JOCKEYS WHERE id = ?;";
    if (sqlite3_prepare_v2(db, jockey_sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, jockey_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            jockey_exists = 1;
        }
        sqlite3_finalize(stmt);
    }

    return (horse_exists && jockey_exists);
}

void races_free(Race** races, int count) {
    if (races && *races) {
        free(*races);
        *races = NULL;
    }
}

void races_free_participants(RaceParticipation** participants, int count) {
    if (participants && *participants) {
        free(*participants);
        *participants = NULL;
    }
}