#include <any>
#include <vector>
#include <cstring>
#include <functional>

extern "C" {
    #include <pixman.h>
    #include <wayland-server-core.h>

    #include <wlr/types/wlr_output.h>
    #include <wlr/types/wlr_linux_dmabuf_v1.h>
    #include <wlr/types/wlr_buffer.h>
    #include <wlr/types/wlr_damage_ring.h>
}

#include "../output/output.hpp"
#include "../util/box.hpp"

#ifndef ELVIRA_SCENEGRAPH_H
#define ELVIRA_SCENEGRAPH_H

class SceneGraph;
class SceneOutput;
class SceneNode;
class SceneRect;
class SceneBuffer;

typedef std::function<void(SceneNode&, int, int)> SceneNodeIterFunc;

class SceneNode {
    public:
    SceneNode(SceneNode *parent, SceneGraph& scene): children(), data(), scene(scene) {
        wl_signal_init(&base_events.destroy);
        pixman_region32_init(visible);

        if (parent) {
            parent->add_child(*this);
        }
    };
    virtual ~SceneNode() {
        pixman_region32_fini(visible);
    };

    // Enable or disable this node. All its children inherit this state.
    void set_enabled(bool enabled);
    
    // Set the node's position relative to its parent. 
    void set_position(int x, int y);

    // Place the node above the specified sibling.
    void place_above(SceneNode& sibling);

    // Place the node below the specified sibling.
    void place_below(SceneNode& sibling);

    // Move the node to the top of all its sibling nodes.
    void move_to_top();

    // Move the node to the bottom of all its sibling nodes.
    void move_to_bottom();

    // Reparent the node under a new parent.
    void reparent(SceneNode& new_parent);

    // Add `new_child` to the Node's children.
    void add_child(SceneNode new_child);

    // Get the layout-local coordinates of the node.
    // True is returned if the node and all of its ancestors are enabled.
    bool get_coords(int& lx, int& ly);

    virtual bool get_size(int& width, int& height);

    //
    void foreach_node_in_box(Box box, SceneNodeIterFunc f);

    // Any user data.
    std::any data;

    // Events that can be emitted by the node.
    struct {
        wl_signal destroy;
    } base_events;

    bool enabled = false;
    int x, y; // relative to parent

    // may be null
    wlr_linux_dmabuf_v1 *linux_dmabuf_v1 = nullptr;

    protected:
    // The parent node, if any.
    SceneNode *parent = nullptr;
    // The scene that the node is a part of.
    SceneGraph& scene;

    std::vector<SceneNode> children;

    // The region that is visible and should be composited.
    pixman_region32_t *visible;

    void update(pixman_region32_t *damage);

    // Get the union of all visible regions for the node and its children.
    void visibility(pixman_region32_t *visible);

    // Sets `visible` to be the rectangle covered by the node and all its children.
    void get_bounds(int x, int y, pixman_region32_t *visible);

    void opaque_region(int lx, int ly, pixman_region32_t *opaque);

    virtual void update_node_update_outputs(
        std::vector<SceneOutput>& outputs, SceneOutput *ignore, SceneOutput *force
    );

    // Code that should be run for all subclasses before their respective destructors are run.
    // If inheriting from this class, always call this function from your destructor.
    void destroy_prelude();

    private:

    // Recursive call for foreach node in box
    void rec_foreach_node_in_box(Box box, SceneNodeIterFunc f, int lx, int ly);

    friend class SceneGraph;
}; // class SceneNode

class SceneRect: SceneNode {
    public:

    SceneRect(SceneNode *parent, SceneGraph& scene, int width, int height, const float color[4]):
        SceneNode {parent, scene},
        width{width},
        height{height}
    {
        std::memcpy(this->color, color, sizeof(float) * 4);

        this->update(nullptr);
    }

    ~SceneRect() {
        this->destroy_prelude();

        // subclass destructor code
    }

    int width {}, height {};

    float color[4] {};

    bool get_size(int& width, int& height);

    protected:

    void update_node_update_outputs(
        std::vector<SceneOutput>& outputs,
        SceneOutput *ignore,
        SceneOutput *force
    );
}; // class SceneRect

// A scenegraph node backed by a buffer.
class SceneBuffer: SceneNode {
    public:

    SceneBuffer(SceneNode *parent, SceneGraph& scene, wlr_buffer *buffer = nullptr): 
        SceneNode {parent, scene},
        buffer{buffer}
    {};

    ~SceneBuffer() {
        this->destroy_prelude();

        // subclass destructor code
    }

    wlr_buffer *buffer;

    struct {
        wl_signal outputs_update;
        wl_signal output_enter;
        wl_signal output_leave;
        wl_signal output_sample;
        wl_signal frame_done;
    } events;

    bool get_size(int& width, int& height);

    protected:

    void update_node_update_outputs(
        std::vector<SceneOutput>& outputs,
        SceneOutput *ignore,
        SceneOutput *force
    );

    private:
    
};


class SceneOutput {
    public:
    SceneOutput(Output& output, SceneGraph& scene): 
        output(output), scene(scene)
    {
        wlr_damage_ring_init(&this->damage_ring);
        pixman_region32_init(&this->pending_commit_damage);
    }

    ~SceneOutput();

    // The output that the SceneOutput corresponds to.
    Output& output;
    // The SceneGraph that owns the SceneOutput.
    SceneGraph& scene;

    wlr_damage_ring damage_ring;

    int x, y;

    struct {
        wl_signal destroy;
    } events;

    private:

    void damage(pixman_region32_t *damage);

    private:
    pixman_region32_t pending_commit_damage;
    
    int index;

    friend class SceneGraph;
};

class SceneGraph {
    public:

    SceneGraph(): outputs(), root(nullptr, *this) {
    };
    ~SceneGraph();

    SceneOutput& create_scene_output(Output& output);
    SceneRect& create_scene_rect();
    SceneBuffer& create_scene_buffer();

    SceneNode root;
    std::vector<SceneOutput> outputs;

    bool calculate_visibility;

    private:

    void update_region(pixman_region32_t *update_region);

    void damage_outputs(pixman_region32_t *damage);

    friend class Output;
    friend class SceneNode;

};

#endif