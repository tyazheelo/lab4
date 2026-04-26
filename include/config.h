#ifndef CONFIG_H
#define CONFIG_H

// Database configuration
#define DB_PATH "hippodrome.db"

// Authentication configuration
#define SESSION_TIMEOUT 3600 // seconds

// Role definitions
#define ROLE_ADMIN "admin"
#define ROLE_JOCKEY "jockey"
#define ROLE_OWNER "owner"

// Prize distribution ratios (50%, 30%, 20%)
#define PRIZE_FIRST 0.50
#define PRIZE_SECOND 0.30
#define PRIZE_THIRD 0.20

#endif // CONFIG_H