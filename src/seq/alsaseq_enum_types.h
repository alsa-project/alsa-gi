#ifndef __ALSA_GOBJECT_ALSASEQ_ENUM_TYPES__H__
#define __ALSA_GOBJECT_ALSASEQ_ENUM_TYPES__H__

#include <sound/asequencer.h>

typedef enum {
    ALSASEQ_CLIENT_NUMBER_ENUM_SYSTEM   = SNDRV_SEQ_CLIENT_SYSTEM,
    ALSASEQ_CLIENT_NUMBER_ENUM_DUMMY    = SNDRV_SEQ_CLIENT_DUMMY,
    ALSASEQ_CLIENT_NUMBER_ENUM_OSS      = SNDRV_SEQ_CLIENT_OSS,
} ALSASeqClientNumberEnum;

typedef enum {
    ALSASEQ_CLIENT_TYPE_ENUM_NO_CLIENT      = NO_CLIENT,
    ALSASEQ_CLIENT_TYPE_ENUM_USER_CLIENT    = USER_CLIENT,
    ALSASEQ_CLIENT_TYPE_ENUM_KERNEL_CLIENT  = KERNEL_CLIENT,
} ALSASeqClientTypeEnum;

typedef enum {
    ALSASEQ_CLIENT_FILTER_FLAG_BROADCAST    = SNDRV_SEQ_FILTER_BROADCAST,
    ALSASEQ_CLIENT_FILTER_FLAG_MULTICAST    = SNDRV_SEQ_FILTER_MULTICAST,
    ALSASEQ_CLIENT_FILTER_FLAG_BOUNCE       = SNDRV_SEQ_FILTER_BOUNCE,
    ALSASEQ_CLIENT_FILTER_FLAG_USE_EVENT    = SNDRV_SEQ_FILTER_USE_EVENT,
} ALSASeqClientFilterFlag;

typedef enum {
    ALSASEQ_PORT_NUMBER_ENUM_SYSTEM = SNDRV_SEQ_CLIENT_SYSTEM,
    ALSASEQ_PORT_NUMBER_ENUM_DUMMY  = SNDRV_SEQ_CLIENT_DUMMY,
    ALSASEQ_PORT_NUMBER_ENUM_OSS    = SNDRV_SEQ_CLIENT_OSS,
} ALSASeqPortNumberEnum;

typedef enum /*< flags >*/
{
    ALSASEQ_PORT_TYPE_FLAG_SPECIFIC         = SNDRV_SEQ_PORT_TYPE_SYNTH,
    ALSASEQ_PORT_TYPE_FLAG_MIDI_GENERIC     = SNDRV_SEQ_PORT_TYPE_MIDI_GENERIC,
    ALSASEQ_PORT_TYPE_FLAG_MIDI_GM          = SNDRV_SEQ_PORT_TYPE_MIDI_GM,
    ALSASEQ_PORT_TYPE_FLAG_MIDI_GS          = SNDRV_SEQ_PORT_TYPE_MIDI_GS,
    ALSASEQ_PORT_TYPE_FLAG_MIDI_XG          = SNDRV_SEQ_PORT_TYPE_MIDI_XG,
    ALSASEQ_PORT_TYPE_FLAG_MIDI_MT32        = SNDRV_SEQ_PORT_TYPE_MIDI_MT32,
    ALSASEQ_PORT_TYPE_FLAG_MIDI_GM2         = SNDRV_SEQ_PORT_TYPE_MIDI_GM2,
    ALSASEQ_PORT_TYPE_FLAG_SYNTH            = SNDRV_SEQ_PORT_TYPE_SYNTH,
    ALSASEQ_PORT_TYPE_FLAG_DIRECT_SAMPLE    = SNDRV_SEQ_PORT_TYPE_DIRECT_SAMPLE,
    ALSASEQ_PORT_TYPE_FLAG_SAMPLE           = SNDRV_SEQ_PORT_TYPE_SAMPLE,
    ALSASEQ_PORT_TYPE_FLAG_HARDWARE         = SNDRV_SEQ_PORT_TYPE_HARDWARE,
    ALSASEQ_PORT_TYPE_FLAG_SOFTWARE         = SNDRV_SEQ_PORT_TYPE_SOFTWARE,
    ALSASEQ_PORT_TYPE_FLAG_SYNTHESIZER      = SNDRV_SEQ_PORT_TYPE_SYNTHESIZER,
    ALSASEQ_PORT_TYPE_FLAG_PORT             = SNDRV_SEQ_PORT_TYPE_PORT,
    ALSASEQ_PORT_TYPE_FLAG_APPLICATION      = SNDRV_SEQ_PORT_TYPE_APPLICATION,
} ALSASeqPortTypeFlag;

#endif
