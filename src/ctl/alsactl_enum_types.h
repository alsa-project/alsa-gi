#ifndef __ALSA_GOBJECT_ALSACTL_ENUM_TYPES__H__
#define __ALSA_GOBJECT_ALSACTL_ENUM_TYPES__H__

#include <sound/asound.h>

typedef enum {
    ALSACTL_ELEM_TYPE_ENUM_NONE         = SNDRV_CTL_ELEM_TYPE_NONE,
    ALSACTL_ELEM_TYPE_ENUM_BOOLEAN      = SNDRV_CTL_ELEM_TYPE_BOOLEAN,
    ALSACTL_ELEM_TYPE_ENUM_INTEGER      = SNDRV_CTL_ELEM_TYPE_INTEGER,
    ALSACTL_ELEM_TYPE_ENUM_ENUMERATED   = SNDRV_CTL_ELEM_TYPE_ENUMERATED,
    ALSACTL_ELEM_TYPE_ENUM_BYTES        = SNDRV_CTL_ELEM_TYPE_BYTES,
    ALSACTL_ELEM_TYPE_ENUM_IEC958       = SNDRV_CTL_ELEM_TYPE_IEC958,
    ALSACTL_ELEM_TYPE_ENUM_INTEGER64    = SNDRV_CTL_ELEM_TYPE_INTEGER64,
} ALSACtlElemTypeEnum;

typedef enum {
    ALSACTL_ELEM_IFACE_ENUM_CARD        = SNDRV_CTL_ELEM_IFACE_CARD,
    ALSACTL_ELEM_IFACE_ENUM_HWDEP       = SNDRV_CTL_ELEM_IFACE_HWDEP,
    ALSACTL_ELEM_IFACE_ENUM_MIXER       = SNDRV_CTL_ELEM_IFACE_MIXER,
    ALSACTL_ELEM_IFACE_ENUM_PCM         = SNDRV_CTL_ELEM_IFACE_PCM,
    ALSACTL_ELEM_IFACE_ENUM_RAWMIDI     = SNDRV_CTL_ELEM_IFACE_RAWMIDI,
    ALSACTL_ELEM_IFACE_ENUM_TIMER       = SNDRV_CTL_ELEM_IFACE_TIMER,
    ALSACTL_ELEM_IFACE_ENUM_SEQUENCER   = SNDRV_CTL_ELEM_IFACE_SEQUENCER,
} ALSACtlElemIfaceEnum;

typedef enum /*< flags >*/
{
    ALSACTL_ELEM_ACCESS_FLAG_READ         = SNDRV_CTL_ELEM_ACCESS_READ,
    ALSACTL_ELEM_ACCESS_FLAG_WRITE        = SNDRV_CTL_ELEM_ACCESS_WRITE,
    ALSACTL_ELEM_ACCESS_FLAG_VOLATILE     = SNDRV_CTL_ELEM_ACCESS_VOLATILE,
    ALSACTL_ELEM_ACCESS_FLAG_TIMESTAMP    = SNDRV_CTL_ELEM_ACCESS_TIMESTAMP,
    ALSACTL_ELEM_ACCESS_FLAG_TLV_READ     = SNDRV_CTL_ELEM_ACCESS_TLV_READ,
    ALSACTL_ELEM_ACCESS_FLAG_TLV_WRITE    = SNDRV_CTL_ELEM_ACCESS_TLV_WRITE,
    ALSACTL_ELEM_ACCESS_FLAG_TLV_COMMAND  = SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND,
    ALSACTL_ELEM_ACCESS_FLAG_INACTIVE     = SNDRV_CTL_ELEM_ACCESS_INACTIVE,
    ALSACTL_ELEM_ACCESS_FLAG_LOCK         = SNDRV_CTL_ELEM_ACCESS_LOCK,
    ALSACTL_ELEM_ACCESS_FLAG_OWNER        = SNDRV_CTL_ELEM_ACCESS_OWNER,
    ALSACTL_ELEM_ACCESS_FLAG_TLV_CALLBACK = SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK,
    ALSACTL_ELEM_ACCESS_FLAG_USER         = SNDRV_CTL_ELEM_ACCESS_USER,
} ALSACtlElemAccessFlag;

typedef enum /*< flags >*/
{
    ALSACTL_EVENT_MASK_FLAG_VALUE   = SNDRV_CTL_EVENT_MASK_VALUE,
    ALSACTL_EVENT_MASK_FLAG_INFO    = SNDRV_CTL_EVENT_MASK_INFO,
    ALSACTL_EVENT_MASK_FLAG_ADD     = SNDRV_CTL_EVENT_MASK_ADD,
    ALSACTL_EVENT_MASK_FLAG_TLV     = SNDRV_CTL_EVENT_MASK_TLV,
    ALSACTL_EVENT_MASK_FLAG_REMOVE  = SNDRV_CTL_EVENT_MASK_REMOVE,
} ALSACtlEventMaskFlag;

#endif
