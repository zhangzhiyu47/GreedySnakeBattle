#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#define APP_VERSION "6.0.0"
#define CONFIG_VERSION "6.0.0"

static const uint64_t MIN_TERMINAL_WIDE = 45;
static const uint64_t MIN_TERMINAL_HIGH = 10;
static const uint64_t ROCKER_BAR_WIDTH = 10;

#define RANGE_EQUAL(expr, min, max) ((expr) >= (min) && (expr) <= (max))

#define UNUSED(var) ((void)(var))

#endif // CONSTANTS_H
