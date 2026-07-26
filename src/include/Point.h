#ifndef POINT_H
#define POINT_H

#include <stdint.h>

/// Describe the location of a point at Terminal.
typedef struct Point {
    uint64_t x;   //< → crosswise
    uint64_t y;   //< ↓ vertical
} Point;

#endif
