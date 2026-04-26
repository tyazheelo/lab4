#ifndef CONFIG_H
#define CONFIG_H

#define DB_PATH "hippodrome.db"
#define SESSION_TIMEOUT 3600

// Prize distribution ratios (50%, 30%, 20%)
#define PRIZE_FIRST 0.50
#define PRIZE_SECOND 0.30
#define PRIZE_THIRD 0.20

// Role definitions
#define ROLE_ADMIN "admin"
#define ROLE_JOCKEY "jockey"
#define ROLE_OWNER "owner"

// Configuration getters
const char* config_get_db_path(void);
int config_get_session_timeout(void);
double config_get_prize_first(void);
double config_get_prize_second(void);
double config_get_prize_third(void);
const char* config_get_role_admin(void);
const char* config_get_role_jockey(void);
const char* config_get_role_owner(void);

// Configuration setters
void config_set_db_path(const char* path);
void config_set_session_timeout(int timeout);
void config_set_prize_ratios(double first, double second, double third);

// Configuration file operations
int config_load_from_file(const char* filename);
int config_save_to_file(const char* filename);
void config_print(void);
int config_init(void);

#endif // CONFIG_H
