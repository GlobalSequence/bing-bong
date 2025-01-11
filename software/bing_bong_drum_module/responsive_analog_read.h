#ifndef RESPONSIVE_ANALOG_READ_H
#define RESPONSIVE_ANALOG_READ_H

/* 
* This library is adapted from dxinteractive's responsive analog read library for Arduino
* which can be found here: https://github.com/dxinteractive/ResponsiveAnalogRead
 */

#include <stdio.h>
#include <stdbool.h>

typedef struct ResponsiveAnalogRead {
    int analogResolution;
    float snapMultiplier;
    int pin;
    bool sleepEnable;
    float activityThreshold;
    bool edgeSnapEnable;

    float smoothValue;
    unsigned long lastActivityMS;
    float errorEMA;
    bool sleeping;

    int rawValue;
    int responsiveValue;
    int prevResponsiveValue;
    bool responsiveValueHasChanged;
} ResponsiveAnalogRead;

// Constructor equivalent for C (initialize the struct)
// void ResponsiveAnalogRead_init(ResponsiveAnalogRead* obj, bool sleepEnable, float snapMultiplier);

#ifdef __cplusplus
 extern "C" {
#endif
// Methods
void ResponsiveAnalogRead_begin(ResponsiveAnalogRead* obj, bool sleepEnable, float snapMultiplier);
float ResponsiveAnalogRead_snapCurve(ResponsiveAnalogRead* obj, float x);
int ResponsiveAnalogRead_getValue(ResponsiveAnalogRead* obj);
int ResponsiveAnalogRead_getRawValue(ResponsiveAnalogRead* obj);
bool ResponsiveAnalogRead_hasChanged(ResponsiveAnalogRead* obj);
bool ResponsiveAnalogRead_isSleeping(ResponsiveAnalogRead* obj);
void ResponsiveAnalogRead_update(ResponsiveAnalogRead* obj);
void ResponsiveAnalogRead_updateWithRawValue(ResponsiveAnalogRead* obj, int rawValueRead);
void ResponsiveAnalogRead_setSnapMultiplier(ResponsiveAnalogRead* obj, float newMultiplier);
void ResponsiveAnalogRead_enableSleep(ResponsiveAnalogRead* obj);
void ResponsiveAnalogRead_disableSleep(ResponsiveAnalogRead* obj);
void ResponsiveAnalogRead_enableEdgeSnap(ResponsiveAnalogRead* obj);
void ResponsiveAnalogRead_disableEdgeSnap(ResponsiveAnalogRead* obj);
void ResponsiveAnalogRead_setActivityThreshold(ResponsiveAnalogRead* obj, float newThreshold);
void ResponsiveAnalogRead_setAnalogResolution(ResponsiveAnalogRead* obj, int resolution);
#ifdef __cplusplus
}
#endif

#endif
