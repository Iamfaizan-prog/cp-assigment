/**
 * @file donor.c
 * @brief Donor registration, medical triage validation, and file-backed operations.
 * @course CS-115 Computer Programming
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/donor.h"
#include "../include/inventory.h"
#include "../include/utils.h"

/**
 * @brief Validates donor candidacy based on age, weight, and donation intervals.
 */
bool donor_check_eligibility(int age, double weight, bool has_donated, const Date *last_donation, const Date *today) {
    bool eligible = true;

    printf("\n" COLOR_BOLD "───[ Medical Eligibility Triage Evaluation ]───" COLOR_RESET "\n");

    if (age < MIN_DONOR_AGE || age > MAX_DONOR_AGE) {
        printf(COLOR_RED "  [!] Ineligible: Age (%d) outside acceptable parameters (%d - %d years).\n" COLOR_RESET,
               age, MIN_DONOR_AGE, MAX_DONOR_AGE);
        eligible = false;
    } else {
        printf(COLOR_GREEN "  [✓] Criteria Met: Age verification passed (%d years).\n" COLOR_RESET, age);
    }

    if (weight < MIN_DONOR_WEIGHT_KG) {
        printf(COLOR_RED "  [!] Ineligible: Weight (%.1f kg) below critical threshold (%.1f kg).\n" COLOR_RESET,
               weight, MIN_DONOR_WEIGHT_KG);
        eligible = false;
    } else {
        printf(COLOR_GREEN "  [✓] Criteria Met: Body mass index verified (%.1f kg).\n" COLOR_RESET, weight);
    }

    if (has_donated) {
        long delta_days = days_between_dates(last_donation, today);
        if (delta_days < MIN_DONATION_INTERVAL) {
            printf(COLOR_RED "  [!] Ineligible: Elapsed recovery period (%ld days) is under threshold (%d days).\n" COLOR_RESET,
                   delta_days, MIN_DONATION_INTERVAL);
            eligible = false;
        } else {
            printf(COLOR_GREEN "  [✓] Criteria Met: Recovery spacing verified (%ld days since last donation).\n" COLOR_RESET, delta_days);
        }
    } else {
        printf(COLOR_GREEN "  [✓] Criteria Met: First-time registered donor status confirmed.\n" COLOR_RESET);
    }

    return eligible;
}

/**
 * @brief Dynamically derives the next sequence ID from binary storage.
 */
uint32_t donor_generate_next_id(void) {
    FILE *fp = fopen(DONOR_DB_FILE, "rb");
    if (fp == NULL) {
        return 1001; /* Initial seed record identifier */
    }

    Donor record;
    uint32_t max_id = 1000;

    while (fread(&record, sizeof(Donor), 1, fp) == 1) {
        if (record.id > max_id) {
            max_id = record.id;
        }
    }

    fclose(fp);
    return max_id + 1;
}

/**
 * @brief Prompts for donor details, checks eligibility, and writes the record.
 */
bool donor_add(BloodInventory *inv) {
    Donor d;
    memset(&d, 0, sizeof(Donor));
    Date today;
    get_current_date(&today);

    ui_print_header("Register New Donor");

    d.id = donor_generate_next_id();
    printf("  Assigned Donor Tracking ID: " COLOR_YELLOW COLOR_BOLD "%u\n" COLOR_RESET, d.id);

    safe_read_string("  Enter Full Name: ", d.name, sizeof(d.name));
    safe_read_int("  Enter Age (years): ", &d.age, 1, 120);
    safe_read_char("  Enter Biological Gender (M/F/O): ", &d.gender, "MFO");
    safe_read_double("  Enter Body Weight (kg): ", &d.weight, 20.0, 300.0);
    safe_read_blood_group("  Enter Blood Group: ", d.blood_group);
    safe_read_string("  Enter Contact Phone: ", d.phone, sizeof(d.phone));

    char prior_choice;
    safe_read_char("  Has donor contributed blood previously? (Y/N): ", &prior_choice, "YN");
    d.has_donated_before = (prior_choice == 'Y');

    if (d.has_donated_before) {
        safe_read_date("  Enter Last Donation Date (DD/MM/YYYY): ", &d.last_donation);
    } else {
        d.last_donation.day = 0;
        d.last_donation.month = 0;
        d.last_donation.year = 0;
    }

    /* Run eligibility verification */
    bool is_eligible = donor_check_eligibility(d.age, d.weight, d.has_donated_before, &d.last_donation, &today);

    if (!is_eligible) {
        ui_print_status(false, "Donor disqualified under clinical safety criteria. Registration aborted.");
        return false;
    }

    /* Append to binary data file */
    FILE *fp = fopen(DONOR_DB_FILE, "ab");
    if (fp == NULL) {
        ui_print_status(false, "Storage access violation: unable to open database file.");
        return false;
    }

    size_t written = fwrite(&d, sizeof(Donor), 1, fp);
    fclose(fp);

    if (written != 1) {
        ui_print_status(false, "Write failure: unable to append record safely.");
        return false;
    }

    ui_print_status(true, "Donor record written to persistent binary storage.");

    /* Optional automatic inventory restocking */
    char donate_now;
    safe_read_char("  Was a unit of blood collected during this intake session? (Y/N): ", &donate_now, "YN");
    if (donate_now == 'Y') {
        if (inventory_add_unit(inv, d.blood_group, 1)) {
            ui_print_status(true, "Inventory automatically incremented by 1 unit.");
        } else {
            ui_print_status(false, "Failed to update inventory unit count.");
        }
    }

    return true;
}

