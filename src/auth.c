#include "../include/auth.h"
#include "../include/utils.h"
#include "../include/database.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

static int callback_get_user(void* data, int argc, char** argv, char** azColName) {
    Session* session = (Session*)data;
    if (argc >= 3) {
        session->user_id = atoi(argv[0]);
        strncpy(session->username, argv[1], sizeof(session->username) - 1);
        strncpy(session->role, argv[2], sizeof(session->role) - 1);
        session->logged_in = 1;
    }
    return 0;
}

int auth_init(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS USERS ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username VARCHAR(50) NOT NULL UNIQUE,"
        "password VARCHAR(255) NOT NULL,"
        "role VARCHAR(20) NOT NULL CHECK(role IN ('admin', 'jockey', 'owner'))"
        ");";
    return db_execute(db, sql);
}

int auth_login(sqlite3* db, Session* session, const char* username, const char* password) {
    char sql[512];
    char hashed_pwd[256];

    // Hash the input password for comparison
    if (!utils_hash_password(password, hashed_pwd, sizeof(hashed_pwd))) {
        return 0;
    }

    snprintf(sql, sizeof(sql),
        "SELECT id, username, role FROM USERS WHERE username='%s' AND password='%s';",
        username, hashed_pwd);

    session->logged_in = 0;
    int rc = sqlite3_exec(db, sql, callback_get_user, session, NULL);

    return (rc == SQLITE_OK && session->logged_in);
}

void auth_logout(Session* session) {
    if (session) {
        memset(session, 0, sizeof(Session));
    }
}

int auth_is_authenticated(const Session* session) {
    return session && session->logged_in;
}

const char* auth_get_role(const Session* session) {
    return session ? session->role : NULL;
}

int auth_get_user_id(const Session* session) {
    return session ? session->user_id : -1;
}

int auth_get_entity_id(sqlite3* db, const Session* session) {
    if (!session || !session->logged_in) return -1;

    char sql[256];
    sqlite3_stmt* stmt;
    int entity_id = -1;

    if (strcmp(session->role, ROLE_JOCKEY) == 0) {
        snprintf(sql, sizeof(sql), "SELECT id FROM JOCKEYS WHERE user_id = %d;", session->user_id);
    }
    else if (strcmp(session->role, ROLE_OWNER) == 0) {
        snprintf(sql, sizeof(sql), "SELECT id FROM OWNERS WHERE user_id = %d;", session->user_id);
    }
    else {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            entity_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return entity_id;
}

int auth_is_admin(const Session* session) {
    return session && session->logged_in && strcmp(session->role, ROLE_ADMIN) == 0;
}

int auth_is_jockey(const Session* session) {
    return session && session->logged_in && strcmp(session->role, ROLE_JOCKEY) == 0;
}

int auth_is_owner(const Session* session) {
    return session && session->logged_in && strcmp(session->role, ROLE_OWNER) == 0;
}

int auth_change_password(sqlite3* db, const Session* session, int user_id,
    const char* old_password, const char* new_password) {
    if (!auth_is_authenticated(session)) return 0;
    if (!auth_is_admin(session) && session->user_id != user_id) return 0;

    char sql[512];
    char old_hash[256], new_hash[256];

    if (!utils_hash_password(old_password, old_hash, sizeof(old_hash))) return 0;
    if (!utils_hash_password(new_password, new_hash, sizeof(new_hash))) return 0;

    snprintf(sql, sizeof(sql),
        "UPDATE USERS SET password='%s' WHERE id=%d AND password='%s';",
        new_hash, user_id, old_hash);

    int rc = db_execute(db, sql);
    return (rc == SQLITE_OK && sqlite3_changes(db) > 0);
}

int auth_create_user(sqlite3* db, const Session* session, const char* username,
    const char* password, const char* role) {
    if (!auth_is_admin(session)) return 0;

    char sql[512];
    char hashed_pwd[256];

    if (!utils_hash_password(password, hashed_pwd, sizeof(hashed_pwd))) return 0;

    snprintf(sql, sizeof(sql),
        "INSERT INTO USERS (username, password, role) VALUES ('%s', '%s', '%s');",
        username, hashed_pwd, role);

    return (db_execute(db, sql) == SQLITE_OK);
}