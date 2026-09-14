#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "args.h"
#include "xkb.h"
#include "wkbmap-protocol.h"

struct wkb_state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_seat *seat;
    struct xkb_layout_manager_v1 *manager;
    struct xkb_layout_keyboard_v1 *keyboard;

    const char *requested_layout;
    int layout_received;
};

static void keyboard_layout(
    void *data,
    struct xkb_layout_keyboard_v1 *keyboard,
    const char *layout)
{
    struct wkb_state *state = data;

    (void)keyboard;

    if (!strcmp(layout, state->requested_layout))
        state->layout_received = 1;
}

static const struct xkb_layout_keyboard_v1_listener keyboard_listener = {
    .layout = keyboard_layout
};

static void registry_global(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{
    struct wkb_state *state = data;

    if (!strcmp(interface, "wl_seat")) {
        state->seat =
            wl_registry_bind(
                registry,
                name,
                &wl_seat_interface,
                version < 7 ? version : 7
            );
    }

    else if (!strcmp(interface, "xkb_layout_manager_v1")) {
        state->manager =
            wl_registry_bind(
                registry,
                name,
                &xkb_layout_manager_v1_interface,
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

    if (parse_args(argc, argv, &config))
        return EXIT_FAILURE;

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
                "wkbmap: invalid XKB layout: %s\n",
                config.layout);

        free_config(&config);
        return EXIT_FAILURE;
    }

    struct wkb_state state = {0};

    state.requested_layout = config.layout;

    state.display = wl_display_connect(NULL);

    if (!state.display) {
        fprintf(stderr,
                "wkbmap: failed to connect to Wayland display\n");

        free_config(&config);
        return EXIT_FAILURE;
    }

    state.registry = wl_display_get_registry(state.display);

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
                "wkbmap: compositor does not implement xkb-layout-v1\n");

        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    if (!state.seat) {
        fprintf(stderr,
                "wkbmap: compositor did not provide a Wayland seat\n");

        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    state.keyboard =
        xkb_layout_manager_v1_get_keyboard(
            state.manager,
            state.seat
        );

    xkb_layout_keyboard_v1_add_listener(
        state.keyboard,
        &keyboard_listener,
        &state
    );

    xkb_layout_keyboard_v1_set_layout(
        state.keyboard,
        config.layout
    );

    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr,
                "wkbmap: failed to communicate with compositor\n");

        xkb_layout_keyboard_v1_destroy(state.keyboard);
        xkb_layout_manager_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    if (!state.layout_received) {
        fprintf(stderr,
                "wkbmap: compositor did not apply layout: %s\n",
                config.layout);

        xkb_layout_keyboard_v1_destroy(state.keyboard);
        xkb_layout_manager_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        free_config(&config);
        return EXIT_FAILURE;
    }

    printf("layout: %s\n", config.layout);

    xkb_layout_keyboard_v1_destroy(state.keyboard);
    xkb_layout_manager_v1_destroy(state.manager);
    wl_display_disconnect(state.display);

    free_config(&config);

    return EXIT_SUCCESS;
}
