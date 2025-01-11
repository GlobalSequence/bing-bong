#ifndef ENVELOPE_H_
#define ENVELOPE_H_

#include <stdio.h>

// Uncomment this to use it as an AR envelope (No hold/sustain stage);
#define AR_ENVELOPE

const uint16_t res_env_portamento_increments[] = {
    65535, 18904, 16417, 14306, 12507, 10968, 9647, 8509, 
    7525, 6672, 5931, 5285, 4719, 4224, 3788, 3405, 
    3066, 2766, 2500, 2264, 2053, 1865, 1697, 1546, 
    1411, 1290, 1180, 1082, 993, 912, 839, 773, 
    713, 658, 608, 562, 521, 483, 448, 416, 
    387, 360, 335, 313, 292, 272, 255, 238, 
    223, 209, 196, 184, 172, 162, 152, 143, 
    135, 127, 119, 113, 106, 100, 95, 90, 
    85, 80, 76, 72, 68, 64, 61, 58, 
    55, 52, 50, 47, 45, 43, 41, 39, 
    37, 35, 33, 32, 30, 29, 28, 26, 
    25, 24, 23, 22, 21, 20, 19, 18, 
    18, 17, 16, 16, 15, 14, 14, 13, 
    13, 12, 12, 11, 11, 10, 10, 9, 
    9, 9, 8, 7, 7, 6, 6, 5, 
    5, 4, 4, 3, 3, 2, 2, 1
};


const uint8_t wav_res_env_expo[] = {
    0, 4, 9, 14, 19, 23, 28, 32,
    37, 41, 45, 49, 53, 57, 61, 65,
    68, 72, 76, 79, 83, 86, 89, 92,
    96, 99, 102, 105, 108, 111, 113, 116,
    119, 121, 124, 127, 129, 132, 134, 136,
    139, 141, 143, 145, 148, 150, 152, 154,
    156, 158, 160, 161, 163, 165, 167, 169,
    170, 172, 174, 175, 177, 178, 180, 181,
    183, 184, 186, 187, 188, 190, 191, 192,
    193, 195, 196, 197, 198, 199, 200, 201,
    202, 203, 205, 206, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 215, 216,
    217, 218, 218, 219, 220, 221, 221, 222,
    223, 223, 224, 225, 225, 226, 226, 227,
    227, 228, 229, 229, 230, 230, 231, 231,
    232, 232, 233, 233, 233, 234, 234, 235,
    235, 236, 236, 236, 237, 237, 238, 238,
    238, 239, 239, 239, 240, 240, 240, 241,
    241, 241, 241, 242, 242, 242, 243, 243,
    243, 243, 244, 244, 244, 244, 245, 245,
    245, 245, 245, 246, 246, 246, 246, 246,
    247, 247, 247, 247, 247, 248, 248, 248,
    248, 248, 248, 248, 249, 249, 249, 249,
    249, 249, 249, 250, 250, 250, 250, 250,
    250, 250, 250, 251, 251, 251, 251, 251,
    251, 251, 251, 251, 251, 252, 252, 252,
    252, 252, 252, 252, 252, 252, 252, 252,
    252, 253, 253, 253, 253, 253, 253, 253,
    253, 253, 253, 253, 253, 253, 253, 253,
    253, 254, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 254, 254, 254, 254, 255,
    255};

static inline uint16_t U8MixU16(uint8_t a, uint8_t b, uint8_t balance) {
  return a * (255 - balance) + b * balance;
}

static inline uint8_t U8MixConst(const uint8_t a, const uint8_t b, uint8_t balance) {
  return (a * (255 - balance) + (b * balance)) >> 8;
}

static inline uint8_t InterpolateSample(
    const uint8_t *table,
    uint16_t phase) {
  const uint8_t *tbl_ptr = table + (phase >> 8);
  const uint8_t *tbl_ptr_2 = 1 + table + (phase >> 8);

  return U8MixConst(*tbl_ptr, *tbl_ptr_2, phase & 0xff);
}


enum EnvelopeStage
{
#ifndef AR_ENVELOPE
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    DEAD,

    NUM_SEGMENTS
# else
    ATTACK,
    RELEASE,
    DEAD,

    DECAY,
    SUSTAIN,
    NUM_SEGMENTS
#endif
};

struct Env_Params {

