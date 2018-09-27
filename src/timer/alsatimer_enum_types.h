#ifndef __ALSA_GOBJECT_ALSATIMER_ENUM_TYPES__H__
#define __ALSA_GOBJECT_ALSATIMER_ENUM_TYPES__H__

#include <sound/asound.h>

typedef enum {
    ALSATIMER_DEVICE_CLASS_ENUM_NONE    = SNDRV_TIMER_CLASS_NONE,
    ALSATIMER_DEVICE_CLASS_ENUM_SLAVE   = SNDRV_TIMER_CLASS_SLAVE,
    ALSATIMER_DEVICE_CLASS_ENUM_GLOBAL  = SNDRV_TIMER_CLASS_GLOBAL,
    ALSATIMER_DEVICE_CLASS_ENUM_CARD    = SNDRV_TIMER_CLASS_CARD,
    ALSATIMER_DEVICE_CLASS_ENUM_PCM     = SNDRV_TIMER_CLASS_PCM,
} ALSATimerDeviceClassEnum;

#endif
