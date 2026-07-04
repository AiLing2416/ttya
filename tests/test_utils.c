#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utils.h"

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

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

int test_xmalloc() {
    printf("Testing xmalloc...\n");
    void *p = xmalloc(100);
    if (p == NULL) {
        printf("FAIL: xmalloc(100) returned NULL\n");
        return 1;
    }
    free(p);
    printf("PASS: xmalloc(100) returned non-NULL\n");

    p = xmalloc(0);
    if (p != NULL) {
        printf("FAIL: xmalloc(0) returned non-NULL\n");
        return 1;
    }
    printf("PASS: xmalloc(0) returned NULL\n");
    return 0;
}

int test_xrealloc() {
    printf("Testing xrealloc...\n");
    void *p = xrealloc(NULL, 100);
    if (p == NULL) {
        printf("FAIL: xrealloc(NULL, 100) returned NULL\n");
        return 1;
    }
    printf("PASS: xrealloc(NULL, 100) returned non-NULL\n");

    void *p2 = xrealloc(p, 200);
    if (p2 == NULL) {
        printf("FAIL: xrealloc(p, 200) returned NULL\n");
        free(p);
        return 1;
    }
    printf("PASS: xrealloc(p, 200) returned non-NULL\n");

    void *p3 = xrealloc(p2, 50);
    if (p3 == NULL) {
        printf("FAIL: xrealloc(p2, 50) returned NULL\n");
        free(p2);
        return 1;
    }
    printf("PASS: xrealloc(p2, 50) returned non-NULL\n");

    void *p4 = xrealloc(p3, 0);
    if (p4 != NULL) {
        printf("FAIL: xrealloc(p3, 0) returned non-NULL\n");
        free(p4);
        return 1;
    }
    printf("PASS: xrealloc(p3, 0) returned NULL\n");

    void *p5 = xrealloc(NULL, 0);
    if (p5 != NULL) {
        printf("FAIL: xrealloc(NULL, 0) returned non-NULL\n");
        return 1;
    }
    printf("PASS: xrealloc(NULL, 0) returned NULL\n");
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

int test_validate_filename(const char *filename, bool expected) {
    bool actual = validate_filename(filename);
    if (actual != expected) {
        printf("FAIL: validate_filename(\"%s\") expected %d, got %d\n", filename ? filename : "NULL", expected, actual);
        return 1;
    }
    printf("PASS: validate_filename(\"%s\") == %d\n", filename ? filename : "NULL", actual);
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

    // Setup relative test directories
    if (MKDIR("./ttyd_test") != 0) {
        perror("mkdir ./ttyd_test");
    }
    if (MKDIR("./ttyd_test/sub") != 0) {
        perror("mkdir ./ttyd_test/sub");
    }
    if (MKDIR("./ttyd_test_other") != 0) {
        perror("mkdir ./ttyd_test_other");
    }
    if (MKDIR("./ttyd_test_prefix") != 0) {
        perror("mkdir ./ttyd_test_prefix");
    }
    FILE *f = fopen("./ttyd_test/file.txt", "w");
    if (f) {
        fclose(f);
    } else {
        perror("fopen ./ttyd_test/file.txt");
    }

    // Happy paths
    failures += test_check_path_case("./ttyd_test", "./ttyd_test", true);
    failures += test_check_path_case("./ttyd_test/sub", "./ttyd_test", true);
    failures += test_check_path_case("./ttyd_test/file.txt", "./ttyd_test", true);

    // Dot dot
    failures += test_check_path_case("./ttyd_test/sub/..", "./ttyd_test", true);

    // Negative cases
    failures += test_check_path_case("./ttyd_test_other", "./ttyd_test", false);
    failures += test_check_path_case("./ttyd_test/../ttyd_test_other", "./ttyd_test", false);

    // Prefix match but not in directory
    failures += test_check_path_case("./ttyd_test_prefix", "./ttyd_test", false);

    // Non-existent
    failures += test_check_path_case("./non_existent_path", "./ttyd_test", false);

    // Cleanup
    remove("./ttyd_test/file.txt");
    rmdir("./ttyd_test/sub");
    rmdir("./ttyd_test");
    rmdir("./ttyd_test_other");
    rmdir("./ttyd_test_prefix");

    return failures;
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

    printf("\n");
    failures += test_xmalloc();
    printf("\n");
    failures += test_xrealloc();

    printf("\nTesting validate_filename...\n");
    failures += test_validate_filename("test.txt", true);
    failures += test_validate_filename("my-file.123", true);
    failures += test_validate_filename("some.dir/file", false);
    failures += test_validate_filename("some\\file", false);
    failures += test_validate_filename("..", false);
    failures += test_validate_filename(".", false);
    failures += test_validate_filename("", false);
    failures += test_validate_filename(NULL, false);

    printf("\n");
    failures += test_check_path();

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
