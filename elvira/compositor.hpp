#include <any>
#include <vector>

extern "C" {
    #include <wlr/backend.h>
    #include <wlr/render/allocator.h>
    #include <wlr/render/wlr_renderer.h>
    #include <wlr/render/gles2.h>
    #include <wlr/types/wlr_output.h>
}

#include <wayland-server-core.h>

#include "scene/scenegraph.hpp"

#ifndef ELVIRA_COMPOSITOR_H
#define ELVIRA_COMPOSITOR_H

/* The central compositor that runs the entire thing. */
class Compositor {

    public:

    Compositor();
    ~Compositor();

    // Initializes the compositor, setting up background protocols.
    void init();
    // Starts the event loop, blocking until the compositor receives the signal to shut down.
    void start_compositor();
    // Shuts down the
    void shutdown();

    void add_listener();

    void new_output(wl_listener *l, std::any data);

    private:

    wl_display *display = nullptr;
    wl_event_loop *evloop = nullptr;
    int drmfd = 0;

    std::vector<wl_listener> listeners;

    wlr_backend *backend = nullptr;
    wlr_renderer *renderer = nullptr;
    wlr_allocator *allocator = nullptr;

};

#endif