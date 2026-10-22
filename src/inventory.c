/**
 * @file inventory.c
 * @brief Inventory management, binary persistence, and quota management.
 * @course CS-115 Computer Programming
 */

#include <stdio.h>
#include <string.h>
#include "../include/inventory.h"
#include "../include/utils.h"

/* Static Global Group Roster */
const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS] = {
    "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"
};

/**
 * @brief Resolves index offset of a blood group; returns -1 on unmatched.
 */
int inventory_resolve_index(const char *blood_group) {
    if (blood_group == NULL) return -1;
    for (int i = 0; i < NUM_BLOOD_GROUPS; ++i) {
        if (strcmp(VALID_BLOOD_GROUPS[i], blood_group) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Initializes inventory state; defaults file if missing.
 */
bool inventory_init(BloodInventory *inv) {
    if (inv == NULL) return false;

    FILE *fp = fopen(INVENTORY_DB_FILE, "rb");
    if (fp == NULL) {
        /* File absent: initialize baseline stock records */
        inv->total_units = 0;
        for (int i = 0; i < NUM_BLOOD_GROUPS; ++i) {
            strncpy(inv->stocks[i].blood_group, VALID_BLOOD_GROUPS[i], BLOOD_GROUP_STR_LEN);
            inv->stocks[i].units = 0;
        }
        return inventory_save(inv);
    }

    fclose(fp);
    return inventory_load(inv);
}

/**
 * @brief Deserializes inventory binary state from file storage.
 */
bool inventory_load(BloodInventory *inv) {
    if (inv == NULL) return false;

    FILE *fp = fopen(INVENTORY_DB_FILE, "rb");
    if (fp == NULL) return false;

    size_t read_count = fread(inv, sizeof(BloodInventory), 1, fp);
    fclose(fp);

    return (read_count == 1);
}

/**
 * @brief Serializes inventory state to binary storage.
 */
bool inventory_save(const BloodInventory *inv) {
    if (inv == NULL) return false;

    FILE *fp = fopen(INVENTORY_DB_FILE, "wb");
    if (fp == NULL) {
        perror("Failed to persist inventory file");
        return false;
    }

    size_t written_count = fwrite(inv, sizeof(BloodInventory), 1, fp);
    fclose(fp);

    return (written_count == 1);
}

/**
 * @brief Atomically increments available units for a specific blood group.
 */
bool inventory_add_unit(BloodInventory *inv, const char *blood_group, uint32_t count) {
    if (inv == NULL || blood_group == NULL) return false;

    int idx = inventory_resolve_index(blood_group);
    if (idx == -1) return false;

    inv->stocks[idx].units += count;
    inv->total_units += count;

    return inventory_save(inv);
}

/**
 * @brief Atomically decrements and validates available inventory stocks.
 */
bool inventory_deduct_unit(BloodInventory *inv, const char *blood_group, uint32_t count) {
    if (inv == NULL || blood_group == NULL) return false;

    int idx = inventory_resolve_index(blood_group);
    if (idx == -1) {
        printf(COLOR_RED "  [!] Error: Blood Group unrecognized.\n" COLOR_RESET);
        return false;
    }

    if (inv->stocks[idx].units < count) {
        printf(COLOR_RED "  [!] Quota Deficit: Requested %u units of %s, but only %u are available.\n" COLOR_RESET,
               count, blood_group, inv->stocks[idx].units);
        return false;
    }

    inv->stocks[idx].units -= count;
    inv->total_units -= count;

    return inventory_save(inv);
}

/**
 * @brief Displays inventory analytics in a UTF-8 box matrix.
 */
void inventory_display_dashboard(const BloodInventory *inv) {
    if (inv == NULL) return;

    printf("\n" COLOR_CYAN COLOR_BOLD);
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    CENTRAL BLOOD STOCK INVENTORY                           ║\n");
    printf("╠════════════════════╦═════════════════════════╦═════════════════════════════╣\n");
    printf("║    Blood Group     ║     Available Units     ║       Stock Status          ║\n");
    printf("╠════════════════════╬═════════════════════════╬═════════════════════════════╣\n");
    printf(COLOR_RESET);

    for (int i = 0; i < NUM_BLOOD_GROUPS; ++i) {
        uint32_t units = inv->stocks[i].units;
        const char *status_color;
        const char *status_str;

        if (units == 0) {
            status_color = COLOR_RED;
            status_str = "DEPLETED (CRITICAL) ";
        } else if (units < 5) {
            status_color = COLOR_YELLOW;
            status_str = "LOW STOCK RESERVE   ";
        } else {
            status_color = COLOR_GREEN;
            status_str = "OPTIMAL STORAGE     ";
        }

        printf("║        %-11s ║           %-13u ║ %s%-27s" COLOR_RESET " ║\n",
               inv->stocks[i].blood_group, units, status_color, status_str);
    }

    printf(COLOR_CYAN COLOR_BOLD);
    printf("╠════════════════════╩═════════════════════════╩═════════════════════════════╣\n");
    printf("║  Total Aggregated Units in Vault: %-40u ║\n", inv->total_units);
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
}