#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include "utils.h"

int test_endswith(const char *str, const char *suffix, bool expected) {
    bool actual = endswith(str, suffix);
    if (actual != expected) {
        printf("FAIL: endswith(\"%s\", \"%s\") expected %d, got %d\n", str, suffix, expected, actual);
        return 1;
    }
    printf("PASS: endswith(\"%s\", \"%s\") == %d\n", str, suffix, actual);
    return 0;
}

int test_get_sig_name(int sig, const char *expected) {
    char buf[32];
    get_sig_name(sig, buf, sizeof(buf));
    if (expected == NULL) {
        // Just check if it's SIGUNKNOWN or some other "SIG..." string when we don't know the exact name
        if (strncmp(buf, "SIG", 3) != 0) {
            printf("FAIL: get_sig_name(%d) expected SIG..., got \"%s\"\n", sig, buf);
            return 1;
        }
    } else if (strcmp(buf, expected) != 0) {
        printf("FAIL: get_sig_name(%d) expected \"%s\", got \"%s\"\n", sig, expected, buf);
        return 1;
    }
    printf("PASS: get_sig_name(%d) == \"%s\"\n", sig, buf);
    return 0;
}

int test_get_sig(const char *sig_name, int expected) {
    int actual = get_sig(sig_name);
    if (actual != expected) {
        printf("FAIL: get_sig(\"%s\") expected %d, got %d\n", sig_name, expected, actual);
        return 1;
    }
    printf("PASS: get_sig(\"%s\") == %d\n", sig_name, actual);
    return 0;
}

int main() {
    int failures = 0;

    printf("Testing endswith...\n");
    failures += test_endswith("hello.sock", ".sock", true);
    failures += test_endswith("file.txt", ".txt", true);
    failures += test_endswith("something", "thing", true);
    failures += test_endswith("hello.sock", ".socket", false);
    failures += test_endswith("file.txt", ".tx", false);
    failures += test_endswith("abc", "def", false);
    failures += test_endswith("abc", "abcd", false);
    failures += test_endswith("abc", "abc", true);
    failures += test_endswith("a", "a", true);
    failures += test_endswith("abc", "", true);
    failures += test_endswith("", "", true);

    printf("\nTesting get_sig_name...\n");
    failures += test_get_sig_name(SIGHUP, "SIGHUP");
    failures += test_get_sig_name(SIGINT, "SIGINT");
    failures += test_get_sig_name(SIGKILL, "SIGKILL");
    failures += test_get_sig_name(SIGTERM, "SIGTERM");
    failures += test_get_sig_name(0, "SIGZERO");
    // signal 999 is definitely unknown
    failures += test_get_sig_name(999, "SIGUNKNOWN");
    // signal -1 is definitely unknown
    failures += test_get_sig_name(-1, "SIGUNKNOWN");

    printf("\nTesting get_sig...\n");
    failures += test_get_sig("HUP", SIGHUP);
    failures += test_get_sig("SIGHUP", SIGHUP);
    failures += test_get_sig("hup", SIGHUP);
    failures += test_get_sig("sighup", SIGHUP);
    failures += test_get_sig("KILL", SIGKILL);
    failures += test_get_sig("SIGKILL", SIGKILL);

    char sigkill_buf[10];
    sprintf(sigkill_buf, "%d", SIGKILL);
    failures += test_get_sig(sigkill_buf, SIGKILL);

    failures += test_get_sig("invalid", 0);
    failures += test_get_sig("SI", 0);
    failures += test_get_sig("", 0);

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
