#include <wayland-server-protocol.h>

#ifndef ELVIRA_UTIL_BOX_H
#define ELVIRA_UTIL_BOX_H

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

struct Point {
    int x, y;

    bool operator==(Point &other) {
        return (x == other.x && y == other.y);
    }

    // Populates `dx` and `dy` with the x and y offsets from `other`.
    void offset_from(Point& other, int& dx, int& dy);

    double distance_to(Point& other);
};

struct Box {
    int x, y;
    int width, height;

    bool operator==(Box& other) {
        return (x == other.x &&
                y == other.y &&
                width == other.width &&
                height == other.height);
    }

    static Box empty();

    // Returns true if either the box's width or height are zero.
    bool is_empty();

    // Returns true iff the point is strictly within the box.
    bool contains_point(Point& pt);

    bool contains_box(Box& bx);

    Box intersect(Box& bx);

    Box transform(wl_output_transform transform, int width, int height);
};

struct FBox {
    double x, y;
    double width, height;
};

#endif