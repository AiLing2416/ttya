#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
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
    if (strcmp(buf, expected) != 0) {
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
    // We use indices instead of macros like SIGHUP because they are not available on all platforms (e.g. MinGW)
    failures += test_get_sig_name(1, "SIGHUP");
    failures += test_get_sig_name(2, "SIGINT");
    failures += test_get_sig_name(9, "SIGKILL");
    failures += test_get_sig_name(15, "SIGTERM");
    failures += test_get_sig_name(0, "SIGZERO");
    // signal 999 is definitely unknown
    failures += test_get_sig_name(999, "SIGUNKNOWN");
    // signal -1 is definitely unknown
    failures += test_get_sig_name(-1, "SIGUNKNOWN");

    printf("\nTesting get_sig...\n");
    failures += test_get_sig("HUP", 1);
    failures += test_get_sig("SIGHUP", 1);
    failures += test_get_sig("hup", 1);
    failures += test_get_sig("sighup", 1);
    failures += test_get_sig("KILL", 9);
    failures += test_get_sig("SIGKILL", 9);
    failures += test_get_sig("9", 9);
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
