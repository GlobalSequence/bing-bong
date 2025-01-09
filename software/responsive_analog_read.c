#include <stdio.h>
#include <math.h>
#include "responsive_analog_read.h"

void ResponsiveAnalogRead_begin(ResponsiveAnalogRead* obj, bool sleepEnable, float snapMultiplier) {
    obj->sleepEnable = sleepEnable;
    ResponsiveAnalogRead_setSnapMultiplier(obj, snapMultiplier);
}

float ResponsiveAnalogRead_snapCurve(ResponsiveAnalogRead* obj, float x) {
    float y = 1.0 / (x + 1.0);
    y = (1.0 - y) * 2.0;
    if (y > 1.0) {
        return 1.0;
    }
    return y;
}

void ResponsiveAnalogRead_setSnapMultiplier(ResponsiveAnalogRead* obj, float newMultiplier) {
    if (newMultiplier > 1.0) {
        newMultiplier = 1.0;
    }
    if (newMultiplier < 0.0) {
        newMultiplier = 0.0;
    }
    obj->snapMultiplier = newMultiplier;
}

int ResponsiveAnalogRead_getResponsiveValue(ResponsiveAnalogRead* obj, int newValue) {
    if (obj->sleepEnable && obj->edgeSnapEnable) {
        if (newValue < obj->activityThreshold) {
            newValue = (newValue * 2) - obj->activityThreshold;
        } else if (newValue > obj->analogResolution - obj->activityThreshold) {
            newValue = (newValue * 2) - obj->analogResolution + obj->activityThreshold;
        }
    }

    unsigned int diff = abs(newValue - obj->smoothValue);

    obj->errorEMA += ((newValue - obj->smoothValue) - obj->errorEMA) * 0.4;

    if (obj->sleepEnable) {
        obj->sleeping = abs(obj->errorEMA) < obj->activityThreshold;
    }

    if (obj->sleepEnable && obj->sleeping) {
        return (int)obj->smoothValue;
    }

    float snap = ResponsiveAnalogRead_snapCurve(obj, diff * obj->snapMultiplier);

    if (obj->sleepEnable) {
        snap *= 0.5 + 0.5;
    }

    obj->smoothValue += (newValue - obj->smoothValue) * snap;

    if (obj->smoothValue < 0.0) {
        obj->smoothValue = 0.0;
    } else if (obj->smoothValue > obj->analogResolution - 1) {
        obj->smoothValue = obj->analogResolution - 1;
    }

    return (int)obj->smoothValue;
}

void ResponsiveAnalogRead_updateWithRawValue(ResponsiveAnalogRead* obj, int rawValueRead) {
    obj->rawValue = rawValueRead;
    obj->prevResponsiveValue = obj->responsiveValue;
    obj->responsiveValue = ResponsiveAnalogRead_getResponsiveValue(obj, obj->rawValue);
    obj->responsiveValueHasChanged = obj->responsiveValue != obj->prevResponsiveValue;
}