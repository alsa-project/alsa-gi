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

typedef enum {
    ALSATIMER_DEVICE_SLAVE_CLASS_ENUM_NONE          = SNDRV_TIMER_SCLASS_NONE,
    ALSATIMER_DEVICE_SLAVE_CLASS_ENUM_APPLICATION   = SNDRV_TIMER_SCLASS_APPLICATION,
    ALSATIMER_DEVICE_SLAVE_CLASS_ENUM_SEQUENCER     = SNDRV_TIMER_SCLASS_SEQUENCER,
    ALSATIMER_DEVICE_SLAVE_CLASS_ENUM_OSS_SEQUENCER = SNDRV_TIMER_SCLASS_OSS_SEQUENCER,
} ALSATimerDeviceSlaveClassEnum;

typedef enum {
    ALSATIMER_GLOBAL_DEVICE_TYPE_ENUM_SYSTEM    = SNDRV_TIMER_GLOBAL_SYSTEM,
    /* SNDRV_TIMER_GLOBAL_RTC is obsoleted. */
    ALSATIMER_GLOBAL_DEVICE_TYPE_ENUM_HPET      = SNDRV_TIMER_GLOBAL_HPET,
    ALSATIMER_GLOBAL_DEVICE_TYPE_ENUM_HRTIMER   = SNDRV_TIMER_GLOBAL_HRTIMER,
} ALSATimerGlobalDeviceTypeEnum;

typedef enum /*< flags >*/
{
    ALSATIMER_DEVICE_INFO_FLAG_SLAVE =  SNDRV_TIMER_FLG_SLAVE,
} ALSATimerDeviceInfoFlag;

typedef enum /*< flags >*/
{
    ALSATIMER_DEVICE_PARAM_FLAG_AUTO        = SNDRV_TIMER_PSFLG_AUTO,
    ALSATIMER_DEVICE_PARAM_FLAG_EXCLUSIVE   = SNDRV_TIMER_PSFLG_EXCLUSIVE,
    ALSATIMER_DEVICE_PARAM_FLAG_EARLY_EVENT = SNDRV_TIMER_PSFLG_EARLY_EVENT,
} ALSATimerDeviceParamFlag;

typedef enum /*< flags >*/
{
    ALSATIMER_EVENT_TYPE_FLAG_RESOLUTION= SNDRV_TIMER_EVENT_RESOLUTION,
    ALSATIMER_EVENT_TYPE_FLAG_TICK      = SNDRV_TIMER_EVENT_TICK,
    ALSATIMER_EVENT_TYPE_FLAG_START     = SNDRV_TIMER_EVENT_START,
    ALSATIMER_EVENT_TYPE_FLAG_STOP      = SNDRV_TIMER_EVENT_STOP,
    ALSATIMER_EVENT_TYPE_FLAG_CONTINUE  = SNDRV_TIMER_EVENT_CONTINUE,
    ALSATIMER_EVENT_TYPE_FLAG_PAUSE     = SNDRV_TIMER_EVENT_PAUSE,
    ALSATIMER_EVENT_TYPE_FLAG_EARLY     = SNDRV_TIMER_EVENT_EARLY,
    ALSATIMER_EVENT_TYPE_FLAG_SUSPEND   = SNDRV_TIMER_EVENT_SUSPEND,
    ALSATIMER_EVENT_TYPE_FLAG_RESUME    = SNDRV_TIMER_EVENT_RESUME,
    ALSATIMER_EVENT_TYPE_FLAG_MSTART    = SNDRV_TIMER_EVENT_MSTART,
    ALSATIMER_EVENT_TYPE_FLAG_MSTOP     = SNDRV_TIMER_EVENT_MSTOP,
    ALSATIMER_EVENT_TYPE_FLAG_MCONTINUE = SNDRV_TIMER_EVENT_MCONTINUE,
    ALSATIMER_EVENT_TYPE_FLAG_MPAUSE    = SNDRV_TIMER_EVENT_MPAUSE,
    ALSATIMER_EVENT_TYPE_FLAG_MSUSPEND  = SNDRV_TIMER_EVENT_MSUSPEND,
    ALSATIMER_EVENT_TYPE_FLAG_MRESUME   = SNDRV_TIMER_EVENT_MRESUME,
} ALSATimerEventTypeFlag;

#endif
