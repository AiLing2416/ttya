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

    if (failures > 0) {
        printf("%d tests failed!\n", failures);
        return 1;
    }
    printf("All tests passed!\n");
    return 0;
}
