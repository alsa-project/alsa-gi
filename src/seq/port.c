#include <alsa/asoundlib.h>
#include "port.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqPortPrivate {
	snd_seq_port_info_t *info;

	ALSASeqClient *client;
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSASeqPort, alsaseq_port, G_TYPE_OBJECT)
#define SEQ_PORT_GET_PRIVATE(obj)					\
        (G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				     ALSASEQ_TYPE_PORT, ALSASeqPortPrivate))

enum seq_port_prop {
	SEQ_PORT_PROP_ID = 1,
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
	ALSASeqPortPrivate *priv = SEQ_PORT_GET_PRIVATE(self);

	switch (id) {
	case SEQ_PORT_PROP_ID:
		g_value_set_int(val,
				snd_seq_port_info_get_port(priv->info));
		break;
	case SEQ_PORT_PROP_NAME:
		g_value_set_string(val,
				   snd_seq_port_info_get_name(priv->info));
		break;
	case SEQ_PORT_PROP_TYPE:
		g_value_set_uint(val,
				 snd_seq_port_info_get_type(priv->info));
		break;
	case SEQ_PORT_PROP_CAPABILITY:
		g_value_set_uint(val,
				 snd_seq_port_info_get_capability(priv->info));
		break;
	case SEQ_PORT_PROP_MIDI_CHANNELS:
		g_value_set_int(val,
			snd_seq_port_info_get_midi_channels(priv->info));
		break;
	case SEQ_PORT_PROP_MIDI_VOICES:
		g_value_set_int(val,
				snd_seq_port_info_get_midi_voices(priv->info));
		break;
	case SEQ_PORT_PROP_SYNTH_VOICES:
		g_value_set_int(val,
				snd_seq_port_info_get_synth_voices(priv->info));
		break;
	case SEQ_PORT_PROP_READ_USE:
		g_value_set_int(val,
				snd_seq_port_info_get_read_use(priv->info));
		break;
	case SEQ_PORT_PROP_WRITE_USE:
		g_value_set_int(val,
				snd_seq_port_info_get_write_use(priv->info));
		break;
	case SEQ_PORT_PROP_PORT_SPECIFIED:
		g_value_set_boolean(val,
			snd_seq_port_info_get_port_specified(priv->info));
		break;
	case SEQ_PORT_PROP_TIMESTAMPING:
		g_value_set_boolean(val,
				snd_seq_port_info_get_timestamping(priv->info));
		break;
	case SEQ_PORT_PROP_TIMESTAMP_REAL:
		g_value_set_boolean(val,
			snd_seq_port_info_get_timestamp_real(priv->info));
		break;
	case SEQ_PORT_PROP_TIMESTAMP_QUEUE_ID:
		g_value_set_int(val,
			snd_seq_port_info_get_timestamp_queue(priv->info));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		return;
	}

}

static void seq_port_set_property(GObject *obj, guint id,
				  const GValue *val, GParamSpec *spec)
{
	ALSASeqPort *self = ALSASEQ_PORT(obj);
	ALSASeqPortPrivate *priv = SEQ_PORT_GET_PRIVATE(self);

	switch (id) {
	case SEQ_PORT_PROP_NAME:
		snd_seq_port_info_set_name(priv->info, g_value_get_string(val));
		break;
	case SEQ_PORT_PROP_TYPE:
		snd_seq_port_info_set_type(priv->info,
					   g_value_get_uint(val));
		break;
	case SEQ_PORT_PROP_CAPABILITY:
		snd_seq_port_info_set_capability(priv->info,
						 g_value_get_uint(val));
		break;
	case SEQ_PORT_PROP_MIDI_CHANNELS:
		snd_seq_port_info_set_midi_channels(priv->info,
						    g_value_get_uint(val));
		break;
	case SEQ_PORT_PROP_MIDI_VOICES:
		snd_seq_port_info_set_midi_voices(priv->info,
						  g_value_get_uint(val));
		break;
	case SEQ_PORT_PROP_SYNTH_VOICES:
		snd_seq_port_info_set_synth_voices(priv->info,
						   g_value_get_uint(val));
		break;
	case SEQ_PORT_PROP_PORT_SPECIFIED:
		snd_seq_port_info_set_port_specified(priv->info,
						g_value_get_boolean(val));
		break;
	case SEQ_PORT_PROP_TIMESTAMPING:
		snd_seq_port_info_set_timestamping(priv->info,
						   g_value_get_boolean(val));
		break;
	case SEQ_PORT_PROP_TIMESTAMP_REAL:
		snd_seq_port_info_set_timestamp_real(priv->info,
						     g_value_get_boolean(val));
		break;
	case SEQ_PORT_PROP_TIMESTAMP_QUEUE_ID:
		snd_seq_port_info_set_timestamp_queue(priv->info,
						      g_value_get_int(val));
		break;
	case SEQ_PORT_PROP_ID:
	case SEQ_PORT_PROP_READ_USE:
	case SEQ_PORT_PROP_WRITE_USE:
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		return;
	}

	/* Lazily, no error handling... */
	snd_seq_set_port_info(priv->client->handle,
			      snd_seq_port_info_get_port(priv->info),
			      priv->info);
}

