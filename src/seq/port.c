#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <sound/asound.h>
#include <sound/asequencer.h>
#include "port.h"

#include "alsaseq_sigs_marshal.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* For error handling. */
G_DEFINE_QUARK("ALSASeqPort", alsaseq_port)
#define raise(exception, errno)                         \
    g_set_error(exception, alsaseq_port_quark(), errno, \
            "%d: %s", __LINE__, strerror(errno))

struct _ALSASeqPortPrivate {
    int fd;
    struct snd_seq_port_info info;

    ALSASeqClient *client;
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSASeqPort, alsaseq_port, G_TYPE_OBJECT)

enum seq_port_prop {
    SEQ_PORT_PROP_FD = 1,
    SEQ_PORT_PROP_CLIENT_NUMBER,
    SEQ_PORT_PROP_NUMBER,
    SEQ_PORT_PROP_NAME,
    SEQ_PORT_PROP_TYPE,
    SEQ_PORT_PROP_CAPABILITY,
    SEQ_PORT_PROP_MIDI_CHANNELS,
    SEQ_PORT_PROP_MIDI_VOICES,
    SEQ_PORT_PROP_SYNTH_VOICES,
    SEQ_PORT_PROP_PORT_SPECIFIED,
    SEQ_PORT_PROP_TIMESTAMPING,
    SEQ_PORT_PROP_TIMESTAMP_REAL,
    SEQ_PORT_PROP_TIMESTAMP_QUEUE_ID,
    /* Read-only */
    SEQ_PORT_PROP_READ_USE,
    SEQ_PORT_PROP_WRITE_USE,
    SEQ_PORT_PROP_COUNT,
};
static GParamSpec *seq_port_props[SEQ_PORT_PROP_COUNT] = { NULL, };

enum seq_port_sig {
    SEQ_PORT_SIGNAL_EVENT = 0,
    SEQ_PORT_SIGNAL_COUNT,
};
static guint seq_port_sigs[SEQ_PORT_SIGNAL_COUNT] = { 0 };

static void seq_port_get_property(GObject *obj, guint id,
                                  GValue *val, GParamSpec *spec)
{
    ALSASeqPort *self = ALSASEQ_PORT(obj);
    ALSASeqPortPrivate *priv = alsaseq_port_get_instance_private(self);

    switch (id) {
    case SEQ_PORT_PROP_NUMBER:
        g_value_set_uchar(val, priv->info.addr.port);
        break;
    case SEQ_PORT_PROP_NAME:
        g_value_set_string(val, priv->info.name);
        break;
    case SEQ_PORT_PROP_TYPE:
        g_value_set_flags(val, (ALSASeqPortTypeFlag)priv->info.type);
        break;
    case SEQ_PORT_PROP_CAPABILITY:
        g_value_set_uint(val, priv->info.capability);
        break;
    case SEQ_PORT_PROP_MIDI_CHANNELS:
        g_value_set_int(val, priv->info.midi_channels);
        break;
    case SEQ_PORT_PROP_MIDI_VOICES:
        g_value_set_int(val, priv->info.midi_voices);
        break;
    case SEQ_PORT_PROP_SYNTH_VOICES:
        g_value_set_int(val, priv->info.synth_voices);
        break;
    case SEQ_PORT_PROP_READ_USE:
        g_value_set_int(val, priv->info.read_use);
        break;
    case SEQ_PORT_PROP_WRITE_USE:
        g_value_set_int(val, priv->info.write_use);
        break;
    case SEQ_PORT_PROP_PORT_SPECIFIED:
        g_value_set_boolean(val,
            priv->info.flags & SNDRV_SEQ_PORT_FLG_GIVEN_PORT);
        break;
    case SEQ_PORT_PROP_TIMESTAMPING:
        g_value_set_boolean(val,
            priv->info.flags & SNDRV_SEQ_PORT_FLG_TIMESTAMP);
        break;
    case SEQ_PORT_PROP_TIMESTAMP_REAL:
        g_value_set_boolean(val,
            priv->info.flags & SNDRV_SEQ_PORT_FLG_TIME_REAL);
        break;
    case SEQ_PORT_PROP_TIMESTAMP_QUEUE_ID:
        g_value_set_int(val, priv->info.time_queue);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }

}

