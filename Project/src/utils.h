#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

typedef struct {
    int year, month, day, hour, minute, second;
} DateTime;

DateTime now_dt(void);
void format_dt(DateTime d, char *out, size_t n);

void read_line(char *buf, size_t n);
int read_int(const char *prompt, int *out);
int read_double(const char *prompt, double *out);
void read_nonempty(const char *prompt, char *out, size_t n);

void str_to_lower(char *s);

#endif
