extern "C" {
    #include <wayland-server-core.h>
    #include <wlr/backend.h>
    #include <wlr/render/allocator.h>
    #include <wlr/render/wlr_renderer.h>
    #include <wlr/types/wlr_output.h>
}

/* The central compositor that runs the entire thing. */
class Compositor {
    
    public:

    Compositor();
    ~Compositor();

    void init();
    void shutdown();

    private:

    wl_display *display = nullptr;
    wl_event_loop *evloop = nullptr;

    wlr_backend *backend = nullptr;
    wlr_renderer *renderer = nullptr;
    wlr_allocator *allocator = nullptr;

};