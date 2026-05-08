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

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
