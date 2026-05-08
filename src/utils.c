#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#if defined(__linux__) && !defined(__ANDROID__)
const char *sys_signame[NSIG] = {"zero", "HUP",   "INT",  "QUIT", "ILL",  "TRAP", "ABRT", "UNUSED", "FPE",
                                 "KILL", "USR1",  "SEGV", "USR2", "PIPE", "ALRM", "TERM", "STKFLT", "CHLD",
                                 "CONT", "STOP",  "TSTP", "TTIN", "TTOU", "URG",  "XCPU", "XFSZ",   "VTALRM",
                                 "PROF", "WINCH", "IO",   "PWR",  "SYS",  NULL};
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#undef NSIG
#define NSIG 33
const char *sys_signame[NSIG] = {"zero", "HUP",   "INT",  "QUIT", "ILL",  "TRAP", "IOT",  "EMT",  "FPE",
                                 "KILL", "BUS",   "SEGV", "SYS",  "PIPE", "ALRM", "TERM", "URG",  "STOP",
                                 "TSTP", "CONT",  "CHLD", "TTIN", "TTOU", "IO",   "XCPU", "XFSZ", "VTALRM",
                                 "PROF", "WINCH", "PWR",  "USR1", "USR2", NULL};
#endif

void *xmalloc(size_t size) {
  if (size == 0) return NULL;
  void *p = malloc(size);
  if (!p) abort();
  return p;
}

void *xrealloc(void *p, size_t size) {
  if ((size == 0) && (p == NULL)) return NULL;
  p = realloc(p, size);
  if (!p) abort();
  return p;
}

static char *string_transform(char *s, int (*transform)(int)) {
  while (*s) {
    *s = (char)transform((int)*s);
    s++;
  }
  return s;
}

char *uppercase(char *s) { return string_transform(s, toupper); }

char *lowercase(char *s) { return string_transform(s, tolower); }

bool endswith(const char *str, const char *suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);
  return str_len >= suffix_len && !strcmp(str + (str_len - suffix_len), suffix);
}

int timingsafe_strcmp(const char *s1, const char *s2) {
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  size_t len = len1 < len2 ? len1 : len2;
  size_t res = 0;

  for (size_t i = 0; i <= len; i++) {
    res |= (size_t)((unsigned char)s1[i] ^ (unsigned char)s2[i]);
  }

  return (int)(res | (len1 ^ len2));
}

int get_sig_name(int sig, char *buf, size_t len) {
  int n = snprintf(buf, len, "SIG%s", sig < NSIG ? sys_signame[sig] : "unknown");
  uppercase(buf);
  return n;
}

int get_sig(const char *sig_name) {
  for (int sig = 1; sig < NSIG; sig++) {
    const char *name = sys_signame[sig];
    if (name != NULL && (strcasecmp(name, sig_name) == 0 || strcasecmp(name, sig_name + 3) == 0)) return sig;
  }
  return atoi(sig_name);
}

bool check_path(const char *path, const char *base) {
#ifdef _WIN32
  char *abs_base = _fullpath(NULL, base, 0);
#else
  char *abs_base = realpath(base, NULL);
#endif
  if (!abs_base) return false;

#ifdef _WIN32
  char *abs_path = _fullpath(NULL, path, 0);
#else
  char *abs_path = realpath(path, NULL);
#endif
  bool ret = false;

  if (abs_path) {
    if (strncmp(abs_path, abs_base, strlen(abs_base)) == 0) {
      char next_char = abs_path[strlen(abs_base)];
      if (next_char == '\0' || next_char == '/' || next_char == '\\') {
        ret = true;
      }
    }
    free(abs_path);
  }

  free(abs_base);
  return ret;
}

#ifndef _WIN32
static int run_command(char *const argv[]) {
  pid_t pid = fork();
  if (pid < 0) return -1;

  if (pid == 0) {
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null != -1) {
      dup2(dev_null, STDIN_FILENO);
      dup2(dev_null, STDOUT_FILENO);
      dup2(dev_null, STDERR_FILENO);
      close(dev_null);
    }
    execvp(argv[0], argv);
    _exit(127);
  }

  int status;
  if (waitpid(pid, &status, 0) == -1) return -1;
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  return -1;
}
#endif

int open_uri(char *uri) {
#ifdef __APPLE__
  char *argv[] = {"open", uri, NULL};
  return run_command(argv);
#elif defined(_WIN32) || defined(__CYGWIN__)
  return ShellExecute(0, 0, uri, 0, 0, SW_SHOW) > (HINSTANCE)32 ? 0 : 1;
#else
  // check if X server is running
  char *xset_argv[] = {"xset", "-q", NULL};
  if (run_command(xset_argv)) return 1;
  char *xdg_open_argv[] = {"xdg-open", uri, NULL};
  return run_command(xdg_open_argv);
#endif
}

#ifdef _WIN32
char *strsep(char **sp, char *sep) {
  char *p, *s;
  if (sp == NULL || *sp == NULL || **sp == '\0') return (NULL);
  s = *sp;
  p = s + strcspn(s, sep);
  if (*p != '\0') *p++ = '\0';
  *sp = p;
  return s;
}

const char *quote_arg(const char *arg) {
  int len = 0, n = 0;
  int force_quotes = 0;
  char *q, *d;
  const char *p = arg;
  if (!*p) force_quotes = 1;
  while (*p) {
    if (isspace(*p) || *p == '*' || *p == '?' || *p == '{' || *p == '\'')
      force_quotes = 1;
    else if (*p == '"')
      n++;
    else if (*p == '\\') {
      int count = 0;
      while (*p == '\\') {
        count++;
        p++;
        len++;
      }
      if (*p == '"' || !*p) n += count * 2 + 1;
      continue;
    }
    len++;
    p++;
  }
  if (!force_quotes && n == 0) return arg;

  d = q = xmalloc(len + n + 3);
  *d++ = '"';
  while (*arg) {
    if (*arg == '"')
      *d++ = '\\';
    else if (*arg == '\\') {
      int count = 0;
      while (*arg == '\\') {
        count++;
        *d++ = *arg++;
      }
      if (*arg == '"' || !*arg) {
        while (count-- > 0) *d++ = '\\';
        if (!*arg) break;
        *d++ = '\\';
      }
    }
    *d++ = *arg++;
  }
  *d++ = '"';
  *d++ = '\0';
  return q;
}

void print_error(char *func) {
  LPVOID buffer;
  DWORD dw = GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, NULL);
  wprintf(L"== %s failed with error %d: %s", func, dw, buffer);
  LocalFree(buffer);
}
#endif