static void seq_port_set_property(GObject *obj, guint id,
                                  const GValue *val, GParamSpec *spec)
{
    ALSASeqPort *self = ALSASEQ_PORT(obj);
    ALSASeqPortPrivate *priv = alsaseq_port_get_instance_private(self);

    switch (id) {
    case SEQ_PORT_PROP_FD:
        priv->fd = g_value_get_int(val);
        break;
    case SEQ_PORT_PROP_CLIENT_NUMBER:
        priv->info.addr.client = g_value_get_uchar(val);
        break;
    case SEQ_PORT_PROP_NUMBER:
        priv->info.addr.port = g_value_get_uchar(val);
        break;
    case SEQ_PORT_PROP_NAME:
        strncpy(priv->info.name, g_value_get_string(val),
            sizeof(priv->info.name));
        break;
    case SEQ_PORT_PROP_TYPE:
        priv->info.type = (ALSASeqPortTypeFlag)g_value_get_flags(val);
        break;
    case SEQ_PORT_PROP_CAPABILITY:
        priv->info.capability = g_value_get_uint(val);
        break;
    case SEQ_PORT_PROP_MIDI_CHANNELS:
        priv->info.midi_channels = g_value_get_uint(val);
        break;
    case SEQ_PORT_PROP_MIDI_VOICES:
        priv->info.midi_voices = g_value_get_uint(val);
        break;
    case SEQ_PORT_PROP_SYNTH_VOICES:
        priv->info.synth_voices = g_value_get_uint(val);
        break;
    case SEQ_PORT_PROP_PORT_SPECIFIED:
        if (g_value_get_boolean(val))
            priv->info.flags |= SNDRV_SEQ_PORT_FLG_GIVEN_PORT;
        else
            priv->info.flags &= ~SNDRV_SEQ_PORT_FLG_GIVEN_PORT;
        break;
    case SEQ_PORT_PROP_TIMESTAMPING:
        if (g_value_get_boolean(val))
            priv->info.flags |= SNDRV_SEQ_PORT_FLG_TIMESTAMP;
        else
            priv->info.flags &= ~SNDRV_SEQ_PORT_FLG_TIMESTAMP;
        break;
    case SEQ_PORT_PROP_TIMESTAMP_REAL:
        if (g_value_get_boolean(val))
            priv->info.flags |= SNDRV_SEQ_PORT_FLG_TIME_REAL;
        else
            priv->info.flags &= ~SNDRV_SEQ_PORT_FLG_TIME_REAL;
        break;
    case SEQ_PORT_PROP_TIMESTAMP_QUEUE_ID:
        priv->info.time_queue = g_value_get_int(val);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        return;
    }
}

static void seq_port_finalize(GObject *obj)
{
    ALSASeqPort *self = ALSASEQ_PORT(obj);
    ALSASeqPortPrivate *priv = alsaseq_port_get_instance_private(self);

    /* Close this port. */
    ioctl(priv->fd, SNDRV_SEQ_IOCTL_DELETE_PORT, &priv->info);

    alsaseq_client_close_port(self->_client, self);

    G_OBJECT_CLASS(alsaseq_port_parent_class)->finalize(obj);
}

/* Require to execute ioctl(2) after parameters are sets. */
static GObject *seq_port_construct(GType type, guint count,
                                   GObjectConstructParam *props)
{
    GObject *obj;
    ALSASeqPort *self;
    ALSASeqPortPrivate *priv;

    obj = G_OBJECT_CLASS(alsaseq_port_parent_class)->constructor(type,
                                                                 count, props);
    self = ALSASEQ_PORT(obj);
    priv = alsaseq_port_get_instance_private(self);

    /* Cannot handle this error... */
    ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_PORT_INFO, &priv->info);

    return obj;
}

