#include <stdio.h>

#include <xkbcommon/xkbcommon.h>

#include "xkb.h"

int xkb_validate(const WkbConfig *config)
{
    struct xkb_context *context;
    struct xkb_keymap *keymap;

    struct xkb_rule_names names = {
        .rules = config->rules,
        .model = config->model,
        .layout = config->layout,
        .variant = config->variant,
        .options = config->options
    };

    context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (!context) {
        fprintf(stderr, "wkbmap: failed to create XKB context\n");
        return 1;
    }

    keymap = xkb_keymap_new_from_names(
        context,
        &names,
        XKB_KEYMAP_COMPILE_NO_FLAGS
    );

    if (!keymap) {
        fprintf(stderr, "wkbmap: failed to compile XKB keymap\n");
        xkb_context_unref(context);
        return 1;
    }

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    return 0;
}