    uint16_t stage_phase_increment_[NUM_SEGMENTS];
    uint16_t stage_target_[NUM_SEGMENTS];
    uint8_t stage_;
    uint8_t a_;
    uint8_t b_;
    uint16_t phase_increment_;
    uint16_t phase_;
    uint16_t value_;

};

void EnvInit(struct Env_Params* e)
{
    e->stage_target_[ATTACK] = 255;
    e->stage_target_[RELEASE] = 0;
    e->stage_target_[DEAD] = 0;
    e->stage_phase_increment_[SUSTAIN] = 0;
    e->stage_phase_increment_[DEAD] = 0;
}

void EnvTrigger(uint8_t stage, struct Env_Params* e)
{
    if (stage == DEAD)
    {
        e->value_ = 0;
    }
    e->a_ = e->value_ >> 8;
    e->b_ = e->stage_target_[stage];
    e->stage_ = stage;
    e->phase_ = 0;
}

inline void EnvUpdate(
    uint8_t attack,
    uint8_t decay,
    uint8_t sustain,
    uint8_t release,
    struct Env_Params* e)
{
    e->stage_phase_increment_[ATTACK] = res_env_portamento_increments[attack];
    e->stage_phase_increment_[DECAY] = res_env_portamento_increments[decay];
    e->stage_phase_increment_[RELEASE] = res_env_portamento_increments[release];
    e->stage_target_[DECAY] = sustain << 1;
    e->stage_target_[SUSTAIN] = e->stage_target_[DECAY];

    e->phase_increment_ = e->stage_phase_increment_[e->stage_];
}

uint16_t EnvRender(struct Env_Params* e)
{
    e->phase_ += e->phase_increment_;
    if (e->phase_ < e->phase_increment_)
    {
        e->value_ = U8MixU16(e->a_, e->b_, 255);
        EnvTrigger(++e->stage_, e);
    }
    if (e->phase_increment_)
    {
        uint8_t step = InterpolateSample(wav_res_env_expo, e->phase_);
        e->value_ = U8MixU16(e->a_, e->b_, step);
    }

    return e->stage_ == SUSTAIN ? e->stage_target_[DECAY] << 8 : e->value_;
}

// C++ Version, missing some adaptations above
/* 
class Envelope
{
public:
    Envelope() {}

    void init(void)
    {
        stage_target_[ATTACK] = 255;
        stage_target_[RELEASE] = 0;
        stage_target_[DEAD] = 0;
        stage_phase_increment_[SUSTAIN] = 0;
        stage_phase_increment_[DEAD] = 0;
    }

    uint8_t stage() { return stage_; }
    uint16_t value() { return value_; }

    void Trigger(uint8_t stage)
    {
        if (stage == DEAD)
        {
            value_ = 0;
        }
        a_ = value_ >> 8;
        b_ = stage_target_[stage];
        stage_ = stage;
        phase_ = 0;
        // phase_increment_ = stage_phase_increment_[stage];
    }

    inline void Update(
        uint8_t attack,
        uint8_t decay,
        uint8_t sustain,
        uint8_t release)
    {
        stage_phase_increment_[ATTACK] = res_env_portamento_increments[attack];
        stage_phase_increment_[DECAY] = res_env_portamento_increments[decay];
        stage_phase_increment_[RELEASE] = res_env_portamento_increments[release];
        stage_target_[DECAY] = sustain << 1;
        stage_target_[SUSTAIN] = stage_target_[DECAY];

        phase_increment_ = stage_phase_increment_[stage_];
    }

    uint16_t Render()
    {
        phase_ += phase_increment_;
        if (phase_ < phase_increment_)
        {
            value_ = U8MixU16(a_, b_, 255);
            Trigger(++stage_);
        }
        if (phase_increment_)
        {
            uint8_t step = InterpolateSample(wav_res_env_expo, phase_);
            value_ = U8MixU16(a_, b_, step);
        }

        return stage_ == SUSTAIN ? stage_target_[DECAY] << 4 : value_ >> 4;
    }

private:
    uint16_t stage_phase_increment_[NUM_SEGMENTS];
    uint16_t stage_target_[NUM_SEGMENTS];
    uint8_t stage_;
    uint8_t a_;
    uint8_t b_;
    uint16_t phase_increment_;
    uint16_t phase_;
    uint16_t value_;
};
 */
#endif