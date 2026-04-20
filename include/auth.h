#ifndef AUTH_H
#define AUTH_H

#include <sqlite3.h>

typedef struct {
    int id;
    char username[50];
    char role[20];
} User;

int register_user(sqlite3* db, const char* username, const char* password, const char* role);
User* login(sqlite3* db, const char* username, const char* password);
void logout(User* user);

#endif