#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string.h>

extern "C" {
#include "../include/auth.h"
#include "../include/database.h"
#include "../include/config.h"
}

class AuthTest : public ::testing::Test {
protected:
    sqlite3* db;
    Session session;

    void SetUp() override {
        config_set_db_path("test_auth.db");
        ASSERT_EQ(SQLITE_OK, db_init(&db));
        auth_init(db);
        memset(&session, 0, sizeof(Session));

        // Create test users
        auth_create_user(db, NULL, "test_admin", "admin123", ROLE_ADMIN);
        auth_create_user(db, NULL, "test_jockey", "jockey123", ROLE_JOCKEY);
        auth_create_user(db, NULL, "test_owner", "owner123", ROLE_OWNER);
    }

    void TearDown() override {
        if (db) {
            db_close(db);
        }
        remove("test_auth.db");
    }
};

TEST_F(AuthTest, InitAuth) {
    // Table should already exist from SetUp
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='USERS';";
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db, sql, -1, &stmt, NULL));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
}

TEST_F(AuthTest, LoginSuccess) {
    ASSERT_TRUE(auth_login(db, &session, "test_admin", "admin123"));
    ASSERT_TRUE(session.logged_in);
    ASSERT_STREQ(ROLE_ADMIN, session.role);
}

TEST_F(AuthTest, LoginFailureWrongPassword) {
    ASSERT_FALSE(auth_login(db, &session, "test_admin", "wrongpass"));
    ASSERT_FALSE(session.logged_in);
}

TEST_F(AuthTest, LoginFailureUserNotFound) {
    ASSERT_FALSE(auth_login(db, &session, "nonexistent", "pass"));
    ASSERT_FALSE(session.logged_in);
}

TEST_F(AuthTest, Logout) {
    auth_login(db, &session, "test_admin", "admin123");
    ASSERT_TRUE(auth_is_authenticated(&session));

    auth_logout(&session);
    ASSERT_FALSE(auth_is_authenticated(&session));
}

TEST_F(AuthTest, RoleChecksAdmin) {
    auth_login(db, &session, "test_admin", "admin123");
    ASSERT_TRUE(auth_is_admin(&session));
    ASSERT_FALSE(auth_is_jockey(&session));
    ASSERT_FALSE(auth_is_owner(&session));
}

TEST_F(AuthTest, RoleChecksJockey) {
    auth_login(db, &session, "test_jockey", "jockey123");
    ASSERT_FALSE(auth_is_admin(&session));
    ASSERT_TRUE(auth_is_jockey(&session));
    ASSERT_FALSE(auth_is_owner(&session));
}

TEST_F(AuthTest, RoleChecksOwner) {
    auth_login(db, &session, "test_owner", "owner123");
    ASSERT_FALSE(auth_is_admin(&session));
    ASSERT_FALSE(auth_is_jockey(&session));
    ASSERT_TRUE(auth_is_owner(&session));
}

TEST_F(AuthTest, GetUserRole) {
    auth_login(db, &session, "test_jockey", "jockey123");
    ASSERT_STREQ(ROLE_JOCKEY, auth_get_role(&session));
}

TEST_F(AuthTest, GetUserId) {
    auth_login(db, &session, "test_jockey", "jockey123");
    ASSERT_GT(auth_get_user_id(&session), 0);
}

TEST_F(AuthTest, ChangeOwnPassword) {
    auth_login(db, &session, "test_jockey", "jockey123");
    int user_id = auth_get_user_id(&session);

    ASSERT_TRUE(auth_change_password(db, &session, user_id, "jockey123", "newpass123"));

    // Try to login with new password
    Session new_session;
    memset(&new_session, 0, sizeof(Session));
    ASSERT_TRUE(auth_login(db, &new_session, "test_jockey", "newpass123"));
}

TEST_F(AuthTest, ChangeOtherPasswordAsAdmin) {
    auth_login(db, &session, "test_admin", "admin123");

    // Get jockey user id
    Session jockey_session;
    auth_login(db, &jockey_session, "test_jockey", "jockey123");
    int jockey_user_id = auth_get_user_id(&jockey_session);

    // Admin changes jockey's password
    ASSERT_TRUE(auth_change_password(db, &session, jockey_user_id, "jockey123", "adminchanged"));

    // Verify new password works
    Session new_session;
    ASSERT_TRUE(auth_login(db, &new_session, "test_jockey", "adminchanged"));
}

TEST_F(AuthTest, ChangeOtherPasswordAsNonAdmin) {
    auth_login(db, &session, "test_jockey", "jockey123");

    Session owner_session;
    auth_login(db, &owner_session, "test_owner", "owner123");
    int owner_user_id = auth_get_user_id(&owner_session);

    // Jockey cannot change owner's password
    ASSERT_FALSE(auth_change_password(db, &session, owner_user_id, "owner123", "hacked"));
}

TEST_F(AuthTest, CreateUserAsAdmin) {
    auth_login(db, &session, "test_admin", "admin123");

    ASSERT_TRUE(auth_create_user(db, &session, "new_user", "newpass", ROLE_JOCKEY));

    // Verify user was created
    Session new_session;
    ASSERT_TRUE(auth_login(db, &new_session, "new_user", "newpass"));
    ASSERT_STREQ(ROLE_JOCKEY, new_session.role);
}

TEST_F(AuthTest, CreateUserAsNonAdmin) {
    auth_login(db, &session, "test_jockey", "jockey123");

    ASSERT_FALSE(auth_create_user(db, &session, "unauth_user", "pass", ROLE_OWNER));
}

TEST_F(AuthTest, GetEntityIdForJockey) {
    // First create jockey entity
    auth_login(db, &session, "test_jockey", "jockey123");
    int user_id = auth_get_user_id(&session);

    // Create jockey record (simplified - in real app this would be done via jockeys_create)
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO JOCKEYS (last_name, experience_years, birth_year, user_id) VALUES ('Test', 5, 1990, %d);", user_id);
    db_execute(db, sql);

    int entity_id = auth_get_entity_id(db, &session);
    ASSERT_GT(entity_id, 0);
}

TEST_F(AuthTest, GetEntityIdForOwner) {
    auth_login(db, &session, "test_owner", "owner123");
    int user_id = auth_get_user_id(&session);

    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO OWNERS (name, last_name, user_id) VALUES ('Test', 'Owner', %d);", user_id);
    db_execute(db, sql);

    int entity_id = auth_get_entity_id(db, &session);
    ASSERT_GT(entity_id, 0);
}

TEST_F(AuthTest, GetEntityIdForAdmin) {
    auth_login(db, &session, "test_admin", "admin123");
    int entity_id = auth_get_entity_id(db, &session);
    ASSERT_EQ(-1, entity_id);
}