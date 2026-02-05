#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

DateTime now_dt(void) {
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    DateTime d;
    d.year = lt->tm_year + 1900;
    d.month = lt->tm_mon + 1;
    d.day = lt->tm_mday;
    d.hour = lt->tm_hour;
    d.minute = lt->tm_min;
    d.second = lt->tm_sec;
    return d;
}

void format_dt(DateTime d, char *out, size_t n) {
    snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d",
             d.year, d.month, d.day, d.hour, d.minute, d.second);
}

void read_line(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
}

int read_int(const char *prompt, int *out) {
    char line[128];
    while (1) {
        printf("%s", prompt);
        read_line(line, sizeof(line));
        if (line[0] == '\0') return 0;
        char *end = NULL;
        long v = strtol(line, &end, 10);
        if (end && *end == '\0') {
            *out = (int)v;
            return 1;
        }
        printf("Invalid number. Try again.\n");
    }
}

int read_double(const char *prompt, double *out) {
    char line[128];
    while (1) {
        printf("%s", prompt);
        read_line(line, sizeof(line));
        if (line[0] == '\0') return 0;
        char *end = NULL;
        double v = strtod(line, &end);
        if (end && *end == '\0') {
            *out = v;
            return 1;
        }
        printf("Invalid amount. Try again.\n");
    }
}

void read_nonempty(const char *prompt, char *out, size_t n) {
    while (1) {
        printf("%s", prompt);
        read_line(out, n);
        if (strlen(out) > 0) return;
        printf("Value cannot be empty.\n");
    }
}

void str_to_lower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}
