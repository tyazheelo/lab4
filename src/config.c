#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global configuration variables
static char g_db_path[256] = "hippodrome.db";
static int g_session_timeout = 3600;
static double g_prize_first = 0.50;
static double g_prize_second = 0.30;
static double g_prize_third = 0.20;

// Getter functions
const char* config_get_db_path(void) {
    return g_db_path;
}

int config_get_session_timeout(void) {
    return g_session_timeout;
}

double config_get_prize_first(void) {
    return g_prize_first;
}

double config_get_prize_second(void) {
    return g_prize_second;
}

double config_get_prize_third(void) {
    return g_prize_third;
}

const char* config_get_role_admin(void) {
    return "admin";
}

const char* config_get_role_jockey(void) {
    return "jockey";
}

const char* config_get_role_owner(void) {
    return "owner";
}

// Setter functions
void config_set_db_path(const char* path) {
    if (path) {
        strncpy(g_db_path, path, sizeof(g_db_path) - 1);
        g_db_path[sizeof(g_db_path) - 1] = '\0';
    }
}

void config_set_session_timeout(int timeout) {
    if (timeout > 0) {
        g_session_timeout = timeout;
    }
}

void config_set_prize_ratios(double first, double second, double third) {
    double sum = first + second + third;
    if (sum > 0.99 && sum < 1.01) {
        g_prize_first = first;
        g_prize_second = second;
        g_prize_third = third;
    }
}

int config_load_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    char line[512];
    char key[256];
    char value[256];
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || line[0] == '#') continue;
        
        if (sscanf(line, "%255[^=]=%255s", key, value) == 2) {
            if (strcmp(key, "DB_PATH") == 0) {
                config_set_db_path(value);
            } else if (strcmp(key, "SESSION_TIMEOUT") == 0) {
                config_set_session_timeout(atoi(value));
            } else if (strcmp(key, "PRIZE_FIRST") == 0) {
                g_prize_first = atof(value);
            } else if (strcmp(key, "PRIZE_SECOND") == 0) {
                g_prize_second = atof(value);
            } else if (strcmp(key, "PRIZE_THIRD") == 0) {
                g_prize_third = atof(value);
            }
        }
    }
    
    fclose(file);
    return 1;
}

int config_save_to_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return 0;
    
    fprintf(file, "# Hippodrome Application Configuration\n");
    fprintf(file, "DB_PATH=%s\n", g_db_path);
    fprintf(file, "SESSION_TIMEOUT=%d\n", g_session_timeout);
    fprintf(file, "PRIZE_FIRST=%.2f\n", g_prize_first);
    fprintf(file, "PRIZE_SECOND=%.2f\n", g_prize_second);
    fprintf(file, "PRIZE_THIRD=%.2f\n", g_prize_third);
    
    fclose(file);
    return 1;
}

void config_print(void) {
    printf("\n=== Current Configuration ===\n");
    printf("Database Path: %s\n", g_db_path);
    printf("Session Timeout: %d seconds\n", g_session_timeout);
    printf("Prize Distribution Ratios:\n");
    printf("  - 1st place: %.0f%%\n", g_prize_first * 100);
    printf("  - 2nd place: %.0f%%\n", g_prize_second * 100);
    printf("  - 3rd place: %.0f%%\n", g_prize_third * 100);
    printf("================================\n\n");
}

int config_init(void) {
    if (config_load_from_file("hippodrome.conf")) {
        return 1;
    }
    return config_save_to_file("hippodrome.conf");
}
