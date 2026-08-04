#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"

int test_validate_filename(const char *filename, bool expected) {
    bool actual = validate_filename(filename);
    if (actual != expected) {
        printf("FAIL: validate_filename(\"%s\") expected %d, got %d\n", filename ? filename : "NULL", expected, actual);
        return 1;
    }
    printf("PASS: validate_filename(\"%s\") == %d\n", filename ? filename : "NULL", actual);
    return 0;
}

int main() {
    int failures = 0;

    printf("Testing validate_filename...\n");
    failures += test_validate_filename("test.txt", true);
    failures += test_validate_filename("my-file.123", true);
    failures += test_validate_filename("some.dir/file", false);
    failures += test_validate_filename("some\\file", false);
    failures += test_validate_filename("..", false);
    failures += test_validate_filename(".", false);
    failures += test_validate_filename("", false);
    failures += test_validate_filename(NULL, false);

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll security tests passed!\n");
    return 0;
}
