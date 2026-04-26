#include "../include/owners.h"
#include "../include/horses.h"
#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int owners_init(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS OWNERS ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name VARCHAR(30),"
        "last_name VARCHAR(30),"
        "middle_name VARCHAR(30),"
        "address VARCHAR(255),"
        "phone VARCHAR(20),"
        "user_id INTEGER NOT NULL UNIQUE,"
        "FOREIGN KEY (user_id) REFERENCES USERS(id)"
        ");";
    return db_execute(db, sql);
}

int owners_create(sqlite3* db, const char* name, const char* last_name, const char* middle_name,
    const char* address, const char* phone, int user_id) {
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "INSERT INTO OWNERS (name, last_name, middle_name, address, phone, user_id) "
        "VALUES ('%s', '%s', '%s', '%s', '%s', %d);",
        name ? name : "", last_name ? last_name : "", middle_name ? middle_name : "",
        address ? address : "", phone ? phone : "", user_id);
    return (db_execute(db, sql) == SQLITE_OK);
}

int owners_read(sqlite3* db, int owner_id, Owner* owner) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT id, name, last_name, middle_name, address, phone, user_id "
        "FROM OWNERS WHERE id = %d;", owner_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        owner->id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        if (name) strncpy(owner->name, name, sizeof(owner->name) - 1);
        else owner->name[0] = '\0';

        const char* lname = (const char*)sqlite3_column_text(stmt, 2);
        if (lname) strncpy(owner->last_name, lname, sizeof(owner->last_name) - 1);
        else owner->last_name[0] = '\0';

        const char* mname = (const char*)sqlite3_column_text(stmt, 3);
        if (mname) strncpy(owner->middle_name, mname, sizeof(owner->middle_name) - 1);
        else owner->middle_name[0] = '\0';

        const char* addr = (const char*)sqlite3_column_text(stmt, 4);
        if (addr) strncpy(owner->address, addr, sizeof(owner->address) - 1);
        else owner->address[0] = '\0';

        const char* phone = (const char*)sqlite3_column_text(stmt, 5);
        if (phone) strncpy(owner->phone, phone, sizeof(owner->phone) - 1);
        else owner->phone[0] = '\0';

        owner->user_id = sqlite3_column_int(stmt, 6);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

int owners_update(sqlite3* db, int owner_id, const char* name, const char* last_name,
    const char* middle_name, const char* address, const char* phone) {
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "UPDATE OWNERS SET name='%s', last_name='%s', middle_name='%s', "
        "address='%s', phone='%s' WHERE id=%d;",
        name ? name : "", last_name ? last_name : "", middle_name ? middle_name : "",
        address ? address : "", phone ? phone : "", owner_id);
    return (db_execute(db, sql) == SQLITE_OK && sqlite3_changes(db) > 0);
}

int owners_delete(sqlite3* db, int owner_id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM OWNERS WHERE id=%d;", owner_id);
    int user_id = 0;

    // First get user_id to delete user as well
    char select_sql[128];
    snprintf(select_sql, sizeof(select_sql), "SELECT user_id FROM OWNERS WHERE id=%d;", owner_id);
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

int owners_get_by_user_id(sqlite3* db, int user_id, Owner* owner) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT id, name, last_name, middle_name, address, phone, user_id "
        "FROM OWNERS WHERE user_id = %d;", user_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        owner->id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        if (name) strncpy(owner->name, name, sizeof(owner->name) - 1);
        else owner->name[0] = '\0';

        const char* lname = (const char*)sqlite3_column_text(stmt, 2);
        if (lname) strncpy(owner->last_name, lname, sizeof(owner->last_name) - 1);
        else owner->last_name[0] = '\0';

        const char* mname = (const char*)sqlite3_column_text(stmt, 3);
        if (mname) strncpy(owner->middle_name, mname, sizeof(owner->middle_name) - 1);
        else owner->middle_name[0] = '\0';

        const char* addr = (const char*)sqlite3_column_text(stmt, 4);
        if (addr) strncpy(owner->address, addr, sizeof(owner->address) - 1);
        else owner->address[0] = '\0';

        const char* phone = (const char*)sqlite3_column_text(stmt, 5);
        if (phone) strncpy(owner->phone, phone, sizeof(owner->phone) - 1);
        else owner->phone[0] = '\0';

        owner->user_id = sqlite3_column_int(stmt, 6);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}

