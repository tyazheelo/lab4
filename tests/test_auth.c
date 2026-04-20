#include <stdio.h>
#include <sqlite3.h>
#include "../include/auth.h"

int main() {
    printf("=== TEST AUTH MODULE ===\n\n");
    int failed = 0;
    
    sqlite3* db;
    int rc = sqlite3_open("data/test.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    
    // Тест 2: Регистрация пользователя
    printf("Test 2: register_user... ");
    if (register_user(db, "admin", "admin123", "admin")) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        failed = 1;
    }
    
    // Тест 3: Успешный вход
    printf("Test 3: login with correct password... ");
    User* user = login(db, "admin", "admin123");
    if (user != NULL) {
        printf("✅ PASSED\n");
        logout(user);
    } else {
        printf("❌ FAILED\n");
        failed = 1;
    }
    
    // Тест 4: Неверный пароль
    printf("Test 4: login with wrong password... ");
    user = login(db, "admin", "wrongpass");
    if (user == NULL) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        logout(user);
        failed = 1;
    }
    
    // Тест 5: Несуществующий пользователь
    printf("Test 5: login with non-existent user... ");
    user = login(db, "unknown", "pass");
    if (user == NULL) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        logout(user);
        failed = 1;
    }
    
    // Тест 6: Дубликат
    printf("Test 6: register duplicate username... ");
    if (!register_user(db, "admin", "newpass", "admin")) {
        printf("✅ PASSED\n");
    } else {
        printf("❌ FAILED\n");
        failed = 1;
    }
    
    sqlite3_close(db);
    
    if (failed) {
        printf("\n=== TESTS FAILED ===\n");
        return 1;
    }
    
    printf("\n=== ALL TESTS PASSED ===\n");
    return 0;
}