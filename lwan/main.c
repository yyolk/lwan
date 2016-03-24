/*
 * lwan - simple web server
 * Copyright (c) 2012 Leandro A. F. Pereira <leandro@hardinfo.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "lwan.h"
#include "lwan-serve-files.h"

enum args {
    ARGS_FAILED,
    ARGS_USE_CONFIG,
    ARGS_SERVE_FILES
};

static enum args
parse_args(int argc, char *argv[], lwan_config_t *config, char *root)
{
    static const struct option opts[] = {
        { .name = "root", .has_arg = 1, .val = 'r' },
        { .name = "listen", .has_arg = 1, .val = 'l' },
        { .name = "help", .val = 'h' },
        { }
    };
    int c, optidx = 0;
    enum args result = ARGS_USE_CONFIG;

    while ((c = getopt_long(argc, argv, "hr:l:", opts, &optidx)) != -1) {
        switch (c) {
        case 'l':
            free(config->listener);
            config->listener = strdup(optarg);
            result = ARGS_SERVE_FILES;
            break;

        case 'r':
            memcpy(root, optarg, strnlen(optarg, PATH_MAX - 1) + 1);
            result = ARGS_SERVE_FILES;
            break;

        case 'h':
            printf("Usage: %s [--root /path/to/root/dir] [--listener addr:port]\n", argv[0]);
            printf("\t[--config]\n");
            printf("Serve files through HTTP.\n\n");
            printf("Defaults to listening on %s, serving from ./wwwroot.\n\n", config->listener);
            printf("Options:\n");
            printf("\t-r, --root      Path to serve files from (default: ./wwwroot).\n");
            printf("\t-l, --listener  Listener (default: %s).\n", config->listener);
            printf("\t-h, --help      This.\n");
            printf("\n");
            printf("Examples:\n");
            printf("  Serve system-wide documentation: %s -r /usr/share/doc\n", argv[0]);
            printf("        Serve on a different port: %s -l '*:1337'\n", argv[0]);
            printf("\n");
            printf("Report bugs at <https://github.com/lpereira/lwan>.\n");
            return ARGS_FAILED;

        default:
            printf("Run %s --help for usage information.\n", argv[0]);
            return ARGS_FAILED;
        }
    }

    return result;
}

int
main(int argc, char *argv[])
{
    lwan_t l;
    lwan_config_t c;
    char root[PATH_MAX];

    if (!getcwd(root, PATH_MAX))
        return 1;

    c = *lwan_get_default_config();
    c.listener = strdup("*:8080");

    switch (parse_args(argc, argv, &c, root)) {
    case ARGS_SERVE_FILES:
        lwan_status_info("Serving files from %s", root);
        lwan_init_with_config(&l, &c);

        const lwan_url_map_t map[] = {
            { .prefix = "/", SERVE_FILES(root) },
            { }
        };
        lwan_set_url_map(&l, map);
        break;
    case ARGS_USE_CONFIG:
        lwan_init(&l);
        break;
    case ARGS_FAILED:
        return EXIT_FAILURE;
    }

    lwan_main_loop(&l);
    lwan_shutdown(&l);

    return EXIT_SUCCESS;
}
