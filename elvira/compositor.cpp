#include <wayland-server-core.h>

#include "compositor.hpp"

void Compositor::init() {
    display = wl_display_create();
    evloop = wl_event_loop_create();

    backend = wlr_backend_autocreate(evloop, NULL);
    drmfd = wlr_backend_get_drm_fd(backend);

    renderer = wlr_gles2_renderer_create_with_drm_fd(drmfd);
    wlr_renderer_init_wl_display(renderer, display);

    allocator = wlr_allocator_autocreate(backend, renderer);
}

void Compositor::start_compositor() {
    

}

void Compositor::shutdown() {

    wl_display_destroy_clients(display);

    wlr_allocator_destroy(allocator);
    wlr_renderer_destroy(renderer);
    wlr_backend_destroy(backend);
    wl_display_destroy(display);
}
