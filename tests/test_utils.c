#include <stdio.h>
#include <stdlib.h>
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

    printf("\n");
    failures += test_xmalloc();
    printf("\n");
    failures += test_xrealloc();

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll tests passed!\n");
    return 0;
}
