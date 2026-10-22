/**
 * @file utils.c
 * @brief Bulletproof input processing firewall and date computation algorithms.
 * @course CS-115 Computer Programming
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../include/utils.h"
#include "../include/inventory.h"

/**
 * @brief Reads a line safely from standard input, strictly discarding overflowing characters.
 */
bool safe_read_line(char *dest, size_t max_len) {
    if (dest == NULL || max_len == 0) {
        return false;
    }

    if (fgets(dest, (int)max_len, stdin) == NULL) {
        return false;
    }

    /* Locate the newline delimiter */
    char *newline_pos = strchr(dest, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0'; /* Strip newline */
    } else {
        /* Flush standard input buffer to clear lingering characters */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    /* Strip carriage return characters if parsed under Windows/CRLF environments */
    char *cr_pos = strchr(dest, '\r');
    if (cr_pos != NULL) {
        *cr_pos = '\0';
    }

    return true;
}

/**
 * @brief Firewall prompt for string ingestion.
 */
bool safe_read_string(const char *prompt, char *dest, size_t max_len) {
    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(dest, max_len)) {
            return false;
        }
        if (strlen(dest) == 0) {
            printf(COLOR_RED "  [!] Input cannot be blank. Please try again.\n" COLOR_RESET);
            continue;
        }
        return true;
    }
}

/**
 * @brief Firewall prompt for integer values bounded within a strict numerical range.
 */
bool safe_read_int(const char *prompt, int *dest, int min_val, int max_val) {
    char buffer[64];
    char extra;

    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(buffer, sizeof(buffer))) {
            return false;
        }

        if (sscanf(buffer, "%d %c", dest, &extra) != 1) {
            printf(COLOR_RED "  [!] Invalid format. Please enter a valid integer.\n" COLOR_RESET);
            continue;
        }

        if (*dest < min_val || *dest > max_val) {
            printf(COLOR_RED "  [!] Out of range [%d to %d]. Please re-enter.\n" COLOR_RESET, min_val, max_val);
            continue;
        }

        return true;
    }
}

/**
 * @brief Firewall prompt for unsigned 32-bit integers.
 */
bool safe_read_uint32(const char *prompt, uint32_t *dest, uint32_t min_val, uint32_t max_val) {
    char buffer[64];
    unsigned int temp;
    char extra;

    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(buffer, sizeof(buffer))) {
            return false;
        }

        if (sscanf(buffer, "%u %c", &temp, &extra) != 1) {
            printf(COLOR_RED "  [!] Invalid format. Please enter a valid positive integer.\n" COLOR_RESET);
            continue;
        }

        if (temp < min_val || temp > max_val) {
            printf(COLOR_RED "  [!] Out of bounds [%u to %u]. Please try again.\n" COLOR_RESET, min_val, max_val);
            continue;
        }

        *dest = (uint32_t)temp;
        return true;
    }
}

/**
 * @brief Firewall prompt for floating-point values bounded within a range.
 */
bool safe_read_double(const char *prompt, double *dest, double min_val, double max_val) {
    char buffer[64];
    char extra;

    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(buffer, sizeof(buffer))) {
            return false;
        }

        if (sscanf(buffer, "%lf %c", dest, &extra) != 1) {
            printf(COLOR_RED "  [!] Invalid decimal value. Please try again.\n" COLOR_RESET);
            continue;
        }

        if (*dest < min_val || *dest > max_val) {
            printf(COLOR_RED "  [!] Range error [%.1f to %.1f]. Please re-enter.\n" COLOR_RESET, min_val, max_val);
            continue;
        }

        return true;
    }
}

/**
 * @brief Firewall prompt for a single character from a restricted set.
 */
bool safe_read_char(const char *prompt, char *dest, const char *valid_chars) {
    char buffer[16];

    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(buffer, sizeof(buffer))) {
            return false;
        }

        if (strlen(buffer) != 1) {
            printf(COLOR_RED "  [!] Single character required.\n" COLOR_RESET);
            continue;
        }

        char candidate = (char)toupper((unsigned char)buffer[0]);
        bool matched = false;

        for (size_t i = 0; i < strlen(valid_chars); ++i) {
            if (candidate == (char)toupper((unsigned char)valid_chars[i])) {
                *dest = candidate;
                matched = true;
                break;
            }
        }

        if (matched) {
            return true;
        }

        printf(COLOR_RED "  [!] Invalid option. Expected one of: %s\n" COLOR_RESET, valid_chars);
    }
}

