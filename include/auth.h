#ifndef AUTH_H
#define AUTH_H

#include <sqlite3.h>

// Session structure
typedef struct {
    int user_id;
    char username[50];
    char role[20];
    int logged_in;
} Session;

// Initialize authentication module
int auth_init(sqlite3* db);

// Login user
int auth_login(sqlite3* db, Session* session, const char* username, const char* password);

// Logout user
void auth_logout(Session* session);

// Check if user is authenticated
int auth_is_authenticated(const Session* session);

// Get current user role
const char* auth_get_role(const Session* session);

// Get current user ID
int auth_get_user_id(const Session* session);

// Get related entity ID (jockey_id or owner_id) for current user
int auth_get_entity_id(sqlite3* db, const Session* session);

// Check if current user has admin role
int auth_is_admin(const Session* session);

// Check if current user has jockey role
int auth_is_jockey(const Session* session);

// Check if current user has owner role
int auth_is_owner(const Session* session);

// Change user password (admin only or own password)
int auth_change_password(sqlite3* db, const Session* session, int user_id, const char* old_password, const char* new_password);

// Create new user (admin only)
int auth_create_user(sqlite3* db, const Session* session, const char* username, const char* password, const char* role);

#endif // AUTH_H