static void alsaseq_port_class_init(ALSASeqPortClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->get_property = seq_port_get_property;
    gobject_class->set_property = seq_port_set_property;
    gobject_class->finalize = seq_port_finalize;
    gobject_class->constructor = seq_port_construct;

    seq_port_props[SEQ_PORT_PROP_FD] =
        g_param_spec_int("fd", "fd",
                         "file descriptor for special file of control device",
                         INT_MIN, INT_MAX,
                         -1,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    seq_port_props[SEQ_PORT_PROP_CLIENT_NUMBER] =
        g_param_spec_uchar("client-number", "client-number",
                           "An identical number of connected client, including "
                           "ALSASeqClientNumberEnum",
                           0, UCHAR_MAX,
                           0,
                           G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    seq_port_props[SEQ_PORT_PROP_NUMBER] =
        g_param_spec_uchar("number", "number",
                           "An identical number of the port, including "
                           "ALSASeqPortNumberEnum",
                           0, UCHAR_MAX,
                           0,
                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    seq_port_props[SEQ_PORT_PROP_NAME] =
        g_param_spec_string("name", "name",
                            "a pointer to name",
                            "port",
                            G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_TYPE] =
        g_param_spec_flags("type", "type",
                           "A bitflag with ALSASeqPortTypeFlag values",
                           ALSASEQ_TYPE_PORT_TYPE_FLAG,
                           ALSASEQ_PORT_TYPE_FLAG_SPECIFIC,
                           G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_CAPABILITY] =
        g_param_spec_uint("capabilities", "capabilities",
                          "A bitmask of SND_SEQ_PORT_CAP_XXX",
                          0, UINT_MAX,
                          0,
                          G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_MIDI_CHANNELS] =
        g_param_spec_int("midi-channels", "midi-channels",
                         "MIDI channels",
                         INT_MIN, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_MIDI_VOICES] =
        g_param_spec_int("midi-voices", "midi-voices",
                         "the number of MIDI voices",
                         INT_MIN, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_SYNTH_VOICES] =
        g_param_spec_int("synth-voices", "synth-voices",
                         "the number of synth voices",
                         INT_MIN, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_PORT_SPECIFIED] =
        g_param_spec_boolean("port-specified", "port-specified",
                             "whether port id is specified at created",
                             FALSE,
                             G_PARAM_READABLE);
    seq_port_props[SEQ_PORT_PROP_TIMESTAMPING] =
        g_param_spec_boolean("timestamping", "timestamping",
                             "updating timestamps of incoming events",
                             FALSE,
                             G_PARAM_READABLE);
    seq_port_props[SEQ_PORT_PROP_TIMESTAMP_REAL] =
        g_param_spec_boolean("timestamp-real", "timestamp-real",
                             "whether timestamp is in real-time mode",
                             FALSE,
                             G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_TIMESTAMP_QUEUE_ID] =
        g_param_spec_int("timestamp-queue", "timestamp-queue",
                         "the queue ID to get timestamps",
                         0, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_port_props[SEQ_PORT_PROP_READ_USE] =
        g_param_spec_int("read-use", "read-use",
                         "the number of read subscriptions",
                         0, INT_MAX,
                         0,
                         G_PARAM_READABLE);
    seq_port_props[SEQ_PORT_PROP_WRITE_USE] =
        g_param_spec_int("write-use", "write-use",
                         "the number of write subscriptions",
                         0, INT_MAX,
                         0,
                         G_PARAM_READABLE);

    g_object_class_install_properties(gobject_class, SEQ_PORT_PROP_COUNT,
                                      seq_port_props);

    /* TODO: annotation */
    seq_port_sigs[SEQ_PORT_SIGNAL_EVENT] =
        g_signal_new("event",
                     G_OBJECT_CLASS_TYPE(klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     alsaseq_sigs_marshal_VOID__STRING_UINT_CHAR_UCHAR_UINT_UINT_UCHAR_UCHAR,
                     G_TYPE_NONE, 8,
                     G_TYPE_STRING, G_TYPE_UINT, G_TYPE_CHAR, G_TYPE_UCHAR,
                     G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UCHAR, G_TYPE_UCHAR);
}

static void alsaseq_port_init(ALSASeqPort *self)
{
    return;
}

/**
 * alsaseq_port_update
 * @self: A ##ALSASeqPort
 * @exception: A #GError
 *
 * After calling this, all properties are updated. When changing properties,
 * this should be called.
 */
void alsaseq_port_update(ALSASeqPort *self, GError **exception)
{
    ALSASeqPortPrivate *priv;

    g_return_if_fail(ALSASEQ_IS_PORT(self));
    priv = alsaseq_port_get_instance_private(self);

    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_SET_PORT_INFO, &priv->info) < 0)
        raise(exception, errno);
}
