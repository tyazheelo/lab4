#include "../include/horses.h"
#include "../include/database.h"
#include "../include/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int horses_init(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS HORSES ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nickname VARCHAR(50) NOT NULL,"
        "age INTEGER,"
        "experience_years INTEGER,"
        "owner_id INTEGER NOT NULL,"
        "FOREIGN KEY (owner_id) REFERENCES OWNERS(id)"
        ");";
    return db_execute(db, sql);
}

int horses_create(sqlite3* db, const char* nickname, int age, int experience_years, int owner_id) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO HORSES (nickname, age, experience_years, owner_id) "
        "VALUES ('%s', %d, %d, %d);",
        nickname, age, experience_years, owner_id);
    return (db_execute(db, sql) == SQLITE_OK);
}

int horses_read(sqlite3* db, int horse_id, Horse* horse) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT h.id, h.nickname, h.age, h.experience_years, h.owner_id, "
        "o.last_name || ' ' || o.name as owner_name "
        "FROM HORSES h "
        "LEFT JOIN OWNERS o ON h.owner_id = o.id "
        "WHERE h.id = %d;", horse_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        horse->id = sqlite3_column_int(stmt, 0);
        strncpy(horse->nickname, (const char*)sqlite3_column_text(stmt, 1), sizeof(horse->nickname) - 1);
        horse->age = sqlite3_column_int(stmt, 2);
        horse->experience_years = sqlite3_column_int(stmt, 3);
        horse->owner_id = sqlite3_column_int(stmt, 4);
        const char* owner_name = (const char*)sqlite3_column_text(stmt, 5);
        if (owner_name) {
            strncpy(horse->owner_name, owner_name, sizeof(horse->owner_name) - 1);
        }
        else {
            horse->owner_name[0] = '\0';
        }
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int horses_update(sqlite3* db, int horse_id, const char* nickname, int age, int experience_years) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "UPDATE HORSES SET nickname='%s', age=%d, experience_years=%d WHERE id=%d;",
        nickname, age, experience_years, horse_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

int horses_delete(sqlite3* db, int horse_id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM HORSES WHERE id=%d;", horse_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

static int callback_collect_horses(void* data, int argc, char** argv, char** azColName) {
    Horse** horses = (Horse**)data;
    int* count = (int*)((void**)data)[1];
    Horse* horse = &(*horses)[*count];

    horse->id = atoi(argv[0]);
    strncpy(horse->nickname, argv[1], sizeof(horse->nickname) - 1);
    horse->age = atoi(argv[2]);
    horse->experience_years = atoi(argv[3]);
    horse->owner_id = atoi(argv[4]);
    if (argv[5]) {
        strncpy(horse->owner_name, argv[5], sizeof(horse->owner_name) - 1);
    }
    else {
        horse->owner_name[0] = '\0';
    }

    (*count)++;
    return 0;
}

int horses_get_by_owner(sqlite3* db, int owner_id, Horse** horses, int* count) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT h.id, h.nickname, h.age, h.experience_years, h.owner_id, "
        "o.last_name || ' ' || o.name as owner_name "
        "FROM HORSES h "
        "LEFT JOIN OWNERS o ON h.owner_id = o.id "
        "WHERE h.owner_id = %d;", owner_id);

    // First count rows
    char count_sql[256];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    sqlite3_stmt* stmt;
    int row_count = 0;

    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *horses = NULL;
        *count = 0;
        return 1;
    }

    *horses = (Horse*)malloc(sizeof(Horse) * row_count);
    if (!*horses) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_horses, (void*)(&(*horses)), NULL);

    if (rc != SQLITE_OK) {
        free(*horses);
        *horses = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

int horses_get_all(sqlite3* db, Horse** horses, int* count) {
    const char* sql =
        "SELECT h.id, h.nickname, h.age, h.experience_years, h.owner_id, "
        "o.last_name || ' ' || o.name as owner_name "
        "FROM HORSES h "
        "LEFT JOIN OWNERS o ON h.owner_id = o.id;";

    sqlite3_stmt* stmt;
    int row_count = 0;

    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM HORSES;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *horses = NULL;
        *count = 0;
        return 1;
    }

    *horses = (Horse*)malloc(sizeof(Horse) * row_count);
    if (!*horses) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_horses, (void*)(&(*horses)), NULL);

    if (rc != SQLITE_OK) {
        free(*horses);
        *horses = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

int horses_get_most_wins(sqlite3* db, Horse* horse, int* win_count) {
    const char* sql =
        "SELECT h.id, h.nickname, h.age, h.experience_years, h.owner_id, "
        "COUNT(*) as wins "
        "FROM HORSES h "
        "JOIN RACES_HORSES_JOCKEYS rhj ON h.id = rhj.horse_id "
        "WHERE rhj.position = 1 "
        "GROUP BY h.id "
        "ORDER BY wins DESC LIMIT 1;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        horse->id = sqlite3_column_int(stmt, 0);
        strncpy(horse->nickname, (const char*)sqlite3_column_text(stmt, 1), sizeof(horse->nickname) - 1);
        horse->age = sqlite3_column_int(stmt, 2);
        horse->experience_years = sqlite3_column_int(stmt, 3);
        horse->owner_id = sqlite3_column_int(stmt, 4);
        *win_count = sqlite3_column_int(stmt, 5);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int horses_get_stats(sqlite3* db, int horse_id, int* total_races, int* wins, double* total_prize) {
    const char* sql =
        "SELECT "
        "COUNT(*) as total_races, "
        "SUM(CASE WHEN position = 1 THEN 1 ELSE 0 END) as wins, "
        "COALESCE(SUM(pd.prize_amount), 0) as total_prize "
        "FROM RACES_HORSES_JOCKEYS rhj "
        "LEFT JOIN PRIZE_DISTRIBUTION pd ON rhj.race_id = pd.race_id AND rhj.horse_id = pd.horse_id "
        "WHERE rhj.horse_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, horse_id);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *total_races = sqlite3_column_int(stmt, 0);
        *wins = sqlite3_column_int(stmt, 1);
        *total_prize = sqlite3_column_double(stmt, 2);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int horses_search(sqlite3* db, const char* nickname, Horse** horses, int* count) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT h.id, h.nickname, h.age, h.experience_years, h.owner_id, "
        "o.last_name || ' ' || o.name as owner_name "
        "FROM HORSES h "
        "LEFT JOIN OWNERS o ON h.owner_id = o.id "
        "WHERE h.nickname LIKE '%%%s%%';", nickname);

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
        *horses = NULL;
        *count = 0;
        return 1;
    }

    *horses = (Horse*)malloc(sizeof(Horse) * row_count);
    if (!*horses) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_horses, (void*)(&(*horses)), NULL);

    if (rc != SQLITE_OK) {
        free(*horses);
        *horses = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

void horses_free(Horse** horses, int count) {
    if (horses && *horses) {
        free(*horses);
        *horses = NULL;
    }
}