static void seq_port_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(alsaseq_port_parent_class)->dispose(gobject);
}

static void seq_port_finalize(GObject *gobject)
{
	ALSASeqPort *self = ALSASEQ_PORT(gobject);
	ALSASeqPortPrivate *priv = SEQ_PORT_GET_PRIVATE(self);

	/* Close this port. */
	snd_seq_delete_port(priv->client->handle,
			    snd_seq_port_info_get_port(priv->info));
	snd_seq_port_info_free(priv->info);
	g_object_unref(priv->client);

	G_OBJECT_CLASS(alsaseq_port_parent_class)->finalize(gobject);
}

static void alsaseq_port_class_init(ALSASeqPortClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = seq_port_get_property;
	gobject_class->set_property = seq_port_set_property;
	gobject_class->dispose = seq_port_dispose;
	gobject_class->finalize = seq_port_finalize;

	seq_port_props[SEQ_PORT_PROP_ID] =
		g_param_spec_int("id", "id",
				 "The id of this port",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	seq_port_props[SEQ_PORT_PROP_NAME] =
		g_param_spec_string("name", "name",
				    "a pointer to name",
				    "port",
				    G_PARAM_READWRITE);
	seq_port_props[SEQ_PORT_PROP_TYPE] =
		g_param_spec_uint("type", "type",
				  "A bitmask of SND_SEQ_PORT_TYPE_XXX",
				  0, UINT_MAX,
				  0,
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
		     alsaseq_sigs_marshal_VOID__INT_UINT_CHAR_UCHAR_UINT_UINT_UCHAR_UCHAR,
		     G_TYPE_NONE, 8,
		     G_TYPE_INT, G_TYPE_UINT, G_TYPE_CHAR, G_TYPE_UCHAR,
		     G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UCHAR, G_TYPE_UCHAR);
}

static void alsaseq_port_init(ALSASeqPort *self)
{
	self->priv = alsaseq_port_get_instance_private(self);
}

ALSASeqPort *alsaseq_port_new(ALSASeqClient *client, const gchar *name,
			      GError **exception)
{
	ALSASeqPort *self;
	ALSASeqPortPrivate *priv;

	snd_seq_port_info_t *info = NULL;

	int err;

	/* Open new port for this client. */
	err = snd_seq_port_info_malloc(&info);
	if (err < 0)
		goto error;
	snd_seq_port_info_set_name(info, name);

	err = snd_seq_create_port(client->handle, info);
	if (err < 0)
		goto error;

	self = g_object_new(ALSASEQ_TYPE_PORT, NULL);
	if (self == NULL) {
		snd_seq_delete_port(client->handle,
				    snd_seq_port_info_get_port(priv->info));
		err = -ENOMEM;
		goto error;
	}
	priv = SEQ_PORT_GET_PRIVATE(self);

	priv->info = info;
	priv->client = g_object_ref(client);

	return self;
error:
	if (info != NULL)
		snd_seq_port_info_free(info);
	g_set_error(exception, g_quark_from_static_string(__func__),
		    -err, "%s", snd_strerror(err));
	return NULL;
}

void alsaseq_port_get_address(ALSASeqClient *self, GArray *addr,
			      GError **exception)
{

}
