#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pty.h"

int test_pty_buf_init_free() {
    char data[] = "hello";
    size_t len = strlen(data);
    pty_buf_t *buf = pty_buf_init(data, len);

    if (buf == NULL) {
        printf("FAIL: pty_buf_init returned NULL\n");
        return 1;
    }
    if (buf->len != len) {
        printf("FAIL: pty_buf_init set wrong len: %zu != %zu\n", buf->len, len);
        return 1;
    }
    if (memcmp(buf->base, data, len) != 0) {
        printf("FAIL: pty_buf_init copied wrong data\n");
        return 1;
    }

    pty_buf_free(buf);
    printf("PASS: pty_buf_init and pty_buf_free (normal case)\n");
    return 0;
}

int test_pty_buf_free_null() {
    // pty_buf_free should handle NULL gracefully
    pty_buf_free(NULL);
    printf("PASS: pty_buf_free (NULL case)\n");
    return 0;
}

int test_pty_buf_zero_len() {
    // pty_buf_init calls xmalloc(len). In this project, xmalloc(0) is guaranteed
    // to return NULL (see src/utils.c).
    pty_buf_t *buf = pty_buf_init(NULL, 0);
    if (buf == NULL) {
        printf("FAIL: pty_buf_init(NULL, 0) returned NULL (struct allocation failed)\n");
        return 1;
    }
    if (buf->len != 0) {
        printf("FAIL: pty_buf_init(NULL, 0) set wrong len: %zu\n", buf->len);
        return 1;
    }
    if (buf->base != NULL) {
        printf("FAIL: pty_buf_init(NULL, 0) base is not NULL (xmalloc(0) should return NULL)\n");
        return 1;
    }

    pty_buf_free(buf);
    printf("PASS: pty_buf_init and pty_buf_free (zero len case)\n");
    return 0;
}

int main() {
    int failures = 0;

    failures += test_pty_buf_init_free();
    failures += test_pty_buf_free_null();
    failures += test_pty_buf_zero_len();

    if (failures > 0) {
        printf("\n%d tests failed!\n", failures);
        return 1;
    }
    printf("\nAll pty tests passed!\n");
    return 0;
}
