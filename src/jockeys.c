#include "../include/jockeys.h"
#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int jockeys_init(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS JOCKEYS ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "last_name VARCHAR(50) NOT NULL,"
        "experience_years INTEGER,"
        "birth_year INTEGER,"
        "address VARCHAR(255),"
        "user_id INTEGER NOT NULL UNIQUE,"
        "FOREIGN KEY (user_id) REFERENCES USERS(id)"
        ");";
    return db_execute(db, sql);
}

int jockeys_create(sqlite3* db, const char* last_name, int experience_years,
    int birth_year, const char* address, int user_id) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO JOCKEYS (last_name, experience_years, birth_year, address, user_id) "
        "VALUES ('%s', %d, %d, '%s', %d);",
        last_name, experience_years, birth_year, address, user_id);
    return (db_execute(db, sql) == SQLITE_OK);
}

int jockeys_read(sqlite3* db, int jockey_id, Jockey* jockey) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT id, last_name, experience_years, birth_year, address, user_id "
        "FROM JOCKEYS WHERE id = %d;", jockey_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jockey->id = sqlite3_column_int(stmt, 0);
        strncpy(jockey->last_name, (const char*)sqlite3_column_text(stmt, 1), sizeof(jockey->last_name) - 1);
        jockey->experience_years = sqlite3_column_int(stmt, 2);
        jockey->birth_year = sqlite3_column_int(stmt, 3);
        strncpy(jockey->address, (const char*)sqlite3_column_text(stmt, 4), sizeof(jockey->address) - 1);
        jockey->user_id = sqlite3_column_int(stmt, 5);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int jockeys_update(sqlite3* db, int jockey_id, const char* last_name, int experience_years,
    int birth_year, const char* address) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "UPDATE JOCKEYS SET last_name='%s', experience_years=%d, birth_year=%d, address='%s' "
        "WHERE id=%d;",
        last_name, experience_years, birth_year, address, jockey_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

int jockeys_delete(sqlite3* db, int jockey_id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM JOCKEYS WHERE id=%d;", jockey_id);
    int user_id = 0;

    // First get user_id to delete user as well
    char select_sql[128];
    snprintf(select_sql, sizeof(select_sql), "SELECT user_id FROM JOCKEYS WHERE id=%d;", jockey_id);
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    int rc = db_execute(db, sql);
    if (rc == SQLITE_OK && user_id > 0) {
        char delete_user[128];
        snprintf(delete_user, sizeof(delete_user), "DELETE FROM USERS WHERE id=%d;", user_id);
        db_execute(db, delete_user);
    }

    return (rc == SQLITE_OK && sqlite3_changes(db) > 0);
}

int jockeys_get_by_user_id(sqlite3* db, int user_id, Jockey* jockey) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT id, last_name, experience_years, birth_year, address, user_id "
        "FROM JOCKEYS WHERE user_id = %d;", user_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jockey->id = sqlite3_column_int(stmt, 0);
        strncpy(jockey->last_name, (const char*)sqlite3_column_text(stmt, 1), sizeof(jockey->last_name) - 1);
        jockey->experience_years = sqlite3_column_int(stmt, 2);
        jockey->birth_year = sqlite3_column_int(stmt, 3);
        strncpy(jockey->address, (const char*)sqlite3_column_text(stmt, 4), sizeof(jockey->address) - 1);
        jockey->user_id = sqlite3_column_int(stmt, 5);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

static int callback_collect_jockeys(void* data, int argc, char** argv, char** azColName) {
    Jockey** jockeys = (Jockey**)data;
    int* count = (int*)((void**)data)[1];
    Jockey* jockey = &(*jockeys)[*count];

    jockey->id = atoi(argv[0]);
    strncpy(jockey->last_name, argv[1], sizeof(jockey->last_name) - 1);
    jockey->experience_years = atoi(argv[2]);
    jockey->birth_year = atoi(argv[3]);
    if (argv[4]) {
        strncpy(jockey->address, argv[4], sizeof(jockey->address) - 1);
    }
    else {
        jockey->address[0] = '\0';
    }
    jockey->user_id = atoi(argv[5]);

    (*count)++;
    return 0;
}

