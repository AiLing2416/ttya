#ifndef TTYD_UTIL_H
#define TTYD_UTIL_H

#include <stdbool.h>
#include <stddef.h>

#define container_of(ptr, type, member)                \
  ({                                                   \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); \
  })

// malloc with NULL check
void *xmalloc(size_t size);

// realloc with NULL check
void *xrealloc(void *p, size_t size);

// Convert a string to upper case
char *uppercase(char *s);

// Convert a string to lower case
char *lowercase(char *s);

// Check whether str ends with suffix
bool endswith(const char *str, const char *suffix);

// Constant-time string comparison
int timingsafe_strcmp(const char *s1, const char *s2);

// Get human readable signal string
int get_sig_name(int sig, char *buf, size_t len);

// Get signal code from string like SIGHUP
int get_sig(const char *sig_name);

// Check path is safe
bool check_path(const char *path, const char *base);

// Validate filename for upload
bool validate_filename(const char *filename);

// Open uri with the default application of system
int open_uri(char *uri);

#ifdef _WIN32
char *strsep(char **sp, char *sep);
const char *quote_arg(const char *arg);
void print_error(char *func);
#endif
#endif  // TTYD_UTIL_H
