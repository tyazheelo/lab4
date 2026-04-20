#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int register_user(sqlite3* db, const char* username, const char* password, const char* role) {
    char sql[512];
    snprintf(sql, sizeof(sql),
             "INSERT INTO USERS (username, password, role) VALUES ('%s', '%s', '%s');",
             username, password, role);
    
    char* err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return 0;
    }
    return 1;
}

User* login(sqlite3* db, const char* username, const char* password) {
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id, username, role FROM USERS WHERE username = '%s' AND password = '%s';",
             username, password);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        return NULL;
    }
    
    User* user = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = (User*)malloc(sizeof(User));
        if (user) {
            user->id = sqlite3_column_int(stmt, 0);
            strncpy(user->username, (const char*)sqlite3_column_text(stmt, 1), 49);
            user->username[49] = '\0';
            strncpy(user->role, (const char*)sqlite3_column_text(stmt, 2), 19);
            user->role[19] = '\0';
        }
    }
    
    sqlite3_finalize(stmt);
    return user;
}

void logout(User* user) {
    if (user) {
        free(user);
    }
}