#ifndef JOCKEYS_H
#define JOCKEYS_H

#include <sqlite3.h>

// Jockey structure
typedef struct {
    int id;
    char last_name[50];
    int experience_years;
    int birth_year;
    char address[255];
    int user_id;
} Jockey;

// Initialize jockeys module
int jockeys_init(sqlite3* db);

// Create new jockey (admin only)
int jockeys_create(sqlite3* db, const char* last_name, int experience_years, int birth_year, const char* address, int user_id);

// Read jockey by ID
int jockeys_read(sqlite3* db, int jockey_id, Jockey* jockey);

// Update jockey (admin only)
int jockeys_update(sqlite3* db, int jockey_id, const char* last_name, int experience_years, int birth_year, const char* address);

// Delete jockey (admin only)
int jockeys_delete(sqlite3* db, int jockey_id);

// Get jockey by user ID
int jockeys_get_by_user_id(sqlite3* db, int user_id, Jockey* jockey);

// Get all jockeys (admin only)
int jockeys_get_all(sqlite3* db, Jockey** jockeys, int* count);

// Get jockey with most race participations
int jockeys_get_most_participations(sqlite3* db, Jockey* jockey, int* participation_count);

// Get jockey race history (* for jockey role)
int jockeys_get_race_history(sqlite3* db, int jockey_id, char*** history, int* count);

// Get jockey statistics (total races, wins, top3 finishes)
int jockeys_get_stats(sqlite3* db, int jockey_id, int* total_races, int* wins, int* top3);

// Calculate jockey earnings from prize distributions
int jockeys_get_earnings(sqlite3* db, int jockey_id, double* earnings);

// Free allocated jockeys array
void jockeys_free(Jockey** jockeys, int count);

#endif // JOCKEYS_H
