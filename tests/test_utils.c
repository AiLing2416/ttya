#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
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

int test_open_uri_injection() {
    printf("\nTesting open_uri for injection vulnerability...\n");
    const char *injected_file = "injected_file";
    unlink(injected_file);

    // Create fake xset and xdg-open that do nothing but succeed
    if (system("echo '#!/bin/sh\nexit 0' > xset && chmod +x xset") != 0) return 1;
    if (system("echo '#!/bin/sh\nexit 0' > xdg-open && chmod +x xdg-open") != 0) return 1;

    char *old_path = getenv("PATH");
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;

    size_t new_path_len = strlen(cwd) + 1 + (old_path ? strlen(old_path) : 0) + 1;
    char *new_path = malloc(new_path_len);
    if (new_path == NULL) return 1;

    snprintf(new_path, new_path_len, "%s:%s", cwd, old_path ? old_path : "");
    setenv("PATH", new_path, 1);
    free(new_path);

    char malicious_uri[] = "http://example.com;touch injected_file";
    open_uri(malicious_uri);

    struct stat st;
    int result = 0;
    if (stat(injected_file, &st) == 0) {
        printf("FAIL: open_uri is VULNERABLE to command injection! (injected_file was created)\n");
        unlink(injected_file);
        result = 1;
    } else {
        printf("PASS: open_uri does not appear to be vulnerable to this injection.\n");
    }

    unlink("xset");
    unlink("xdg-open");
    return result;
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

    failures += test_open_uri_injection();

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
