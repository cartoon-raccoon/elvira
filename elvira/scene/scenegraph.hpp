#include <vector>

extern "C" {
    #include <pixman.h>
    #include <wayland-server-core.h>

    #include <wlr/types/wlr_output.h>
}

class SceneNode {

};

class SceneRect: SceneNode {

};

class SceneTree: SceneNode {

};

class SceneBuffer: SceneNode {

};

class SceneGraph {
    public:

    SceneTree root;
    std::vector<wlr_output> outputs;

};