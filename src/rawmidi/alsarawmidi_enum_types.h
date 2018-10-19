#ifndef __ALSA_GOBJECT_ALSARAWMIDI_ENUM_TYPES__H__
#define __ALSA_GOBJECT_ALSARAWMIDI_ENUM_TYPES__H__

#include <sound/asound.h>

typedef enum {
    ALSARAWMIDI_STREAM_ENUM_OUTPUT  = SNDRV_RAWMIDI_STREAM_OUTPUT,
    ALSARAWMIDI_STREAM_ENUM_INPUT   = SNDRV_RAWMIDI_STREAM_INPUT,
} ALSARawmidiStreamEnum;

typedef enum /*< flags >*/
{
    ALSARAWMIDI_INFO_FLAG_OUTPUT    = SNDRV_RAWMIDI_INFO_OUTPUT,
    ALSARAWMIDI_INFO_FLAG_INPUT     = SNDRV_RAWMIDI_INFO_INPUT,
    ALSARAWMIDI_INFO_FLAG_DUPLEX    = SNDRV_RAWMIDI_INFO_DUPLEX,
} ALSARawmidiInfoFlag;

#endif