#ifndef UTILS_H
#define UTILS_H

#include <sqlite3.h>
#include <time.h>

// String utilities
char* utils_trim(char* str);
char* utils_to_lower(const char* str);
int utils_starts_with(const char* str, const char* prefix);

// Date utilities
int utils_parse_date(const char* date_str, struct tm* date);
int utils_validate_date(const char* date_str);
char* utils_get_current_date(void);
int utils_date_diff(const char* start_date, const char* end_date);

// Input utilities
int utils_get_int(const char* prompt);
double utils_get_double(const char* prompt);
void utils_get_string(const char* prompt, char* buffer, int buffer_size);
int utils_get_yes_no(const char* prompt);

// Display utilities
void utils_clear_screen(void);
void utils_print_header(const char* title);
void utils_print_separator(int length);
void utils_print_error(const char* error_msg);
void utils_print_success(const char* msg);
void utils_print_info(const char* msg);
void utils_print_table_row(const char* columns[], int col_count, int widths[]);

// Password utilities
int utils_hash_password(const char* password, char* hash, int hash_size);
int utils_verify_password(const char* password, const char* hash);

// File utilities
int utils_file_exists(const char* path);
int utils_read_file(const char* path, char* buffer, int buffer_size);

// Database utilities for triggers/functions (simulating PL/SQL in C)
int utils_check_horse_exists(sqlite3* db, int horse_id);
int utils_check_jockey_exists(sqlite3* db, int jockey_id);
int utils_get_jockey_by_horse(sqlite3* db, int race_id, int horse_id, int* jockey_id);

#endif // UTILS_H