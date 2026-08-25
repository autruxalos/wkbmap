#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "wayland.h"
#include "wkbmap-protocol.h"

struct wkbmap_state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wkbmap_manager_unstable_v1 *manager;

    int done;
    int failed;
};

static void handle_done(
    void *data,
    struct wkbmap_manager_unstable_v1 *manager)
{
    struct wkbmap_state *state = data;

    (void)manager;

    state->done = 1;
}

static void handle_failed(
    void *data,
    struct wkbmap_manager_unstable_v1 *manager,
    uint32_t reason)
{
    struct wkbmap_state *state = data;

    (void)manager;
    (void)reason;

    state->failed = 1;
}

static const struct wkbmap_manager_unstable_v1_listener manager_listener = {
    .done = handle_done,
    .failed = handle_failed
};

static void registry_global(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{
    struct wkbmap_state *state = data;

    if (strcmp(interface, "wkbmap_manager_unstable_v1") == 0) {
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

int wkbmap_set_layout(const char *layout)
{
    struct wkbmap_state state = {0};

    state.display = wl_display_connect(NULL);

    if (!state.display) {
        fprintf(stderr,
                "wkbmap: failed to connect to Wayland\n");
        return 1;
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
        return 1;
    }

    if (!state.manager) {
        fprintf(stderr,
                "wkbmap: compositor does not support wkbmap\n");

        wl_display_disconnect(state.display);
        return 1;
    }

    wkbmap_manager_unstable_v1_set_layout(
        state.manager,
        layout
    );

    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr,
                "wkbmap: failed communicating with compositor\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        return 1;
    }

    if (state.failed) {
        fprintf(stderr,
                "wkbmap: compositor rejected layout\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        return 1;
    }

    if (!state.done) {
        fprintf(stderr,
                "wkbmap: compositor did not apply layout\n");

        wkbmap_manager_unstable_v1_destroy(state.manager);
        wl_display_disconnect(state.display);
        return 1;
    }

    wkbmap_manager_unstable_v1_destroy(state.manager);
    wl_display_disconnect(state.display);

    return 0;
}
