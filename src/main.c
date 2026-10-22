/**
 * @file main.c
 * @brief Application entry point, main execution loop, and user navigation.
 * @course CS-115 Computer Programming
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/donor.h"
#include "../include/inventory.h"
#include "../include/utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Displays the top-level system navigation menu.
 */
static void render_menu(void) {
    printf(COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    CONTROL DASHBOARD RUNTIME                               ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Register New Donor (Triage Validation & Immediate Intake)              ║\n");
    printf("║  2. Tabulate All Donor Records                                             ║\n");
    printf("║  3. Query Donor Record by Unique ID                                        ║\n");
    printf("║  4. Query Donors by Blood Group Designation                                ║\n");
    printf("║  5. Modify Registered Donor Profile                                        ║\n");
    printf("║  6. Purge Donor Record from Database                                       ║\n");
    printf("║  7. Review Central Blood Stock Dashboard                                   ║\n");
    printf("║  8. Manually Restock Blood Units                                           ║\n");
    printf("║  9. Issue / Dispense Blood Units                                           ║\n");
    printf("║  0. Clean Shutdown & Exit                                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
}

int main(void) {
#ifdef _WIN32
    /* Force UTF-8 code page so Windows terminals render Unicode box characters correctly */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    /* Enable ANSI escape sequence processing for colors and UI layout on Windows */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    BloodInventory inventory;

    /* Initialize core subsystems */
    if (!inventory_init(&inventory)) {
        fprintf(stderr, COLOR_RED "Critical Error: Could not mount inventory storage.\n" COLOR_RESET);
        return EXIT_FAILURE;
    }

    int choice = -1;

    while (1) {
        ui_clear_screen();
        ui_print_banner();
        render_menu();

        if (!safe_read_int("  Select Operation [0-9]: ", &choice, 0, 9)) {
            continue;
        }

        if (choice == 0) {
            ui_clear_screen();
            ui_print_banner();
            printf(COLOR_GREEN "  [System Execution Terminated Gracefully]\n\n" COLOR_RESET);
            break;
        }

        switch (choice) {
            case 1: {
                donor_add(&inventory);
                ui_pause();
                break;
            }
            case 2: {
                ui_print_header("Registry Roster");
                donor_display_all();
                ui_pause();
                break;
            }
            case 3: {
                ui_print_header("Identification Search");
                uint32_t target_id;
                safe_read_uint32("  Enter Query ID: ", &target_id, 1, UINT32_MAX);
                donor_search_by_id(target_id);
                ui_pause();
                break;
            }
            case 4: {
                ui_print_header("Phenotype Search");
                char group[BLOOD_GROUP_STR_LEN];
                safe_read_blood_group("  Enter Target Group: ", group);
                donor_search_by_blood_group(group);
                ui_pause();
                break;
            }
            case 5: {
                ui_print_header("Record Revision");
                uint32_t target_id;
                safe_read_uint32("  Enter Target ID to Update: ", &target_id, 1, UINT32_MAX);
                donor_update(target_id);
                ui_pause();
                break;
            }
            case 6: {
                ui_print_header("Purge Record");
                uint32_t target_id;
                safe_read_uint32("  Enter Target ID to Purge: ", &target_id, 1, UINT32_MAX);
                char confirm;
                safe_read_char("  Confirm deletion? (Y/N): ", &confirm, "YN");
                if (confirm == 'Y') {
                    donor_delete(target_id);
                } else {
                    ui_print_status(false, "Deletion aborted by user.");
                }
                ui_pause();
                break;
            }
            case 7: {
                inventory_display_dashboard(&inventory);
                ui_pause();
                break;
            }
            case 8: {
                ui_print_header("Manual Stock Acquisition");
                char group[BLOOD_GROUP_STR_LEN];
                uint32_t units;
                safe_read_blood_group("  Enter Target Blood Group: ", group);
                safe_read_uint32("  Enter Unit Quantity to Add: ", &units, 1, 1000);
                if (inventory_add_unit(&inventory, group, units)) {
                    ui_print_status(true, "Inventory successfully incremented.");
                } else {
                    ui_print_status(false, "Unable to commit inventory additions.");
                }
                ui_pause();
                break;
            }
            case 9: {
                ui_print_header("Dispense / Transfuse Blood Units");
                char group[BLOOD_GROUP_STR_LEN];
                uint32_t units;
                safe_read_blood_group("  Enter Requested Blood Group: ", group);
                safe_read_uint32("  Enter Unit Quantity to Deduct: ", &units, 1, 1000);
                if (inventory_deduct_unit(&inventory, group, units)) {
                    ui_print_status(true, "Units dispensed successfully.");
                }
                ui_pause();
                break;
            }
            default:
                break;
        }
    }

    return EXIT_SUCCESS;
}
