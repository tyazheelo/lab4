#include "../include/database.h"
#include "../include/auth.h"
#include "../include/horses.h"
#include "../include/jockeys.h"
#include "../include/owners.h"
#include "../include/races.h"
#include "../include/reports.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sqlite3* db = NULL;
static Session current_session;

void print_menu(void) {
    printf("\n");
    printf("1. Login\n");
    printf("2. Register (Admin only)\n");
    printf("3. Exit\n");
    printf("\nChoice: ");
}

void print_admin_menu(void) {
    printf("\n");
    printf("=== ADMIN MENU ===\n");
    printf("1. Manage Horses\n");
    printf("2. Manage Jockeys\n");
    printf("3. Manage Owners\n");
    printf("4. Manage Races\n");
    printf("5. View Reports\n");
    printf("6. Create User\n");
    printf("7. Logout\n");
    printf("\nChoice: ");
}

void print_jockey_menu(void) {
    printf("\n");
    printf("=== JOCKEY MENU ===\n");
    printf("1. View My Race History (*)\n");
    printf("2. View My Statistics\n");
    printf("3. View My Earnings\n");
    printf("4. View My Horse Information\n");
    printf("5. Logout\n");
    printf("\nChoice: ");
}

void print_owner_menu(void) {
    printf("\n");
    printf("=== OWNER MENU ===\n");
    printf("1. View My Horses (**)\n");
    printf("2. Add New Horse\n");
    printf("3. Update Horse\n");
    printf("4. Delete Horse\n");
    printf("5. View Horse Race Results (**)\n");
    printf("6. View Horse with Most Wins (**)\n");
    printf("7. Logout\n");
    printf("\nChoice: ");
}

void admin_manage_horses(void) {
    int choice;
    printf("\n--- Manage Horses ---\n");
    printf("1. List all horses\n");
    printf("2. Add horse\n");
    printf("3. Update horse\n");
    printf("4. Delete horse\n");
    printf("Choice: ");
    scanf("%d", &choice);
    while (getchar() != '\n');

    switch (choice) {
    case 1: {
        Horse* horses = NULL;
        int count = 0;
        if (horses_get_all(db, &horses, &count)) {
            printf("\n=== All Horses ===\n");
            for (int i = 0; i < count; i++) {
                printf("ID: %d, Nickname: %s, Age: %d, Owner: %s\n",
                    horses[i].id, horses[i].nickname, horses[i].age, horses[i].owner_name);
            }
            horses_free(&horses, count);
        }
        break;
    }
    case 2: {
        char nickname[50];
        int age, exp_years, owner_id;
        utils_get_string("Nickname: ", nickname, sizeof(nickname));
        age = utils_get_int("Age: ");
        exp_years = utils_get_int("Experience years: ");
        owner_id = utils_get_int("Owner ID: ");
        if (horses_create(db, nickname, age, exp_years, owner_id)) {
            utils_print_success("Horse added successfully");
        }
        else {
            utils_print_error("Failed to add horse");
        }
        break;
    }
    case 3: {
        int horse_id = utils_get_int("Horse ID: ");
        char nickname[50];
        int age, exp_years;
        utils_get_string("New nickname: ", nickname, sizeof(nickname));
        age = utils_get_int("New age: ");
        exp_years = utils_get_int("New experience years: ");
        if (horses_update(db, horse_id, nickname, age, exp_years)) {
            utils_print_success("Horse updated successfully");
        }
        else {
            utils_print_error("Failed to update horse");
        }
        break;
    }
    case 4: {
        int horse_id = utils_get_int("Horse ID: ");
        if (horses_delete(db, horse_id)) {
            utils_print_success("Horse deleted successfully");
        }
        else {
            utils_print_error("Failed to delete horse");
        }
        break;
    }
    default:
        utils_print_error("Invalid choice");
    }
}


