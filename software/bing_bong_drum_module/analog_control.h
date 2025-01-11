#ifndef ANALOG_CONTROL_H
#define ANALOG_CONTROL_H

#include <stdio.h>
#define ADC_DEFAULT_RESOLUTION 12
// Define the analog resolution here 
// We choose 8 bits for a value 0-255
#define ADC_SCALED_RESOLUTION 8
#define Q28 1<<27
#define ANALOG_INPUT_TRUNCATION_BITS (ADC_DEFAULT_RESOLUTION - ADC_SCALED_RESOLUTION)
#define ANALOG_CONTROL_BIT_WIDTH (ADC_DEFAULT_RESOLUTION - ANALOG_INPUT_TRUNCATION_BITS)
#define ANALOG_CONTROL_MAX_VAL ((1 << ANALOG_CONTROL_BIT_WIDTH) - 1)
#define ANALOG_CONTROL_SHIFT_BITS 27 - (ADC_SCALED_RESOLUTION);

uint32_t controlValPolar(uint32_t input) {
    return (input >> ANALOG_INPUT_TRUNCATION_BITS) << ANALOG_CONTROL_SHIFT_BITS;
}

int32_t controlValBipolar(uint32_t input) {
    return (controlValPolar(input) - (1 << 26)) << 1;
}

uint64_t controlValMapPositive(uint64_t input, uint64_t in_max, uint64_t out_max) {
    return (input * out_max) / in_max;
}

const uint8_t lock_threshold = 48;
const uint8_t measure_threshold = 32;
const unsigned long time_out_length = 100L;

struct LockPotParams {
  bool locked = true;
  bool lock_clock_running;
  uint16_t last_val;
  uint16_t locked_val;
  uint8_t current_threshold;
  unsigned long clock_start_time;
};

uint16_t lockPotGetValue(uint16_t raw_val, unsigned long t, struct LockPotParams* l) {
  
  unsigned long time_elapsed = t - l->clock_start_time;
  if(!l->locked) {
    if(abs(raw_val - l->last_val) >= measure_threshold) {
      l->clock_start_time = t;
      // time_elapsed = 0;
      l->last_val = raw_val;
    }
    if(time_elapsed >= time_out_length) {
      l->locked_val = l->last_val;
      l->locked = true;
    }
    return raw_val;
  } else {
    if(abs(raw_val - l->locked_val) >= lock_threshold) {
      l->locked = false;
      return raw_val;
    } else {
      return l->locked_val;
    }
    return 0;
  }
}

#endif 