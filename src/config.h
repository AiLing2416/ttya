#ifndef TTYA_CONFIG_H
#define TTYA_CONFIG_H

#include <libwebsockets.h>
#include <stdbool.h>
#include "server.h"

int parse_config_file(const char *config_path, struct server *server, struct lws_context_creation_info *info, char *iface, bool parse_command);

#endif // TTYA_CONFIG_H
