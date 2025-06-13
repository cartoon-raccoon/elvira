#include <cmath>

#include "box.hpp"

void Point::offset_from(Point& other, int& dx, int& dy) {
    dx = other.x - x;
    dy = other.y - y;
}

double Point::distance_to(Point& other) {
    int dx {}, dy {};
    offset_from(other, dx, dy);

    return sqrt(pow((double) dx, 2) + pow((double) dy,2));
}



Box Box::empty() {
    return Box {0, 0, 0, 0};
}

bool Box::is_empty() {
    return this->width <= 0 || this->height <= 0;
}

bool Box::contains_point(Point& pt) {
    if (is_empty())
        return false;

    return (
        (pt.x > this->x && pt.x < this->x + width) &&
        (pt.y > this->y && pt.y < this->y + height)
    );
}

Box Box::transform(wl_output_transform transform, int width, int height) {
    Box ret {};

    if (transform % 2 == 0) {
        ret.width = this->width;
        ret.height = this->height;
    } else {
        ret.width = this->height;
        ret.height = this->width;
    }

    //todo

    return ret;
}