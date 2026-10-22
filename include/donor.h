/**
 * @file donor.h
 * @brief Donor entity model, medical eligibility rules, and CRUD prototypes.
 * @course CS-115 Computer Programming
 */

#ifndef DONOR_H
#define DONOR_H

#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include "inventory.h"

#define DONOR_DB_FILE   "donors.dat"
#define MAX_NAME_LEN    48
#define MAX_PHONE_LEN   16

/* Primary Donor Record Structure */
typedef struct {
    uint32_t id;
    char name[MAX_NAME_LEN];
    int age;
    char gender; /* 'M', 'F', 'O' */
    double weight;
    char blood_group[BLOOD_GROUP_STR_LEN];
    char phone[MAX_PHONE_LEN];
    Date last_donation;
    bool has_donated_before;
} Donor;

/* Eligibility Business Rules */
#define MIN_DONOR_AGE          18
#define MAX_DONOR_AGE          65
#define MIN_DONOR_WEIGHT_KG    50.0
#define MIN_DONATION_INTERVAL  90 /* Mandatory safe interval between donations in days */

/* Core Operational Prototypes (Pass-by-Reference) */
bool donor_check_eligibility(int age, double weight, bool has_donated, const Date *last_donation, const Date *today);
bool donor_add(BloodInventory *inv);
bool donor_display_all(void);
bool donor_search_by_id(uint32_t target_id);
bool donor_search_by_blood_group(const char *target_group);
bool donor_update(uint32_t target_id);
bool donor_delete(uint32_t target_id);
uint32_t donor_generate_next_id(void);

#endif /* DONOR_H */