void admin_manage_races(void) {
    int choice;
    printf("\n--- Manage Races ---\n");
    printf("1. List all races\n");
    printf("2. Create race\n");
    printf("3. Add participant to race\n");
    printf("4. Set race results\n");
    printf("5. Distribute prize fund\n");
    printf("Choice: ");
    scanf("%d", &choice);
    while (getchar() != '\n');

    switch (choice) {
    case 1: {
        Race* races = NULL;
        int count = 0;
        if (races_get_all(db, &races, &count)) {
            printf("\n=== All Races ===\n");
            for (int i = 0; i < count; i++) {
                printf("ID: %d, Date: %s, #%d, Prize: %.2f\n",
                    races[i].id, races[i].race_date, races[i].race_number, races[i].prize_fund);
            }
            races_free(&races, count);
        }
        break;
    }
    case 2: {
        char date[20];
        int race_num;
        double prize;
        utils_get_string("Race date (YYYY-MM-DD): ", date, sizeof(date));
        race_num = utils_get_int("Race number: ");
        prize = utils_get_double("Prize fund: ");
        if (races_create(db, date, race_num, prize)) {
            utils_print_success("Race created successfully");
        }
        else {
            utils_print_error("Failed to create race");
        }
        break;
    }
    case 3: {
        int race_id = utils_get_int("Race ID: ");
        int horse_id = utils_get_int("Horse ID: ");
        int jockey_id = utils_get_int("Jockey ID: ");
        if (races_add_participant(db, race_id, horse_id, jockey_id)) {
            utils_print_success("Participant added");
        }
        else {
            utils_print_error("Failed to add participant");
        }
        break;
    }
    case 4: {
        int race_id = utils_get_int("Race ID: ");
        int horse_id = utils_get_int("Horse ID: ");
        int position = utils_get_int("Position (1-3 or 0 for no place): ");
        if (races_set_result(db, race_id, horse_id, position)) {
            utils_print_success("Result set");
        }
        else {
            utils_print_error("Failed to set result");
        }
        break;
    }
    case 5: {
        int race_id = utils_get_int("Race ID: ");
        Race race;
        if (races_read(db, race_id, &race)) {
            if (races_distribute_prize(db, race_id, race.prize_fund)) {
                utils_print_success("Prize fund distributed");
            }
            else {
                utils_print_error("Failed to distribute prize fund");
            }
        }
        else {
            utils_print_error("Race not found");
        }
        break;
    }
    default:
        utils_print_error("Invalid choice");
    }
}

void jockey_view_history(void) {
    int jockey_id = auth_get_entity_id(db, &current_session);
    if (jockey_id < 0) {
        utils_print_error("Jockey not found");
        return;
    }
    
    char **history = NULL;
    int count = 0;
    if (jockeys_get_race_history(db, jockey_id, &history, &count)) {
        printf("\n=== My Race History ===\n");
        for (int i = 0; i < count; i++) {
            printf("%s\n", history[i]);
            free(history[i]);
        }
        if (count == 0) {
            printf("No race history found.\n");
        }
        free(history);
    } else {
        utils_print_error("Failed to get race history");
    }
}

void owner_view_horses(void) {
    int owner_id = auth_get_entity_id(db, &current_session);
    if (owner_id < 0) {
        utils_print_error("Owner not found");
        return;
    }

    Horse* horses = NULL;
    int count = 0;
    if (owners_get_horses(db, owner_id, &horses, &count)) {
        printf("\n=== My Horses ===\n");
        for (int i = 0; i < count; i++) {
            printf("ID: %d, Nickname: %s, Age: %d, Experience: %d years\n",
                horses[i].id, horses[i].nickname, horses[i].age, horses[i].experience_years);
        }
        if (count == 0) {
            printf("No horses found.\n");
        }
        horses_free(&horses, count);
    }
}

