#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "args.h"
#include "xkb.h"
#include "wkbmap-protocol.h"

struct wkb_state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wkbmap_manager_unstable_v1 *manager;
};

static void registry_global(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{
    struct wkb_state *state = data;

    if (!strcmp(interface, "wkbmap_manager_unstable_v1")) {
        state->manager =
            wl_registry_bind(
                registry,
                name,
                &wkbmap_manager_unstable_v1_interface,
                version < 1 ? version : 1
            );
    }
}

static void registry_global_remove(
    void *data,
    struct wl_registry *registry,
    uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove
};

int main(int argc, char **argv)
{
    WkbConfig config = {0};

    if (parse_args(argc, argv, &config)) {
        free_config(&config);
        return EXIT_FAILURE;
    }

    if (config.help) {
        print_help();
        free_config(&config);
        return EXIT_SUCCESS;
    }

    if (config.version) {
        print_version();
        free_config(&config);
        return EXIT_SUCCESS;
    }

    if (!config.layout) {
        fprintf(stderr,
                "wkbmap: no layout specified\n"
                "Try 'wkbmap --help' for usage.\n");
        free_config(&config);
        return EXIT_FAILURE;
    }

    if (!xkb_layout_exists(config.layout)) {
        fprintf(stderr,
                "wkbmap: failed to compile XKB keymap\n");
        free_config(&config);
        return EXIT_FAILURE;
    }

    struct wkb_state state = {0};

    state.display = wl_display_connect(NULL);

    if (!state.display) {
        fprintf(stderr,
                "wkbmap: failed to connect to Wayland display\n");
        free_config(&config);
        return EXIT_FAILURE;
    }

    state.registry = wl_display_get_registry(state.display);

    if (!state.registry) {
        fprintf(stderr,
                "wkbmap: failed to get Wayland registry\n");
        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    wl_registry_add_listener(
        state.registry,
        &registry_listener,
        &state
    );

    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr,
                "wkbmap: Wayland roundtrip failed\n");
        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    if (!state.manager) {
        fprintf(stderr,
                "wkbmap: wkbmap protocol is not supported by this compositor\n");
        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    wkbmap_manager_unstable_v1_set_layout(
        state.manager,
        config.layout
    );

    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr,
                "wkbmap: failed to send layout request\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    wkbmap_manager_unstable_v1_destroy(state.manager);
    wl_display_disconnect(state.display);

    free_config(&config);

    return EXIT_SUCCESS;
}
