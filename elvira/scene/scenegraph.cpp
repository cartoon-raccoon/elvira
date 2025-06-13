#include <cassert>
#include <vector>
#include <algorithm>

#include <pixman.h>
#include <wlr/util/region.h>

#include "scenegraph.hpp"
#include "../util/macros.hpp"
#include "../util/box.hpp"

struct SceneUpdateData {
    pixman_region32_t *visible;
    pixman_region32_t *update_region;

    Box update_box;
    std::vector<SceneOutput>& outputs;

    bool calculate_visibility;
};

static uint32_t region_area(pixman_region32_t *region) {
	uint32_t area = 0;

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(region, &nrects);
	for (int i = 0; i < nrects; ++i) {
		area += (rects[i].x2 - rects[i].x1) * (rects[i].y2 - rects[i].y1);
	}

	return area;
}

static void scale_region(pixman_region32_t *region, float scale, bool round_up) {
    wlr_region_scale(region, region, scale);

    if (round_up && floor(scale) != scale) {
        wlr_region_expand(region, region, 1);
    }
}

void SceneNode::set_enabled(bool enabled) {
    if (this->enabled == enabled) return;
}

void SceneNode::set_position(int x, int y) {

}

void SceneNode::reparent(SceneNode& new_parent) {
    new_parent.add_child(*this);
}

void SceneNode::add_child(SceneNode new_child) {
    this->children.push_back(new_child);
    new_child.parent = this;
}

bool SceneNode::get_coords(int& lx, int& ly) {
    lx = x;
    lx = y;
    return enabled;
}

void SceneNode::foreach_node_in_box(Box box, SceneNodeIterFunc f) {
    int x {}, y {};
    this->get_coords(x, y);

    this->rec_foreach_node_in_box(box, f, x, y);
}

void SceneNode::update(pixman_region32_t *damage = nullptr) {
    int x {}, y {};
    if (!get_coords(x, y)) {
        if (damage) {
            // if the node is enabled
            this->scene.update_region(damage);
            this->scene.damage_outputs(damage);
            pixman_region32_fini(damage);
        }

        return;
    }

    pixman_region32_t visible;
    if (!damage) {
        pixman_region32_init(&visible);
        visibility(&visible);
        damage = &visible;
    }

    assert(damage);

    new_px_region32(update_region);
    pixman_region32_copy(&update_region, damage);
    get_bounds(x, y, &update_region);

    this->scene.update_region(&update_region);
    pixman_region32_fini(&update_region);

    this->visibility(damage);
    this->scene.damage_outputs(damage);
    pixman_region32_fini(damage);
}

void SceneNode::visibility(pixman_region32_t *visible) {
    if (!this->enabled)
        return;

    for (auto child : this->children) {
        child.visibility(visible);
    }

    pixman_region32_union(visible, visible, this->visible);
}

void SceneNode::get_bounds(int x, int y, pixman_region32_t *visible) {
    if (!enabled)
        return;

    for (auto child : this->children) {
        child.get_bounds(x + child.x, y + child.y, visible);
    }

    int width, height;
    this->get_size(width, height);
    pixman_region32_union_rect(visible, visible, x, y, width, height);
}

void SceneNode::destroy_prelude() {
    // emit this as the very first thing, in case listeners want to remove
    // children from the scenegraph
    wl_signal_emit_mutable(&base_events.destroy, NULL);

    this->set_enabled(false);
}

void SceneNode::rec_foreach_node_in_box(Box box, SceneNodeIterFunc iter, int lx, int ly) {
    if (!this->enabled) {
        return;
    }

    // recursively call ourselves in each child
    std::for_each(this->children.rbegin(), this->children.rend(), 
     [this, box, iter, lx, ly](auto node){
            this->rec_foreach_node_in_box(box, iter, lx, ly);
        }
    );

    Box node_box = { .x = lx, .y = ly };
    this->get_size(node_box.width, node_box.height);

    if (!node_box.intersect(box).is_empty()) {
        iter(*this, lx, ly);
    }
}

bool SceneRect::get_size(int& width, int& height) {
    width = this->width;
    height = this->height;

    return this->enabled;
}

void SceneRect::update_node_update_outputs(
        std::vector<SceneOutput>& outputs,
        SceneOutput *ignore,
        SceneOutput *force
) {
    return;
}

bool SceneBuffer::get_size(int& width, int& height) {
    // todo

    return this->enabled;
}

void SceneBuffer::update_node_update_outputs(
        std::vector<SceneOutput>& outputs,
        SceneOutput *ignore,
        SceneOutput *force
) {
    // todo
}

void SceneGraph::update_region(pixman_region32_t *update_region) {
    new_px_region32(visible);
    pixman_region32_copy(&visible, update_region);

    pixman_box32 *region_box = pixman_region32_extents(update_region);
    SceneUpdateData data = {
        &visible,
        update_region,
        Box {
            region_box->x1,
            region_box->y1,
            region_box->x2 - region_box->x1,
            region_box->y2 - region_box -> y1
        },
        this->outputs,
        this->calculate_visibility,
    };

    this->root.foreach_node_in_box(data.update_box, 
     [data](SceneNode& node, int lx, int ly) {
            Box box = { .x = lx, .y = ly};
            node.get_size(box.width, box.height);

            pixman_region32_subtract(node.visible, node.visible, data.update_region);
            pixman_region32_union(node.visible, node.visible, data.visible);
            pixman_region32_intersect_rect(node.visible, node.visible,
                lx, ly, box.width, box.height);

            if (data.calculate_visibility) {
                new_px_region32(opaque);
                node.opaque_region(lx, ly, &opaque);
                pixman_region32_subtract(data.visible, data.visible, &opaque);
                pixman_region32_fini(&opaque);
            }

            node.update_node_update_outputs(data.outputs, nullptr, nullptr);
        });
}

void SceneGraph::damage_outputs(pixman_region32_t *damage) {
    if (pixman_region32_empty(damage))
        return;

    for (auto scene_output : this->outputs) {
        new_px_region32(output_damage);

        pixman_region32_copy(&output_damage, damage);
        pixman_region32_translate(&output_damage, scene_output.x, scene_output.y);
        scale_region(&output_damage, scene_output.output.scale, true);
    }
}