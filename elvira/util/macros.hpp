#ifndef ELVIRA_UTIL_MACROS_H
#define ELVIRA_UTIL_MACROS_H

#define new_px_region32(var) \
    pixman_region32_t var; \
    pixman_region32_init(&var);

#endif