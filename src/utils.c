#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sqlite3.h>

// Simple hash function (for demo - in production use proper crypto like SHA256)
static unsigned int simple_hash(const char* str) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// String utilities
char* utils_trim(char* str) {
    if (!str) return NULL;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    // Trim trailing space
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

char* utils_to_lower(const char* str) {
    if (!str) return NULL;

    char* result = malloc(strlen(str) + 1);
    if (!result) return NULL;

    for (int i = 0; str[i]; i++) {
        result[i] = tolower((unsigned char)str[i]);
    }
    result[strlen(str)] = '\0';

    return result;
}

int utils_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Date utilities
int utils_parse_date(const char* date_str, struct tm* date) {
    if (!date_str || !date) return 0;
    return sscanf(date_str, "%d-%d-%d", &date->tm_year, &date->tm_mon, &date->tm_mday) == 3;
}

int utils_validate_date(const char* date_str) {
    struct tm date;
    if (!utils_parse_date(date_str, &date)) return 0;

    // Basic validation
    if (date.tm_year < 2020 || date.tm_year > 2030) return 0;
    if (date.tm_mon < 1 || date.tm_mon > 12) return 0;
    if (date.tm_mday < 1 || date.tm_mday > 31) return 0;

    return 1;
}

char* utils_get_current_date(void) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    char* date_str = malloc(11); // YYYY-MM-DD + null
    if (!date_str) return NULL;

    snprintf(date_str, 11, "%04d-%02d-%02d",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

    return date_str;
}

int utils_date_diff(const char* start_date, const char* end_date) {
    struct tm start_tm = { 0 }, end_tm = { 0 };

    sscanf(start_date, "%d-%d-%d", &start_tm.tm_year, &start_tm.tm_mon, &start_tm.tm_mday);
    sscanf(end_date, "%d-%d-%d", &end_tm.tm_year, &end_tm.tm_mon, &end_tm.tm_mday);

    start_tm.tm_year -= 1900;
    start_tm.tm_mon -= 1;
    end_tm.tm_year -= 1900;
    end_tm.tm_mon -= 1;

    time_t start_time = mktime(&start_tm);
    time_t end_time = mktime(&end_tm);

    return (int)difftime(end_time, start_time) / (60 * 60 * 24);
}

// Input utilities
int utils_get_int(const char* prompt) {
    int value;
    printf("%s", prompt);
    scanf("%d", &value);
    while (getchar() != '\n'); // Clear buffer
    return value;
}

double utils_get_double(const char* prompt) {
    double value;
    printf("%s", prompt);
    scanf("%lf", &value);
    while (getchar() != '\n');
    return value;
}

void utils_get_string(const char* prompt, char* buffer, int buffer_size) {
    printf("%s", prompt);
    fgets(buffer, buffer_size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
}

int utils_get_yes_no(const char* prompt) {
    char response[10];
    printf("%s (y/n): ", prompt);
    fgets(response, sizeof(response), stdin);
    response[strcspn(response, "\n")] = 0;

    return (strcmp(response, "y") == 0 || strcmp(response, "Y") == 0 ||
        strcmp(response, "yes") == 0 || strcmp(response, "YES") == 0);
}

// Display utilities
void utils_clear_screen(void) {
    printf("\033[2J\033[1;1H"); // ANSI escape codes for clear screen
}

void utils_print_header(const char* title) {
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

void utils_print_separator(int length) {
    for (int i = 0; i < length; i++) {
        printf("-");
    }
    printf("\n");
}

void utils_print_error(const char* error_msg) {
    printf("\n[ERROR] %s\n\n", error_msg);
}

void utils_print_success(const char* msg) {
    printf("\n[SUCCESS] %s\n\n", msg);
}

void utils_print_info(const char* msg) {
    printf("[INFO] %s\n", msg);
}

void utils_print_table_row(const char* columns[], int col_count, int widths[]) {
    for (int i = 0; i < col_count; i++) {
        printf("%-*s", widths[i], columns[i]);
        if (i < col_count - 1) printf(" | ");
    }
    printf("\n");
}

// Password utilities
int utils_hash_password(const char* password, char* hash, int hash_size) {
    if (!password || !hash || hash_size < 32) return 0;

    unsigned int h = simple_hash(password);
    snprintf(hash, hash_size, "%08x%08x%08x%08x", h, h >> 16, h << 16, ~h);
    return 1;
}

int utils_verify_password(const char* password, const char* hash) {
    char computed_hash[256];
    if (!utils_hash_password(password, computed_hash, sizeof(computed_hash))) {
        return 0;
    }
    return strcmp(computed_hash, hash) == 0;
}

// File utilities
int utils_file_exists(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int utils_read_file(const char* path, char* buffer, int buffer_size) {
    FILE* file = fopen(path, "r");
    if (!file) return 0;

    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return bytes_read;
}

// Database utilities
int utils_check_horse_exists(sqlite3* db, int horse_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT 1 FROM HORSES WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, horse_id);
    int exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return exists;
}

int utils_check_jockey_exists(sqlite3* db, int jockey_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT 1 FROM JOCKEYS WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, jockey_id);
    int exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return exists;
}

int utils_get_jockey_by_horse(sqlite3* db, int race_id, int horse_id, int* jockey_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT jockey_id FROM RACES_HORSES_JOCKEYS WHERE race_id = ? AND horse_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, race_id);
    sqlite3_bind_int(stmt, 2, horse_id);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *jockey_id = sqlite3_column_int(stmt, 0);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found;
}