/**
 * @brief Validates leap year according to the Gregorian calendar.
 */
bool is_leap_year(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

/**
 * @brief Validates if a calendar date is structurally sound.
 */
bool is_valid_date(int day, int month, int year) {
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) {
        return false;
    }

    static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_day = days_in_month[month];

    if (month == 2 && is_leap_year(year)) {
        max_day = 29;
    }

    return (day <= max_day);
}

/**
 * @brief Prompts for and strictly validates a Gregorian date.
 */
bool safe_read_date(const char *prompt, Date *dest) {
    char buffer[32];
    int d, m, y;
    char extra;

    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(buffer, sizeof(buffer))) {
            return false;
        }

        if (sscanf(buffer, "%d/%d/%d %c", &d, &m, &y, &extra) != 3 &&
            sscanf(buffer, "%d-%d-%d %c", &d, &m, &y, &extra) != 3) {
            printf(COLOR_RED "  [!] Bad date format. Use DD/MM/YYYY or DD-MM-YYYY.\n" COLOR_RESET);
            continue;
        }

        if (!is_valid_date(d, m, y)) {
            printf(COLOR_RED "  [!] Non-existent calendar date provided. Try again.\n" COLOR_RESET);
            continue;
        }

        dest->day = d;
        dest->month = m;
        dest->year = y;
        return true;
    }
}

/**
 * @brief Validates input against accepted human blood groups.
 */
bool safe_read_blood_group(const char *prompt, char *dest) {
    char buffer[16];

    while (1) {
        printf("%s", prompt);
        if (!safe_read_line(buffer, sizeof(buffer))) {
            return false;
        }

        /* Normalize case */
        for (size_t i = 0; i < strlen(buffer); ++i) {
            buffer[i] = (char)toupper((unsigned char)buffer[i]);
        }

        if (inventory_resolve_index(buffer) != -1) {
            strncpy(dest, buffer, BLOOD_GROUP_STR_LEN - 1);
            dest[BLOOD_GROUP_STR_LEN - 1] = '\0';
            return true;
        }

        printf(COLOR_RED "  [!] Unknown group. Choose: A+, A-, B+, B-, AB+, AB-, O+, O-\n" COLOR_RESET);
    }
}

/**
 * @brief Converts a Gregorian calendar date to astronomical Julian Day Number (JDN).
 */
long date_to_julian_days(int day, int month, int year) {
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + (153 * m + 2) / 5 + 365L * y + y / 4 - y / 100 + y / 400 - 32045;
}

/**
 * @brief Calculates linear day difference between two dates.
 */
long days_between_dates(const Date *earlier, const Date *later) {
    long jdn1 = date_to_julian_days(earlier->day, earlier->month, earlier->year);
    long jdn2 = date_to_julian_days(later->day, later->month, later->year);
    return jdn2 - jdn1;
}

/**
 * @brief Retrieves the active system date.
 */
void get_current_date(Date *dest) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    dest->day = local->tm_mday;
    dest->month = local->tm_mon + 1;
    dest->year = local->tm_year + 1900;
}

/* UI Helpers */
void ui_clear_screen(void) {
    /* POSIX / ANSI terminal clear command sequence */
    printf("\033[2J\033[H");
}

void ui_pause(void) {
    printf(COLOR_YELLOW "\n  Press [Enter] to continue..." COLOR_RESET);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void ui_print_banner(void) {
    printf(COLOR_RED COLOR_BOLD);
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                 HEMOGLOBIN PRO: BLOOD BANK ARCHITECTURE                    ║\n");
    printf("║                CS-115 Advanced Systems Implementation                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
}

void ui_print_header(const char *title) {
    printf("\n" COLOR_CYAN COLOR_BOLD "───[ %s ]" COLOR_RESET "\n", title);
}

void ui_print_status(bool success, const char *message) {
    if (success) {
        printf(COLOR_GREEN "  [✓] %s\n" COLOR_RESET, message);
    } else {
        printf(COLOR_RED "  [✗] %s\n" COLOR_RESET, message);
    }
}