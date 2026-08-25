#include <stdio.h>

#include "args.h"
#include "xkb.h"
#include "wayland.h"

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

    if (!config.layout) {
        fprintf(stderr,
                "wkbmap: no layout specified\n");
        fprintf(stderr,
                "Try 'wkbmap --help' for more information.\n");

        free_config(&config);
        return 1;
    }

    if (!xkb_layout_exists(config.layout)) {
        fprintf(stderr,
                "wkbmap: unknown XKB layout: %s\n",
                config.layout);

        free_config(&config);
        return 1;
    }

    int result = wkbmap_set_layout(config.layout);

    free_config(&config);

    return result;
}
