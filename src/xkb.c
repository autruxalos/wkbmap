#include <stdio.h>

#include <xkbcommon/xkbcommon.h>

#include "xkb.h"

int xkb_layout_exists(const char *layout)
{
    struct xkb_context *context;
    struct xkb_keymap *keymap;

    struct xkb_rule_names names = {
        .rules = NULL,
        .model = NULL,
        .layout = layout,
        .variant = NULL,
        .options = NULL
    };

    context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (!context) {
        fprintf(stderr,
                "wkbmap: failed to create XKB context\n");
        return 0;
    }

    keymap = xkb_keymap_new_from_names(
        context,
        &names,
        XKB_KEYMAP_COMPILE_NO_FLAGS
    );

    if (!keymap) {
        xkb_context_unref(context);
        return 0;
    }

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    return 1;
}
