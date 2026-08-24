#include <stdio.h>
#include <string.h>

#include "args.h"
#include "xkb.h"

int main(int argc, char **argv)
{
    WkbConfig config = {0};

    if (parse_args(argc, argv, &config))
        return 1;

    if (config.help) {
        print_help();
        free_config(&config);
        return 0;
    }

    if (config.version) {
        print_version();
        free_config(&config);
        return 0;
    }

    if (!config.layout &&
        !config.variant &&
        !config.options &&
        !config.model &&
        !config.rules &&
        !config.query) {

        fprintf(stderr,
                "wkbmap: no keyboard configuration specified\n");

        fprintf(stderr,
                "Try 'wkbmap --help' for more information.\n");

        free_config(&config);
        return 1;
    }

    if (config.query) {
        fprintf(stderr,
                "wkbmap: query is not implemented yet\n");

        free_config(&config);
        return 1;
    }

    if (xkb_validate(&config)) {
        free_config(&config);
        return 1;
    }

    printf("XKB configuration is valid.\n");

    if (config.layout)
        printf("layout: %s\n", config.layout);

    if (config.variant)
        printf("variant: %s\n", config.variant);

    if (config.options)
        printf("option: %s\n", config.options);

    if (config.model)
        printf("model: %s\n", config.model);

    if (config.rules)
        printf("rules: %s\n", config.rules);

    free_config(&config);

    return 0;
}