int main(void) {
    // Initialize database
    if (db_init(&db) != SQLITE_OK) {
        utils_print_error("Failed to initialize database");
        return 1;
    }

    // Initialize tables
    auth_init(db);
    horses_init(db);
    jockeys_init(db);
    owners_init(db);
    races_init(db);

    // Initialize session
    memset(&current_session, 0, sizeof(Session));

    int choice;
    while (1) {
        if (!auth_is_authenticated(&current_session)) {
            print_menu();
            choice = utils_get_int("");

            switch (choice) {
            case 1: {
                char username[50], password[50];
                utils_get_string("Username: ", username, sizeof(username));
                utils_get_string("Password: ", password, sizeof(password));
                if (auth_login(db, &current_session, username, password)) {
                    utils_print_success("Login successful");
                }
                else {
                    utils_print_error("Invalid username or password");
                }
                break;
            }
            case 2:
                if (auth_create_user(db, &current_session, "admin", "admin", "admin")) {
                    utils_print_success("Admin user created. Please login.");
                }
                else {
                    utils_print_error("Only admin can create users. Login first.");
                }
                break;
            case 3:
                utils_print_info("Goodbye!");
                db_close(db);
                return 0;
            default:
                utils_print_error("Invalid choice");
            }
        }
        else if (auth_is_admin(&current_session)) {
            print_admin_menu();
            choice = utils_get_int("");

            switch (choice) {
            case 1: admin_manage_horses(); break;
            case 2: // Manage jockeys
                utils_print_info("Jockey management - to be implemented");
                break;
            case 3: // Manage owners
                utils_print_info("Owner management - to be implemented");
                break;
            case 4: admin_manage_races(); break;
            case 5: // View reports
                utils_print_info("Reports - to be implemented");
                break;
            case 6: {
                char username[50], password[50], role[20];
                utils_get_string("Username: ", username, sizeof(username));
                utils_get_string("Password: ", password, sizeof(password));
                utils_get_string("Role (admin/jockey/owner): ", role, sizeof(role));
                if (auth_create_user(db, &current_session, username, password, role)) {
                    utils_print_success("User created");
                }
                else {
                    utils_print_error("Failed to create user");
                }
                break;
            }
            case 7: auth_logout(&current_session); break;
            default: utils_print_error("Invalid choice");
            }
        }
        else if (auth_is_jockey(&current_session)) {
            print_jockey_menu();
            choice = utils_get_int("");

            switch (choice) {
            case 1: jockey_view_history(); break;
            case 2:
                utils_print_info("Statistics - to be implemented");
                break;
            case 3:
                utils_print_info("Earnings - to be implemented");
                break;
            case 4:
                utils_print_info("Horse info - to be implemented");
                break;
            case 5: auth_logout(&current_session); break;
            default: utils_print_error("Invalid choice");
            }
        }
        else if (auth_is_owner(&current_session)) {
            print_owner_menu();
            choice = utils_get_int("");

            switch (choice) {
            case 1: owner_view_horses(); break;
            case 2: {
                char nickname[50];
                int age, exp_years;
                int owner_id = auth_get_entity_id(db, &current_session);
                utils_get_string("Nickname: ", nickname, sizeof(nickname));
                age = utils_get_int("Age: ");
                exp_years = utils_get_int("Experience years: ");
                if (horses_create(db, nickname, age, exp_years, owner_id)) {
                    utils_print_success("Horse added");
                }
                else {
                    utils_print_error("Failed to add horse");
                }
                break;
            }
            case 3: {
                int horse_id = utils_get_int("Horse ID: ");
                char nickname[50];
                int age, exp_years;
                Horse horse;
                if (horses_read(db, horse_id, &horse)) {
                    utils_get_string("New nickname: ", nickname, sizeof(nickname));
                    age = utils_get_int("New age: ");
                    exp_years = utils_get_int("New experience years: ");
                    if (horses_update(db, horse_id, nickname, age, exp_years)) {
                        utils_print_success("Horse updated");
                    }
                    else {
                        utils_print_error("Failed to update");
                    }
                }
                else {
                    utils_print_error("Horse not found or not yours");
                }
                break;
            }
            case 4: {
                int horse_id = utils_get_int("Horse ID: ");
                if (horses_delete(db, horse_id)) {
                    utils_print_success("Horse deleted");
                }
                else {
                    utils_print_error("Failed to delete");
                }
                break;
            }
            case 5: {
                int owner_id = auth_get_entity_id(db, &current_session);
                ReportData report;
                if (reports_owner_horses_races(db, owner_id, &report) == SQLITE_OK) {
                    printf("\n=== My Horses and Race Results ===\n");
                    for (int i = 0; i < report.row_count; i++) {
                        printf("%s\n", report.rows[i]);
                    }
                    reports_free(&report);
                }
                break;
            }
            case 6: {
                ReportData report;
                if (reports_horse_most_wins(db, &report) == SQLITE_OK) {
                    printf("\n=== Horse with Most Wins ===\n");
                    for (int i = 0; i < report.row_count; i++) {
                        printf("%s\n", report.rows[i]);
                    }
                    reports_free(&report);
                }
                break;
            }
            case 7: auth_logout(&current_session); break;
            default: utils_print_error("Invalid choice");
            }
        }
    }

    db_close(db);
    return 0;
}
