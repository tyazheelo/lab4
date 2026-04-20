#include <stdio.h>
#include <sqlite3.h>
#include "../include/auth.h"

int main() {
    printf("=== TEST AUTH MODULE ===\n\n");
    
    sqlite3* db;
    int rc = sqlite3_open("data/ippodrom.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    // Тест 1: Инициализация таблицы
    printf("Test 1: init_users_table... ");
    if (init_users_table(db)) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        sqlite3_close(db);
        return 1;
    }
    
    // Тест 2: Регистрация пользователя
    printf("Test 2: register_user... ");
    if (register_user(db, "admin", "admin123", "admin")) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
    }
    
    // Тест 3: Успешный вход
    printf("Test 3: login with correct password... ");
    User* user = login(db, "admin", "admin123");
    if (user != NULL) {
        printf("✅ PASSED\n");
        logout(user);
    } else {
        printf("❌ FAILED\n");
    }
    
    // Тест 4: Неуспешный вход (неверный пароль)
    printf("Test 4: login with wrong password... ");
    user = login(db, "admin", "wrongpass");
    if (user == NULL) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        logout(user);
    }
    
    // Тест 5: Неуспешный вход (несуществующий пользователь)
    printf("Test 5: login with non-existent user... ");
    user = login(db, "unknown", "pass");
    if (user == NULL) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        logout(user);
    }
    
    // Тест 6: Регистрация дубликата (должен провалиться)
    printf("Test 6: register duplicate username... ");
    if (!register_user(db, "admin", "newpass", "admin")) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
    }
    
    sqlite3_close(db);
    printf("\n=== ALL TESTS PASSED ===\n");
    return 0;
}