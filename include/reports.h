#ifndef REPORTS_H
#define REPORTS_H

#include <sqlite3.h>

// Report structure for queries
typedef struct {
    char** rows;
    int row_count;
    int col_count;
} ReportData;

// Query 2a (*): By specified jockey - information about race dates, horses and places taken
int reports_jockey_races(sqlite3* db, int jockey_id, ReportData* report);

// Query 2b (**): By specified owner - list of his horses with race dates and places
int reports_owner_horses_races(sqlite3* db, int owner_id, ReportData* report);

// Query 2c: Horse that won maximum number of times - information, competition dates and jockeys (**)
int reports_horse_most_wins(sqlite3* db, ReportData* report);

// Query 2d: Jockey with most race participations - information and total races
int reports_jockey_most_participations(sqlite3* db, ReportData* report);

// Query 2e: All races - information about races and participating horses for a specified period
int reports_races_by_period(sqlite3* db, const char* start_date, const char* end_date, ReportData* report);

// Get jockey earnings by period
int reports_jockey_earnings_by_period(sqlite3* db, int jockey_id, const char* start_date, const char* end_date, double* earnings);

// Get prize distribution history
int reports_prize_distribution(sqlite3* db, int race_id, ReportData* report);

// Free report data
void reports_free(ReportData* report);

#endif // REPORTS_H