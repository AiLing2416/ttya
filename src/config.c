#include "config.h"
#include <json-c/json.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int parse_config_file(const char *config_path, struct server *server, struct lws_context_creation_info *info, char *iface, bool parse_command) {
    struct json_object *obj = json_object_from_file(config_path);
    if (!obj) {
        fprintf(stderr, "ttya: failed to read config file %s\n", config_path);
        return -1;
    }
    
    struct json_object *val;
    if (json_object_object_get_ex(obj, "port", &val)) {
        info->port = json_object_get_int(val);
    }
    if (json_object_object_get_ex(obj, "interface", &val)) {
        strncpy(iface, json_object_get_string(val), 127);
        iface[127] = '\0';
    }
    if (json_object_object_get_ex(obj, "credential", &val)) {
        const char *optarg_val = json_object_get_string(val);
        char *colon = strchr(optarg_val, ':');
        if (colon != NULL) {
            char b64_text[256];
            lws_b64_encode_string(optarg_val, (int)strlen(optarg_val), b64_text, sizeof(b64_text));
            server->credential = strdup(b64_text);
            size_t user_len = colon - optarg_val;
            server->username = malloc(user_len + 1);
            memcpy(server->username, optarg_val, user_len);
            server->username[user_len] = '\0';
        }
    }
    if (json_object_object_get_ex(obj, "cwd", &val)) {
        server->cwd = strdup(json_object_get_string(val));
    }
    if (json_object_object_get_ex(obj, "writable", &val)) {
        server->writable = json_object_get_boolean(val);
    }
    if (json_object_object_get_ex(obj, "title", &val)) {
        snprintf(server->title, sizeof(server->title), "%s", json_object_get_string(val));
    }
    if (parse_command && json_object_object_get_ex(obj, "command", &val)) {
        if (json_object_is_type(val, json_type_array)) {
            int arr_len = json_object_array_length(val);
            if (arr_len > 0) {
                if (server->argv != NULL) {
                    for (int i = 0; i < server->argc; i++) free(server->argv[i]);
                    free(server->argv);
                }
                server->argc = arr_len;
                server->argv = malloc(sizeof(char*) * (arr_len + 1));
                size_t cmd_len = 0;
                for (int i = 0; i < arr_len; i++) {
                    server->argv[i] = strdup(json_object_get_string(json_object_array_get_idx(val, i)));
                    cmd_len += strlen(server->argv[i]);
                    if (i != arr_len - 1) cmd_len++;
                }
                server->argv[arr_len] = NULL;
                free(server->command);
                server->command = malloc(cmd_len + 1);
                char *ptr = server->command;
                for (int i = 0; i < arr_len; i++) {
                    size_t len = strlen(server->argv[i]);
                    ptr = memcpy(ptr, server->argv[i], len + 1) + len;
                    if (i != arr_len - 1) *ptr++ = ' ';
                }
                *ptr = '\0';
            }
        }
    }
    json_object_put(obj);
    return 0;
}
