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

static int set_value(char **destination, const char *value)
{
    char *copy = duplicate_string(value);

    if (!copy) {
        fprintf(stderr, "wkbmap: out of memory\n");
        return 1;
    }

    free(*destination);
    *destination = copy;

    return 0;
}

static int require_value(int argc, char **argv, int *index)
{
    if (*index + 1 >= argc) {
        fprintf(stderr, "wkbmap: option requires an argument: %s\n",
                argv[*index]);
        return 1;
    }

    (*index)++;
    return 0;
}

int parse_args(int argc, char **argv, WkbConfig *config)
{
    for (int i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-layout") ||
            !strcmp(argv[i], "--layout")) {

            if (require_value(argc, argv, &i))
                return 1;

            if (set_value(&config->layout, argv[i]))
                return 1;
        }

        else if (!strcmp(argv[i], "-variant") ||
                 !strcmp(argv[i], "--variant")) {

            if (require_value(argc, argv, &i))
                return 1;

            if (set_value(&config->variant, argv[i]))
                return 1;
        }

        else if (!strcmp(argv[i], "-option") ||
                 !strcmp(argv[i], "--option")) {

            if (require_value(argc, argv, &i))
                return 1;

            if (set_value(&config->options, argv[i]))
                return 1;
        }

        else if (!strcmp(argv[i], "-model") ||
                 !strcmp(argv[i], "--model")) {

            if (require_value(argc, argv, &i))
                return 1;

            if (set_value(&config->model, argv[i]))
                return 1;
        }

        else if (!strcmp(argv[i], "-rules") ||
                 !strcmp(argv[i], "--rules")) {

            if (require_value(argc, argv, &i))
                return 1;

            if (set_value(&config->rules, argv[i]))
                return 1;
        }

        else if (!strcmp(argv[i], "-query") ||
                 !strcmp(argv[i], "--query")) {
            config->query = 1;
        }

        else if (!strcmp(argv[i], "-h") ||
                 !strcmp(argv[i], "--help")) {
            config->help = 1;
        }

        else if (!strcmp(argv[i], "-V") ||
                 !strcmp(argv[i], "--version")) {
            config->version = 1;
        }

        /*
         * Shortcut:
         *
         *     wkbmap latam
         *
         * is equivalent to:
         *
         *     wkbmap -layout latam
         */
        else if (argv[i][0] != '-') {

            if (config->layout) {
                fprintf(stderr, "wkbmap: multiple layouts specified\n");
                return 1;
            }

            if (set_value(&config->layout, argv[i]))
                return 1;
        }

        else {
            fprintf(stderr, "wkbmap: unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    return 0;
}

void free_config(WkbConfig *config)
{
    free(config->rules);
    free(config->model);
    free(config->layout);
    free(config->variant);
    free(config->options);
}

void print_help(void)
{
    printf(
        "wkbmap - Wayland keyboard map utility\n"
        "\n"
        "Usage:\n"
        "  wkbmap [OPTIONS]\n"
        "  wkbmap LAYOUT\n"
        "\n"
        "Options:\n"
        "  -layout LAYOUT       Keyboard layout\n"
        "  -variant VARIANT     Keyboard variant\n"
        "  -option OPTION       Keyboard option\n"
        "  -model MODEL         Keyboard model\n"
        "  -rules RULES         XKB rules\n"
        "  -query               Show current configuration\n"
        "  -h, --help           Show this help\n"
        "  -V, --version        Show version\n"
        "\n"
        "Examples:\n"
        "  wkbmap latam\n"
        "  wkbmap -layout us\n"
        "  wkbmap -layout us -variant intl\n"
        "  wkbmap -layout us -option ctrl:nocaps\n"
    );
}

void print_version(void)
{
    printf("wkbmap 0.1.0\n");
}
