#ifndef SVF_H
#define SVF_H

#include <stdio.h>
#include "bing_bong_math.h"

enum Filter_Type{
    LOW_PASS,
    HIGH_PASS,
    BAND_PASS
};

struct SVF_Variables{
    int32_t damp;
    int32_t freq;
    int32_t low;
    int32_t band;
};

static inline void SVFSetCutoff(int32_t input, struct SVF_Variables* v) {
        int32_t alpha;
        MTOFEXTENDED(input, alpha);
        SINE2TINTERP(alpha, v->freq);
}

static inline void SVFSetResonance(int32_t input, struct SVF_Variables* v) {
        v->damp = (0x80 << 24) - (__USAT(input, 27) << 4);
        v->damp = ___SMMUL(v->damp, v->damp);
}

static inline int32_t SVFProcess(int32_t in_sample, enum Filter_Type type, struct SVF_Variables* v) {

    /* 
    * Version of SVF without saturation of waveform. 
    * Clips very badly (and non-musically) at anything approaching high resonance.  
    */

    // int32_t notch = (in_sample - (___SMMUL(v->damp, v->band) << 1));
    // v->low = v->low + (___SMMUL(v->freq, v->band) << 1);
    // int32_t high = notch - v->low;
    // v->band = (___SMMUL(v->freq, high) << 1) + v->band;
    // return v->band;

    int32_t band2 = __SSAT(v->band, 31);
    int32_t notch = (in_sample - (___SMMUL(v->damp, band2) << 1));
    v->low = v->low + (___SMMUL(v->freq, band2) << 1);
    int32_t high = notch - v->low;
    v->band = (___SMMUL(v->freq, high) << 1) + band2;
    // return v->band;
    switch(type) {
        case LOW_PASS:
            return v->low;
            break;
        case HIGH_PASS:
            return high;
            break;
        case BAND_PASS:
            return v->band;
            break;
    }
    return 0;
}

#endif