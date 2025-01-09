#ifndef PM_OSC_H
#define PM_OSC_H

#include <stdio.h>
#include "bing_bong_math.h"

struct PM_Osc_Variables {
    uint32_t phase;
    int32_t freq;
};

void PMOscSetup(struct PM_Osc_Variables* p) {
    p->phase = 0;
}

void PMOscUpdateFreq(int32_t pitch, struct PM_Osc_Variables* p) {
    MTOFEXTENDED(pitch, p->freq);
}

int32_t PMOscRender(int32_t phase_mod, struct PM_Osc_Variables* p) {
    p->phase += p->freq;
    int32_t r;
    int32_t p2 = p->phase + (phase_mod << 4);
    SINE2TINTERP(p2, r);
    return r >> 4;
}

#endif