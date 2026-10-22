/**
 * @file inventory.h
 * @brief Blood stock data structures, tracking, and persistence interface.
 * @course CS-115 Computer Programming
 */

#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define INVENTORY_DB_FILE   "inventory.dat"
#define NUM_BLOOD_GROUPS    8
#define BLOOD_GROUP_STR_LEN 4

/* Supported Blood Group Catalog */
extern const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS];

/* Blood Stock Record for an Individual Group */
typedef struct {
    char blood_group[BLOOD_GROUP_STR_LEN];
    uint32_t units;
} BloodStock;

/* Aggregated Inventory Management Structure */
typedef struct {
    BloodStock stocks[NUM_BLOOD_GROUPS];
    uint32_t total_units;
} BloodInventory;

/* Inventory Lifecycle & Operations */
bool inventory_init(BloodInventory *inv);
bool inventory_load(BloodInventory *inv);
bool inventory_save(const BloodInventory *inv);
bool inventory_add_unit(BloodInventory *inv, const char *blood_group, uint32_t count);
bool inventory_deduct_unit(BloodInventory *inv, const char *blood_group, uint32_t count);
void inventory_display_dashboard(const BloodInventory *inv);
int  inventory_resolve_index(const char *blood_group);

#endif /* INVENTORY_H */