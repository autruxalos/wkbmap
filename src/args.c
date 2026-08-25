#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"

static char *duplicate_string(const char *source)
{
    size_t length = strlen(source) + 1;
    char *copy = malloc(length);

    if (!copy)
        return NULL;

    memcpy(copy, source, length);

    return copy;
}

static int set_layout(WkbConfig *config, const char *layout)
{
    char *copy = duplicate_string(layout);

    if (!copy) {
        fprintf(stderr, "wkbmap: out of memory\n");
        return 1;
    }

    free(config->layout);
    config->layout = copy;

    return 0;
}

int parse_args(int argc, char **argv, WkbConfig *config)
{
    for (int i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-layout") ||
            !strcmp(argv[i], "--layout")) {

            if (i + 1 >= argc) {
                fprintf(stderr,
                        "wkbmap: -layout requires an argument\n");
                return 1;
            }

            i++;

            if (set_layout(config, argv[i]))
                return 1;
        }

        else if (!strcmp(argv[i], "-h") ||
                 !strcmp(argv[i], "--help")) {

            config->help = 1;
        }

        else if (!strcmp(argv[i], "-V") ||
                 !strcmp(argv[i], "--version")) {

            config->version = 1;
        }

        else {
            fprintf(stderr,
                    "wkbmap: unknown option: %s\n",
                    argv[i]);
            return 1;
        }
    }

    return 0;
}

void free_config(WkbConfig *config)
{
    free(config->layout);
}

void print_help(void)
{
    printf(
        "wkbmap - Wayland keyboard layout utility\n"
        "\n"
        "Usage:\n"
        "  wkbmap -layout LAYOUT\n"
        "\n"
        "Options:\n"
        "  -layout LAYOUT       Set keyboard layout\n"
        "  -h, --help           Show this help\n"
        "  -V, --version        Show version\n"
        "\n"
        "Examples:\n"
        "  wkbmap -layout us\n"
        "  wkbmap -layout latam\n"
        "  wkbmap -layout es\n"
        "  wkbmap -layout de\n"
        "  wkbmap -layout fr\n"
    );
}

void print_version(void)
{
    printf("wkbmap 0.1.0\n");
}
