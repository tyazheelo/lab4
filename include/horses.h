#ifndef HORSES_H
#define HORSES_H

#include <sqlite3.h>

// Horse structure
typedef struct {
    int id;
    char nickname[50];
    int age;
    int experience_years;
    int owner_id;
    char owner_name[100]; // For JOIN queries
} Horse;

// Initialize horses module
int horses_init(sqlite3* db);

// Create new horse (owner or admin)
int horses_create(sqlite3* db, const char* nickname, int age, int experience_years, int owner_id);

// Read horse by ID
int horses_read(sqlite3* db, int horse_id, Horse* horse);

// Update horse (owner of this horse or admin)
int horses_update(sqlite3* db, int horse_id, const char* nickname, int age, int experience_years);

// Delete horse (owner of this horse or admin)
int horses_delete(sqlite3* db, int horse_id);

// Get all horses for an owner (* for owner role)
int horses_get_by_owner(sqlite3* db, int owner_id, Horse** horses, int* count);

// Get all horses (admin only)
int horses_get_all(sqlite3* db, Horse** horses, int* count);

// Get horse with most wins (** for owner role)
int horses_get_most_wins(sqlite3* db, Horse* horse, int* win_count);

// Get horse statistics (total races, wins, prize money)
int horses_get_stats(sqlite3* db, int horse_id, int* total_races, int* wins, double* total_prize);

// Search horses by nickname
int horses_search(sqlite3* db, const char* nickname, Horse** horses, int* count);

// Free allocated horses array
void horses_free(Horse** horses, int count);

#endif // HORSES_H