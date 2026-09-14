#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

#include "args.h"
#include "xkb.h"
#include "wkbmap-protocol.h"

struct wkb_state {
    struct wl_display *display;
    struct wkbmap_manager_unstable_v1 *manager;

    int done;
    int failed;
};

static void manager_done(
    void *data,
    struct wkbmap_manager_unstable_v1 *manager)
{
    (void)manager;

    struct wkb_state *state = data;
    state->done = 1;
}

static void manager_failed(
    void *data,
    struct wkbmap_manager_unstable_v1 *manager,
    uint32_t reason)
{
    (void)manager;
    (void)reason;

    struct wkb_state *state = data;
    state->failed = 1;
}

static const struct wkbmap_manager_unstable_v1_listener manager_listener = {
    .done = manager_done,
    .failed = manager_failed
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

        wkbmap_manager_unstable_v1_add_listener(
            state->manager,
            &manager_listener,
            state
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
    struct wkb_state state = {0};

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
        fprintf(stderr, "wkbmap: no layout specified\n");
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

    state.display = wl_display_connect(NULL);

    if (!state.display) {
        fprintf(stderr,
                "wkbmap: unable to connect to Wayland display\n");
        free_config(&config);
        return 1;
    }

    struct wl_registry *registry =
        wl_display_get_registry(state.display);

    wl_registry_add_listener(
        registry,
        &registry_listener,
        &state
    );

    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr,
                "wkbmap: Wayland communication failed\n");

        wl_display_disconnect(state.display);
        free_config(&config);
        return 1;
    }

    if (!state.manager) {
        fprintf(stderr,
                "wkbmap: compositor does not support wkbmap\n");

        wl_display_disconnect(state.display);
        free_config(&config);
        return 1;
    }

    wkbmap_manager_unstable_v1_set_layout(
        state.manager,
        config.layout
    );

    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr,
                "wkbmap: Wayland communication failed\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        free_config(&config);
        return 1;
    }

    if (state.failed) {
        fprintf(stderr,
                "wkbmap: compositor rejected layout\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        free_config(&config);
        return 1;
    }

    if (!state.done) {
        fprintf(stderr,
                "wkbmap: compositor did not confirm layout\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        free_config(&config);
        return 1;
    }

    printf("layout: %s\n", config.layout);

    wkbmap_manager_unstable_v1_destroy(state.manager);
    wl_display_disconnect(state.display);
    free_config(&config);

    return 0;
}