static int callback_collect_owners(void* data, int argc, char** argv, char** azColName) {
    Owner** owners = (Owner**)data;
    int* count = (int*)((void**)data)[1];
    Owner* owner = &(*owners)[*count];

    owner->id = atoi(argv[0]);
    if (argv[1]) strncpy(owner->name, argv[1], sizeof(owner->name) - 1);
    else owner->name[0] = '\0';
    if (argv[2]) strncpy(owner->last_name, argv[2], sizeof(owner->last_name) - 1);
    else owner->last_name[0] = '\0';
    if (argv[3]) strncpy(owner->middle_name, argv[3], sizeof(owner->middle_name) - 1);
    else owner->middle_name[0] = '\0';
    if (argv[4]) strncpy(owner->address, argv[4], sizeof(owner->address) - 1);
    else owner->address[0] = '\0';
    if (argv[5]) strncpy(owner->phone, argv[5], sizeof(owner->phone) - 1);
    else owner->phone[0] = '\0';
    owner->user_id = atoi(argv[6]);

    (*count)++;
    return 0;
}

int owners_get_all(sqlite3* db, Owner** owners, int* count) {
    const char* sql = "SELECT id, name, last_name, middle_name, address, phone, user_id FROM OWNERS;";

    sqlite3_stmt* stmt;
    int row_count = 0;

    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM OWNERS;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *owners = NULL;
        *count = 0;
        return 1;
    }

    *owners = (Owner*)malloc(sizeof(Owner) * row_count);
    if (!*owners) return 0;

    *count = 0;
    int rc = sqlite3_exec(db, sql, callback_collect_owners, (void*)(&(*owners)), NULL);

    if (rc != SQLITE_OK) {
        free(*owners);
        *owners = NULL;
        *count = 0;
        return 0;
    }

    return 1;
}

int owners_get_with_horses_and_races(sqlite3* db, int owner_id, char** result, int* count) {
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "SELECT h.nickname, r.race_date, r.race_number, rhj.position "
        "FROM OWNERS o "
        "JOIN HORSES h ON o.id = h.owner_id "
        "LEFT JOIN RACES_HORSES_JOCKEYS rhj ON h.id = rhj.horse_id "
        "LEFT JOIN RACES r ON rhj.race_id = r.id "
        "WHERE o.id = %d "
        "ORDER BY h.nickname, r.race_date;", owner_id);

    sqlite3_stmt* stmt;
    int row_count = 0;

    char count_sql[1024];
    snprintf(count_sql, sizeof(count_sql), "SELECT COUNT(*) FROM (%s);", sql);
    if (sqlite3_prepare_v2(db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            row_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (row_count == 0) {
        *result = NULL;
        *count = 0;
        return 1;
    }

    *result = (char**)malloc(sizeof(char*) * row_count);
    if (!*result) return 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(*result);
        return 0;
    }

    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char line[512];
        const char* horse = (const char*)sqlite3_column_text(stmt, 0);
        const char* date = (const char*)sqlite3_column_text(stmt, 1);
        int race_num = sqlite3_column_int(stmt, 2);
        int pos = sqlite3_column_int(stmt, 3);

        if (date) {
            snprintf(line, sizeof(line), "Horse: %s, Date: %s, Race #%d, Position: %d",
                horse ? horse : "unknown", date, race_num, pos);
        }
        else {
            snprintf(line, sizeof(line), "Horse: %s (no races yet)", horse ? horse : "unknown");
        }

        (*result)[*count] = (char*)malloc(strlen(line) + 1);
        if ((*result)[*count]) {
            strcpy((*result)[*count], line);
        }
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int owners_get_horses(sqlite3* db, int owner_id, Horse** horses, int* count) {
    return horses_get_by_owner(db, owner_id, horses, count);
}

void owners_free(Owner** owners, int count) {
    if (owners && *owners) {
        free(*owners);
        *owners = NULL;
    }
}