#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int open_uri(char *uri);

int main() {
    char *vulnerable_uri = "http://example.com; touch /tmp/pwned";
    printf("Opening URI: %s\n", vulnerable_uri);
    open_uri(vulnerable_uri);
    return 0;
}