int jockeys_get_all(sqlite3* db, Jockey** jockeys, int* count) {
    const char* sql = "SELECT id, last_name, experience_years, birth_year, address, user_id FROM JOCKEYS;";

    sqlite3_stmt* stmt;
    int row_count = 0;

    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM JOCKEYS;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *jockeys = NULL;
        *count = 0;
        return 1;
    }

    *jockeys = (Jockey*)malloc(sizeof(Jockey) * row_count);
    if (!*jockeys) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_jockeys, (void*)(&(*jockeys)), NULL);

    if (rc != SQLITE_OK) {
        free(*jockeys);
        *jockeys = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

int jockeys_get_most_participations(sqlite3* db, Jockey* jockey, int* participation_count) {
    const char* sql =
        "SELECT j.id, j.last_name, j.experience_years, j.birth_year, j.address, j.user_id, "
        "COUNT(*) as participations "
        "FROM JOCKEYS j "
        "JOIN RACES_HORSES_JOCKEYS rhj ON j.id = rhj.jockey_id "
        "GROUP BY j.id "
        "ORDER BY participations DESC LIMIT 1;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        jockey->id = sqlite3_column_int(stmt, 0);
        strncpy(jockey->last_name, (const char*)sqlite3_column_text(stmt, 1), sizeof(jockey->last_name) - 1);
        jockey->experience_years = sqlite3_column_int(stmt, 2);
        jockey->birth_year = sqlite3_column_int(stmt, 3);
        strncpy(jockey->address, (const char*)sqlite3_column_text(stmt, 4), sizeof(jockey->address) - 1);
        jockey->user_id = sqlite3_column_int(stmt, 5);
        *participation_count = sqlite3_column_int(stmt, 6);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int jockeys_get_race_history(sqlite3 *db, int jockey_id, char ***history, int *count) {
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT r.race_date, r.race_number, h.nickname, rhj.position "
             "FROM RACES_HORSES_JOCKEYS rhj "
             "JOIN RACES r ON rhj.race_id = r.id "
             "JOIN HORSES h ON rhj.horse_id = h.id "
             "WHERE rhj.jockey_id = %d "
             "ORDER BY r.race_date DESC;", jockey_id);
    
    sqlite3_stmt *stmt;
    int row_count = 0;
    
    // Count rows
    char count_sql[512];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    if (row_count == 0) {
        *history = NULL;
        *count = 0;
        return 1;
    }
    
    *history = (char**)malloc(sizeof(char*) * row_count);
    if (!*history) return 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(*history);
        return 0;
    }
    
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char line[512];
        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        int race_num = sqlite3_column_int(stmt, 1);
        const char *horse = (const char*)sqlite3_column_text(stmt, 2);
        int pos = sqlite3_column_int(stmt, 3);
        
        snprintf(line, sizeof(line), "Date: %s, Race #%d, Horse: %s, Position: %d",
                 date ? date : "unknown", race_num, horse ? horse : "unknown", pos);
        
        (*history)[*count] = (char*)malloc(strlen(line) + 1);
        if ((*history)[*count]) {
            strcpy((*history)[*count], line);
        }
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 1;
}

int jockeys_get_stats(sqlite3* db, int jockey_id, int* total_races, int* wins, int* top3) {
    const char* sql =
        "SELECT "
        "COUNT(*) as total_races, "
        "SUM(CASE WHEN position = 1 THEN 1 ELSE 0 END) as wins, "
        "SUM(CASE WHEN position <= 3 THEN 1 ELSE 0 END) as top3 "
        "FROM RACES_HORSES_JOCKEYS "
        "WHERE jockey_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, jockey_id);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *total_races = sqlite3_column_int(stmt, 0);
        *wins = sqlite3_column_int(stmt, 1);
        *top3 = sqlite3_column_int(stmt, 2);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int jockeys_get_earnings(sqlite3* db, int jockey_id, double* earnings) {
    const char* sql =
        "SELECT COALESCE(SUM(pd.prize_amount), 0) "
        "FROM PRIZE_DISTRIBUTION pd "
        "JOIN RACES_HORSES_JOCKEYS rhj ON pd.race_id = rhj.race_id AND pd.horse_id = rhj.horse_id "
        "WHERE rhj.jockey_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, jockey_id);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *earnings = sqlite3_column_double(stmt, 0);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

void jockeys_free(Jockey** jockeys, int count) {
    if (jockeys && *jockeys) {
        free(*jockeys);
        *jockeys = NULL;
    }
}
