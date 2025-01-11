#ifndef NOISE_H
#define NOISE_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>

// /* These state variables must be initialized so that they are not all zero. */
struct xorShiftVariables{
    uint32_t x, y, z, w;
};

void xorShift128Init(struct xorShiftVariables* var) {
    var->x = rand();
    var->y = rand();
    var->z = rand();
    var->w = rand();
}

uint32_t xorShift128(struct xorShiftVariables* var) {
    uint32_t t = var->x ^ (var->x << 11);
    var->x = var->y; var->y = var->z; var->z = var->w;
    return var->w = var->w ^ (var->w >> 19) ^ t ^ (t >> 8);
}

// C++ Version, missing some of the addaptations above
// /* These state variables must be initialized so that they are not all zero. */
// uint32_t x, y, z, w;

// void initializeRandomVariables() {
//     x = rand();
//     y = rand();
//     z = rand();
//     w = rand();
// }
 
// uint32_t xorshift128(void) {
//     uint32_t t = x ^ (x << 11);
//     x = y; y = z; z = w;
//     return w = w ^ (w >> 19) ^ t ^ (t >> 8);
// }

#endif