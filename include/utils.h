/**
 * @file utils.h
 * @brief Utility declarations, ANSI styling, input firewall, and date manipulation.
 * @course CS-115 Computer Programming
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ANSI Terminal Styling Codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"

/* Date Structure */
typedef struct {
    int day;
    int month;
    int year;
} Date;

/* Safe Input Firewall Functions */
bool safe_read_line(char *dest, size_t max_len);
bool safe_read_string(const char *prompt, char *dest, size_t max_len);
bool safe_read_int(const char *prompt, int *dest, int min_val, int max_val);
bool safe_read_uint32(const char *prompt, uint32_t *dest, uint32_t min_val, uint32_t max_val);
bool safe_read_double(const char *prompt, double *dest, double min_val, double max_val);
bool safe_read_char(const char *prompt, char *dest, const char *valid_chars);
bool safe_read_date(const char *prompt, Date *dest);
bool safe_read_blood_group(const char *prompt, char *dest);

/* Date Arithmetic Functions */
bool is_leap_year(int year);
bool is_valid_date(int day, int month, int year);
long date_to_julian_days(int day, int month, int year);
long days_between_dates(const Date *earlier, const Date *later);
void get_current_date(Date *dest);

/* UI & Formatting Helpers */
void ui_clear_screen(void);
void ui_pause(void);
void ui_print_banner(void);
void ui_print_header(const char *title);
void ui_print_status(bool success, const char *message);

#endif /* UTILS_H */