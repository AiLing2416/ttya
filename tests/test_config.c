#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "config.h"

// Define stubs for global variables that config.h/server.h might depend on if any.
struct lws_context *context = NULL;
struct server *server_inst = NULL;
struct endpoints endpoints = {0};
volatile bool force_exit = false;

int main() {
    struct server srv;
    memset(&srv, 0, sizeof(srv));
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    char iface[128] = "";

    const char *test_json = "{"
        "\"port\": 8080,"
        "\"interface\": \"127.0.0.1\","
        "\"credential\": \"user:pass\","
        "\"cwd\": \"/tmp\","
        "\"writable\": true,"
        "\"title\": \"test terminal\","
        "\"command\": [\"echo\", \"hello\"]"
    "}";
    FILE *f = fopen("test_config.json", "w");
    if (!f) {
        perror("fopen test_config.json");
        return 1;
    }
    fputs(test_json, f);
    fclose(f);

    int ret = parse_config_file("test_config.json", &srv, &info, iface, true);
    assert(ret == 0);
    assert(info.port == 8080);
    assert(strcmp(iface, "127.0.0.1") == 0);
    assert(srv.writable == true);
    assert(strcmp(srv.cwd, "/tmp") == 0);
    assert(strcmp(srv.title, "test terminal") == 0);
    assert(srv.argc == 2);
    assert(strcmp(srv.argv[0], "echo") == 0);
    assert(strcmp(srv.argv[1], "hello") == 0);
    assert(strcmp(srv.command, "echo hello") == 0);
    assert(srv.credential != NULL);
    assert(strcmp(srv.username, "user") == 0);

    remove("test_config.json");
    
    // Cleanup
    free(srv.cwd);
    free(srv.username);
    free(srv.credential);
    free(srv.command);
    for (int i = 0; i < srv.argc; i++) free(srv.argv[i]);
    free(srv.argv);

    printf("parse_config_file parse_command=true test passed!\n");

    memset(&srv, 0, sizeof(srv));
    memset(&info, 0, sizeof(info));
    iface[0] = '\0';
    
    f = fopen("test_config2.json", "w");
    if (!f) {
        perror("fopen test_config2.json");
        return 1;
    }
    fputs(test_json, f);
    fclose(f);

    ret = parse_config_file("test_config2.json", &srv, &info, iface, false);
    assert(ret == 0);
    assert(info.port == 8080);
    assert(srv.argc == 0);
    assert(srv.command == NULL);

    remove("test_config2.json");

    // Cleanup
    free(srv.cwd);
    free(srv.username);
    free(srv.credential);

    printf("parse_config_file parse_command=false test passed!\n");

    return 0;
}
