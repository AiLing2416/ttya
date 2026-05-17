#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
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

int test_timingsafe_strcmp(const char *s1, const char *s2, int expected_match) {
    int actual = timingsafe_strcmp(s1, s2);
    int match = (actual == 0);
    if (match != expected_match) {
        printf("FAIL: timingsafe_strcmp(\"%s\", \"%s\") expected match %d, got match %d (return value: %d)\n", s1, s2, expected_match, match, actual);
        return 1;
    }
    printf("PASS: timingsafe_strcmp(\"%s\", \"%s\") == %s (return value: %d)\n", s1, s2, match ? "match" : "mismatch", actual);
    return 0;
}

int test_open_uri_injection() {
    printf("Testing open_uri for command injection...\n");
    // Ensure clean state
    remove("/tmp/pwned_ttya");

    // This should not create the file /tmp/pwned_ttya
    char *injection_uri = "http://localhost; touch /tmp/pwned_ttya";
    open_uri(injection_uri);

    FILE *f = fopen("/tmp/pwned_ttya", "r");
    if (f) {
        printf("FAIL: Command injection successful! /tmp/pwned_ttya was created.\n");
        fclose(f);
        remove("/tmp/pwned_ttya");
        return 1;
    }
    printf("PASS: Command injection failed (as expected).\n");
    return 0;
}

int test_check_path_case(const char *path, const char *base, bool expected) {
    bool actual = check_path(path, base);
    if (actual != expected) {
        printf("FAIL: check_path(\"%s\", \"%s\") expected %d, got %d\n", path, base, expected, actual);
        return 1;
    }
    printf("PASS: check_path(\"%s\", \"%s\") == %d\n", path, base, actual);
    return 0;
}

int test_check_path() {
    printf("Testing check_path...\n");
    int failures = 0;

    // Setup
    if (mkdir("/tmp/ttyd_test", 0755) != 0) {
        perror("mkdir /tmp/ttyd_test");
    }
    if (mkdir("/tmp/ttyd_test/sub", 0755) != 0) {
        perror("mkdir /tmp/ttyd_test/sub");
    }
    if (mkdir("/tmp/ttyd_test_other", 0755) != 0) {
        perror("mkdir /tmp/ttyd_test_other");
    }
    if (mkdir("/tmp/ttyd_test_prefix", 0755) != 0) {
        perror("mkdir /tmp/ttyd_test_prefix");
    }
    FILE *f = fopen("/tmp/ttyd_test/file.txt", "w");
    if (f) {
        fclose(f);
    } else {
        perror("fopen /tmp/ttyd_test/file.txt");
    }

    // Happy paths
    failures += test_check_path_case("/tmp/ttyd_test", "/tmp/ttyd_test", true);
    failures += test_check_path_case("/tmp/ttyd_test/sub", "/tmp/ttyd_test", true);
    failures += test_check_path_case("/tmp/ttyd_test/file.txt", "/tmp/ttyd_test", true);

    // Dot dot
    failures += test_check_path_case("/tmp/ttyd_test/sub/..", "/tmp/ttyd_test", true);

    // Negative cases
    failures += test_check_path_case("/tmp/ttyd_test_other", "/tmp/ttyd_test", false);
    failures += test_check_path_case("/tmp/ttyd_test/../ttyd_test_other", "/tmp/ttyd_test", false);
    failures += test_check_path_case("/", "/tmp/ttyd_test", false);

    // Prefix match but not in directory
    failures += test_check_path_case("/tmp/ttyd_test_prefix", "/tmp/ttyd_test", false);

    // Non-existent
    failures += test_check_path_case("/tmp/non_existent_path", "/tmp/ttyd_test", false);

    // Cleanup
    remove("/tmp/ttyd_test/file.txt");
    rmdir("/tmp/ttyd_test/sub");
    rmdir("/tmp/ttyd_test");
    rmdir("/tmp/ttyd_test_other");
    rmdir("/tmp/ttyd_test_prefix");

    return failures;
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

    printf("\nTesting timingsafe_strcmp...\n");
    // Happy paths (match)
    failures += test_timingsafe_strcmp("hello", "hello", 1);
    failures += test_timingsafe_strcmp("", "", 1);
    failures += test_timingsafe_strcmp("a", "a", 1);
    failures += test_timingsafe_strcmp("verylongstringthatshouldmatch", "verylongstringthatshouldmatch", 1);

    // Mismatches (equal length)
    failures += test_timingsafe_strcmp("hello", "hellp", 0);
    failures += test_timingsafe_strcmp("abcde", "abXde", 0);
    failures += test_timingsafe_strcmp("X", "Y", 0);

    // Mismatches (different length)
    failures += test_timingsafe_strcmp("hello", "hell", 0);
    failures += test_timingsafe_strcmp("hell", "hello", 0);
    failures += test_timingsafe_strcmp("", "a", 0);
    failures += test_timingsafe_strcmp("a", "", 0);

    failures += test_open_uri_injection();

    failures += test_check_path();

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