/**
 * @brief Reads all stored donor records and displays them in a formatted table.
 */
bool donor_display_all(void) {
    FILE *fp = fopen(DONOR_DB_FILE, "rb");
    if (fp == NULL) {
        ui_print_status(false, "Database empty or file does not exist.");
        return false;
    }

    printf("\n" COLOR_WHITE COLOR_BOLD);
    printf("╔══════╦══════════════════════════╦═════╦════╦════════╦═══════╦═══════════════╦════════════╗\n");
    printf("║ ID   ║ Name                     ║ Age ║ Gen║ Weight ║ Group ║ Contact       ║ Last Don.  ║\n");
    printf("╠══════╬══════════════════════════╬═════╬════╬════════╬═══════╬═══════════════╬════════════╣\n");
    printf(COLOR_RESET);

    Donor d;
    size_t count = 0;
    while (fread(&d, sizeof(Donor), 1, fp) == 1) {
        char date_str[16];
        if (d.has_donated_before) {
            snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d", d.last_donation.day, d.last_donation.month, d.last_donation.year);
        } else {
            snprintf(date_str, sizeof(date_str), "N/A (First)");
        }

        printf("║ %-4u ║ %-24.24s ║ %-3d ║ %-2c ║ %-6.1f ║ %-5s ║ %-13.13s ║ %-10s ║\n",
               d.id, d.name, d.age, d.gender, d.weight, d.blood_group, d.phone, date_str);
        count++;
    }

    printf(COLOR_WHITE COLOR_BOLD);
    printf("╚══════╩══════════════════════════╩═════╩════╩════════╩═══════╩═══════════════╩════════════╝\n");
    printf(COLOR_RESET);

    fclose(fp);
    printf("  Total records displayed: %lu\n", (unsigned long)count);
    return true;
}

/**
 * @brief Locates a donor record by unique identifier.
 */
bool donor_search_by_id(uint32_t target_id) {
    FILE *fp = fopen(DONOR_DB_FILE, "rb");
    if (fp == NULL) {
        ui_print_status(false, "Data repository unavailable.");
        return false;
    }

    Donor d;
    bool found = false;
    while (fread(&d, sizeof(Donor), 1, fp) == 1) {
        if (d.id == target_id) {
            found = true;
            printf("\n" COLOR_GREEN "  ───[ Record Located ]───" COLOR_RESET "\n");
            printf("  ID:            %u\n", d.id);
            printf("  Name:          %s\n", d.name);
            printf("  Age:           %d\n", d.age);
            printf("  Gender:        %c\n", d.gender);
            printf("  Weight:        %.1f kg\n", d.weight);
            printf("  Blood Group:   " COLOR_RED "%s\n" COLOR_RESET, d.blood_group);
            printf("  Phone:         %s\n", d.phone);
            if (d.has_donated_before) {
                printf("  Last Donation: %02d/%02d/%04d\n", d.last_donation.day, d.last_donation.month, d.last_donation.year);
            } else {
                printf("  Last Donation: None on record\n");
            }
            break;
        }
    }

    fclose(fp);
    if (!found) {
        ui_print_status(false, "Donor record not found for specified ID.");
    }
    return found;
}

/**
 * @brief Performs linear scan to display records matching a target blood group.
 */
