#ifndef RACES_H
#define RACES_H

#include <sqlite3.h>

// Race structure
typedef struct {
    int id;
    char race_date[20];
    int race_number;
    double prize_fund;
} Race;

// Race participation structure
typedef struct {
    int race_id;
    int horse_id;
    int jockey_id;
    int position;
    char horse_nickname[50];
    char jockey_name[50];
} RaceParticipation;

// Initialize races module
int races_init(sqlite3* db);

// Create new race (admin only)
int races_create(sqlite3* db, const char* race_date, int race_number, double prize_fund);

// Read race by ID
int races_read(sqlite3* db, int race_id, Race* race);

// Update race (admin only)
int races_update(sqlite3* db, int race_id, const char* race_date, int race_number, double prize_fund);

// Delete race (admin only)
int races_delete(sqlite3* db, int race_id);

// Add horse and jockey to race (admin only)
int races_add_participant(sqlite3* db, int race_id, int horse_id, int jockey_id);

// Set race result (position) for a participant
int races_set_result(sqlite3* db, int race_id, int horse_id, int position);

// Get all races
int races_get_all(sqlite3* db, Race** races, int* count);

// Get races for a date range
int races_get_by_date_range(sqlite3* db, const char* start_date, const char* end_date, Race** races, int* count);

// Get participants for a race
int races_get_participants(sqlite3* db, int race_id, RaceParticipation** participants, int* count);

// Get winners for a race (positions 1,2,3)
int races_get_winners(sqlite3* db, int race_id, RaceParticipation** winners, int* count);

// Distribute prize fund to top 3
int races_distribute_prize(sqlite3* db, int race_id, double prize_fund);

// Get all races with details (for reports)
int races_get_all_with_details(sqlite3* db, char*** report, int* count);

// Check if horse and jockey exist before inserting
int races_validate_participants(sqlite3* db, int horse_id, int jockey_id);

// Free allocated races array
void races_free(Race** races, int count);

// Free allocated participants array
void races_free_participants(RaceParticipation** participants, int count);

#endif // RACES_H