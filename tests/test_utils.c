#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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

int test_uppercase(const char *input, const char *expected) {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    strncpy(buf, input, sizeof(buf) - 1);
    char *ret = uppercase(buf);
    if (strcmp(buf, expected) != 0) {
        printf("FAIL: uppercase(\"%s\") expected \"%s\", got \"%s\"\n", input, expected, buf);
        return 1;
    }
    if (ret != buf + strlen(expected)) {
        printf("FAIL: uppercase(\"%s\") returned wrong pointer\n", input);
        return 1;
    }
    printf("PASS: uppercase(\"%s\") == \"%s\"\n", input, buf);
    return 0;
}

int test_lowercase(const char *input, const char *expected) {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    strncpy(buf, input, sizeof(buf) - 1);
    char *ret = lowercase(buf);
    if (strcmp(buf, expected) != 0) {
        printf("FAIL: lowercase(\"%s\") expected \"%s\", got \"%s\"\n", input, expected, buf);
        return 1;
    }
    if (ret != buf + strlen(expected)) {
        printf("FAIL: lowercase(\"%s\") returned wrong pointer\n", input);
        return 1;
    }
    printf("PASS: lowercase(\"%s\") == \"%s\"\n", input, buf);
    return 0;
}

int test_get_sig_name(int sig, const char *expected) {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    get_sig_name(sig, buf, sizeof(buf));
    if (strcmp(buf, expected) != 0) {
        printf("FAIL: get_sig_name(%d) expected \"%s\", got \"%s\"\n", sig, expected, buf);
        return 1;
    }
    printf("PASS: get_sig_name(%d) == \"%s\"\n", sig, buf);
    return 0;
}

int main() {
    int failures = 0;

    printf("Testing endswith...\n");

    // Happy paths
    failures += test_endswith("hello.sock", ".sock", true);
    failures += test_endswith("file.txt", ".txt", true);
    failures += test_endswith("something", "thing", true);

    // Negative cases
    failures += test_endswith("hello.sock", ".socket", false);
    failures += test_endswith("file.txt", ".tx", false);
    failures += test_endswith("abc", "def", false);

    // Edge cases
    failures += test_endswith("abc", "abcd", false); // suffix longer than string
    failures += test_endswith("abc", "abc", true);   // identical strings
    failures += test_endswith("a", "a", true);       // single char identical
    failures += test_endswith("abc", "", true);      // empty suffix
    failures += test_endswith("", "", true);         // empty string and empty suffix

    printf("\nTesting uppercase...\n");
    // Happy paths
    failures += test_uppercase("hello", "HELLO");
    failures += test_uppercase("Hello", "HELLO");
    // Edge cases
    failures += test_uppercase("HELLO", "HELLO");
    failures += test_uppercase("123!@#", "123!@#");
    failures += test_uppercase("", "");
    failures += test_uppercase("a", "A");

    printf("\nTesting lowercase...\n");
    // Happy paths
    failures += test_lowercase("HELLO", "hello");
    failures += test_lowercase("Hello", "hello");
    // Edge cases
    failures += test_lowercase("hello", "hello");
    failures += test_lowercase("123!@#", "123!@#");
    failures += test_lowercase("", "");
    failures += test_lowercase("A", "a");

    printf("\nTesting get_sig_name...\n");
    // Happy paths
    failures += test_get_sig_name(1, "SIGHUP");
    failures += test_get_sig_name(2, "SIGINT");
    failures += test_get_sig_name(9, "SIGKILL");
    failures += test_get_sig_name(15, "SIGTERM");
    failures += test_get_sig_name(0, "SIGZERO");

    // Edge cases
    failures += test_get_sig_name(100, "SIGUNKNOWN");
    failures += test_get_sig_name(-1, "SIGUNKNOWN");
    failures += test_get_sig_name(32, "SIGUNKNOWN"); // NULL entry

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