bool donor_search_by_blood_group(const char *target_group) {
    FILE *fp = fopen(DONOR_DB_FILE, "rb");
    if (fp == NULL) {
        ui_print_status(false, "Data repository empty or unavailable.");
        return false;
    }

    Donor d;
    size_t matches = 0;

    printf("\n" COLOR_WHITE COLOR_BOLD);
    printf("╔══════╦══════════════════════════╦═════╦════╦════════╦═══════╦═══════════════╦════════════╗\n");
    printf("║ ID   ║ Name                     ║ Age ║ Gen║ Weight ║ Group ║ Contact       ║ Last Don.  ║\n");
    printf("╠══════╬══════════════════════════╬═════╬════╬════════╬═══════╬═══════════════╬════════════╣\n");
    printf(COLOR_RESET);

    while (fread(&d, sizeof(Donor), 1, fp) == 1) {
        if (strcmp(d.blood_group, target_group) == 0) {
            char date_str[16];
            if (d.has_donated_before) {
                snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d", d.last_donation.day, d.last_donation.month, d.last_donation.year);
            } else {
                snprintf(date_str, sizeof(date_str), "N/A (First)");
            }

            printf("║ %-4u ║ %-24.24s ║ %-3d ║ %-2c ║ %-6.1f ║ %-5s ║ %-13.13s ║ %-10s ║\n",
                   d.id, d.name, d.age, d.gender, d.weight, d.blood_group, d.phone, date_str);
            matches++;
        }
    }

    printf(COLOR_WHITE COLOR_BOLD);
    printf("╚══════╩══════════════════════════╩═════╩════╩════════╩═══════╩═══════════════╩════════════╝\n");
    printf(COLOR_RESET);

    fclose(fp);

    if (matches == 0) {
        ui_print_status(false, "No matching records found for this blood group.");
        return false;
    }

    printf("  Found %lu donor record(s) matching group %s.\n", (unsigned long)matches, target_group);
    return true;
}

/**
 * @brief Updates donor data using an atomic stream-rewriting pattern.
 */
bool donor_update(uint32_t target_id) {
    FILE *source_fp = fopen(DONOR_DB_FILE, "rb");
    if (source_fp == NULL) {
        ui_print_status(false, "Database access failure.");
        return false;
    }

    FILE *temp_fp = fopen("donors.tmp", "wb");
    if (temp_fp == NULL) {
        fclose(source_fp);
        ui_print_status(false, "I/O error: unable to allocate temporary storage.");
        return false;
    }

    Donor current;
    bool found = false;

    while (fread(&current, sizeof(Donor), 1, source_fp) == 1) {
        if (current.id == target_id) {
            found = true;
            printf(COLOR_CYAN "  [Editing Donor Record ID: %u]" COLOR_RESET "\n", current.id);
            safe_read_string("  Enter Updated Name: ", current.name, sizeof(current.name));
            safe_read_int("  Enter Updated Age: ", &current.age, 1, 120);
            safe_read_double("  Enter Updated Weight (kg): ", &current.weight, 20.0, 300.0);
            safe_read_string("  Enter Updated Contact: ", current.phone, sizeof(current.phone));

            char update_date;
            safe_read_char("  Update Last Donation Date? (Y/N): ", &update_date, "YN");
            if (update_date == 'Y') {
                safe_read_date("  Enter Date (DD/MM/YYYY): ", &current.last_donation);
                current.has_donated_before = true;
            }
        }
        /* Stream record to temporary storage */
        fwrite(&current, sizeof(Donor), 1, temp_fp);
    }

    fclose(source_fp);
    fclose(temp_fp);

    if (found) {
        remove(DONOR_DB_FILE);
        rename("donors.tmp", DONOR_DB_FILE);
        ui_print_status(true, "Donor record updated successfully.");
        return true;
    } else {
        remove("donors.tmp");
        ui_print_status(false, "ID not found. Database unchanged.");
        return false;
    }
}

/**
 * @brief Removes a donor by ID using an atomic stream-rewriting pattern.
 */
bool donor_delete(uint32_t target_id) {
    FILE *source_fp = fopen(DONOR_DB_FILE, "rb");
    if (source_fp == NULL) {
        ui_print_status(false, "Database file missing or unreadable.");
        return false;
    }

    FILE *temp_fp = fopen("donors.tmp", "wb");
    if (temp_fp == NULL) {
        fclose(source_fp);
        ui_print_status(false, "Unable to allocate temporary swap stream.");
        return false;
    }

    Donor current;
    bool found = false;

    while (fread(&current, sizeof(Donor), 1, source_fp) == 1) {
        if (current.id == target_id) {
            found = true;
            /* Record intentionally omitted to delete it */
            continue;
        }
        fwrite(&current, sizeof(Donor), 1, temp_fp);
    }

    fclose(source_fp);
    fclose(temp_fp);

    if (found) {
        remove(DONOR_DB_FILE);
        rename("donors.tmp", DONOR_DB_FILE);
        ui_print_status(true, "Donor purged from persistence registry.");
        return true;
    } else {
        remove("donors.tmp");
        ui_print_status(false, "Target ID non-existent. No deletions committed.");
        return false;
    }
}