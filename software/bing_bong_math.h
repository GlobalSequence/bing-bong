#ifndef BING_BONG_MATH_H
#define BING_BONG_MATH_H

#include <Arduino.h>
#include <stdio.h>
#include <math.h>

#define SAMPLE_RATE 48000.0f
#define PI_F 3.1415927f
#define INT32_MAX 0x7FFFFFFFL

#define SINE2TSIZE 4096
extern const int32_t sine2t[SINE2TSIZE + 1];
// extern int32_t sine2t[SINE2TSIZE + 1];
#define PITCHTSIZE 257
extern uint32_t pitcht[PITCHTSIZE];

#ifdef __cplusplus
 extern "C" {
#endif
void bingBongMathInit();
#ifdef __cplusplus
}
#endif

inline int32_t ___SMMLA(int32_t op1, int32_t op2, int32_t op3) __attribute__((always_inline));
inline int32_t ___SMMUL(int32_t op1, int32_t op2) __attribute__((always_inline));
inline int32_t ___SMMLS(int32_t op1, int32_t op2, int32_t op3) __attribute__((always_inline));
inline uint32_t mtof48k_ext_q31(int32_t pitch) __attribute__((always_inline)); 
inline int32_t sin_q31(int32_t phase) __attribute__((always_inline));

int32_t ___SMMLA (int32_t op1, int32_t op2, int32_t op3)
{
  int32_t result;

  asm volatile ("smmla %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

int32_t ___SMMUL (int32_t op1, int32_t op2)
{
  int32_t result;

  asm volatile ("smmul %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2));
  return(result);
}

int32_t ___SMMLS (int32_t op1, int32_t op2, int32_t op3)
{
  int32_t result;

  asm volatile ("smmls %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

uint32_t mtof48k_ext_q31(int32_t pitch) {
    int32_t p=__SSAT(pitch,29);
    uint32_t pi = p>>21;
    int32_t y1 = pitcht[128+pi];
    int32_t y2 = pitcht[128+1+pi];
    int32_t pf= (p&0x1fffff)<<10;
    int32_t pfc = INT32_MAX - pf;
    uint32_t r;
    r = ___SMMUL(y1,pfc);
    r = ___SMMLA(y2,pf,r);
    uint32_t frequency = r<<1;
    return frequency;
}

int32_t sin_q31(int32_t phase) {
    uint32_t p = (uint32_t)(phase);
    uint32_t pi = p>>20;
    int32_t y1 = sine2t[pi];
    int32_t y2 = sine2t[1+pi];
    int32_t pf= (p&0xfffff)<<11;
    int32_t pfc = INT32_MAX - pf;
    int32_t rr;
    rr = ___SMMUL(y1,pfc);
    rr = ___SMMLA(y2,pf,rr);
    return rr<<1;
}

#define MTOFEXTENDED(pitch, frequency) \
  frequency = mtof48k_ext_q31(pitch);

#define SINE2TINTERP(phase, output) \
  output = sin_q31(phase);

#endif