#ifndef OWNERS_H
#define OWNERS_H

#include <sqlite3.h>
#include "horses.h"  // Include horses.h directly instead of forward declaration

// Owner structure
typedef struct {
    int id;
    char name[30];
    char last_name[30];
    char middle_name[30];
    char address[255];
    char phone[20];
    int user_id;
} Owner;

// Initialize owners module
int owners_init(sqlite3* db);

// Create new owner (admin only)
int owners_create(sqlite3* db, const char* name, const char* last_name, const char* middle_name,
    const char* address, const char* phone, int user_id);

// Read owner by ID
int owners_read(sqlite3* db, int owner_id, Owner* owner);

// Update owner (admin only)
int owners_update(sqlite3* db, int owner_id, const char* name, const char* last_name,
    const char* middle_name, const char* address, const char* phone);

// Delete owner (admin only)
int owners_delete(sqlite3* db, int owner_id);

// Get owner by user ID
int owners_get_by_user_id(sqlite3* db, int user_id, Owner* owner);

// Get all owners (admin only)
int owners_get_all(sqlite3* db, Owner** owners, int* count);

// Get owner with their horses and race results (** for owner role)
int owners_get_with_horses_and_races(sqlite3* db, int owner_id, char*** result, int* count);

// Get owner horses list (** for owner role)
int owners_get_horses(sqlite3* db, int owner_id, Horse** horses, int* count);

// Free allocated owners array
void owners_free(Owner** owners, int count);

#endif // OWNERS_H