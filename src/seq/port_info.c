#include <alsa/asoundlib.h>
#include "port_info.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqPortInfoPrivate {
	snd_seq_port_info_t *handle;
};

G_DEFINE_TYPE_WITH_PRIVATE (ALSASeqPortInfo, alsaseq_port_info,
			    G_TYPE_OBJECT)

enum alsaseq_port_info_prop {
	PORT_INFO_PROP_HANDLE = 1,
	PORT_INFO_PROP_NAME,
	PORT_INFO_PROP_CAPABILITY,
	PORT_INFO_PROP_TYPE,
	PORT_INFO_PROP_MIDI_CHANNELS,
	PORT_INFO_PROP_MIDI_VOICES,
	PORT_INFO_PROP_SYNTH_VOICES,
	PORT_INFO_PROP_READ_USE,
	PORT_INFO_PROP_WRITE_USE,
	PORT_INFO_PROP_PORT_SPECIFIED,
	PORT_INFO_PROP_TIMESTAMPING,
	PORT_INFO_PROP_TIMESTAMP_REAL,
	PORT_INFO_PROP_TIMESTAMP_QUEUE_ID,
	PORT_INFO_PROP_COUNT,
};

static GParamSpec *port_info_props[PORT_INFO_PROP_COUNT] = { NULL, };

static void alseseq_port_info_get_property(GObject *obj, guint id,
					     GValue *val, GParamSpec *spec)
{
	snd_seq_port_info_t *handle = ALSASEQ_PORT_INFO(obj)->priv->handle;
	void *temp;

	switch (id) {
	case PORT_INFO_PROP_HANDLE:
		g_value_set_pointer(val, handle);
		break;
	case PORT_INFO_PROP_NAME:
		g_value_set_string(val,
				   snd_seq_port_info_get_name(handle));
		break;
	case PORT_INFO_PROP_CAPABILITY:
		g_value_set_uint(val,
				 snd_seq_port_info_get_capability(handle));
		break;
	case PORT_INFO_PROP_TYPE:
		g_value_set_uint(val,
				 snd_seq_port_info_get_type(handle));
		break;
	case PORT_INFO_PROP_MIDI_CHANNELS:
		g_value_set_int(val,
				snd_seq_port_info_get_midi_channels(handle));
		break;
	case PORT_INFO_PROP_MIDI_VOICES:
		g_value_set_int(val,
				snd_seq_port_info_get_midi_voices(handle));
		break;
	case PORT_INFO_PROP_SYNTH_VOICES:
		g_value_set_int(val,
				snd_seq_port_info_get_synth_voices(handle));
		break;
	case PORT_INFO_PROP_READ_USE:
		g_value_set_int(val,
				snd_seq_port_info_get_read_use(handle));
		break;
	case PORT_INFO_PROP_WRITE_USE:
		g_value_set_int(val,
				snd_seq_port_info_get_write_use(handle));
		break;
	case PORT_INFO_PROP_PORT_SPECIFIED:
		g_value_set_boolean(val,
				snd_seq_port_info_get_port_specified(handle));
		break;
	case PORT_INFO_PROP_TIMESTAMPING:
		g_value_set_boolean(val,
				snd_seq_port_info_get_timestamping(handle));
		break;
	case PORT_INFO_PROP_TIMESTAMP_REAL:
		g_value_set_boolean(val,
				snd_seq_port_info_get_timestamp_real(handle));
		break;
	case PORT_INFO_PROP_TIMESTAMP_QUEUE_ID:
		g_value_set_int(val,
				snd_seq_port_info_get_timestamp_queue(handle));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		return;
	}

}

static void alseseq_port_info_set_property(GObject *obj, guint id,
					     const GValue *val,
					     GParamSpec *spec)
{
	snd_seq_port_info_t *handle = ALSASEQ_PORT_INFO(obj)->priv->handle;

	g_object_get(obj, "handle", &handle, NULL);

	switch (id) {
	case PORT_INFO_PROP_HANDLE:
		break;
	case PORT_INFO_PROP_NAME:
		snd_seq_port_info_set_name(handle, g_value_get_string(val));
		break;
	case PORT_INFO_PROP_CAPABILITY:
		snd_seq_port_info_set_capability(handle,
						 g_value_get_uint(val));
		break;
	case PORT_INFO_PROP_TYPE:
		snd_seq_port_info_set_type(handle,
					   g_value_get_uint(val));
		break;
	case PORT_INFO_PROP_MIDI_CHANNELS:
		snd_seq_port_info_set_midi_channels(handle,
						    g_value_get_uint(val));
		break;
	case PORT_INFO_PROP_MIDI_VOICES:
		snd_seq_port_info_set_midi_voices(handle,
						  g_value_get_uint(val));
		break;
	case PORT_INFO_PROP_SYNTH_VOICES:
		snd_seq_port_info_set_synth_voices(handle,
						   g_value_get_uint(val));
		break;
	case PORT_INFO_PROP_PORT_SPECIFIED:
		snd_seq_port_info_set_port_specified(handle,
						g_value_get_boolean(val));
	case PORT_INFO_PROP_TIMESTAMPING:
		snd_seq_port_info_set_timestamping(handle,
						   g_value_get_boolean(val));
		break;
	case PORT_INFO_PROP_TIMESTAMP_REAL:
		snd_seq_port_info_set_timestamp_real(handle,
						     g_value_get_boolean(val));
		break;
	case PORT_INFO_PROP_TIMESTAMP_QUEUE_ID:
		snd_seq_port_info_set_timestamp_queue(handle,
						      g_value_get_int(val));
		break;
	case PORT_INFO_PROP_READ_USE:
	case PORT_INFO_PROP_WRITE_USE:
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		return;
	}
}

static void alsaseq_port_info_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsaseq_port_info_parent_class)->dispose(gobject);
}

static void alsaseq_port_info_finalize (GObject *gobject)
{
	ALSASeqPortInfo *self = ALSASEQ_PORT_INFO(gobject);

	snd_seq_port_info_free(self->priv->handle);

	G_OBJECT_CLASS(alsaseq_port_info_parent_class)->finalize (gobject);
}

static void alsaseq_port_info_class_init(ALSASeqPortInfoClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = alseseq_port_info_get_property;
	gobject_class->set_property = alseseq_port_info_set_property;
	gobject_class->dispose = alsaseq_port_info_dispose;
	gobject_class->finalize = alsaseq_port_info_finalize;

	port_info_props[PORT_INFO_PROP_HANDLE] =
		g_param_spec_pointer("handle", "handle",
				     "a pointer to snd_seq_port_info_t",
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
	port_info_props[PORT_INFO_PROP_NAME] =
		g_param_spec_string("name", "name",
				    "a pointer to name",
				    "port",
				    G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_CAPABILITY] =
		g_param_spec_uint("capabilities", "capabilities",
				  "capability bits",
				  0, UINT_MAX, 0,
				  G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_TYPE] =
		g_param_spec_uint("type", "type",
				  "port type bits",
				  0, UINT_MAX, 0,
				  G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_MIDI_CHANNELS] =
		g_param_spec_int("midi_channels", "midi_channels",
				 "MIDI channels",
				 INT_MIN, INT_MAX, 0,
				 G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_MIDI_VOICES] =
		g_param_spec_int("midi_voices", "midi_voices",
				 "the number of MIDI voices",
				 INT_MIN, INT_MAX, 0,
				 G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_SYNTH_VOICES] =
		g_param_spec_int("synth_voices", "synth_voices",
				 "the number of synth voices",
				 INT_MIN, INT_MAX, 0,
				 G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_READ_USE] =
		g_param_spec_int("read_use", "read_use",
				 "the number of read subscriptions",
				 INT_MIN, INT_MAX, 0,
				 G_PARAM_READABLE);
	port_info_props[PORT_INFO_PROP_WRITE_USE] =
		g_param_spec_int("write_use", "write_use",
				 "the number of write subscriptions",
				 INT_MIN, INT_MAX, 0,
				 G_PARAM_READABLE);
	port_info_props[PORT_INFO_PROP_PORT_SPECIFIED] =
		g_param_spec_boolean("port_specified", "port_specified",
				     "whether port id is specified at created",
				     FALSE,
				     G_PARAM_READABLE);
	port_info_props[PORT_INFO_PROP_TIMESTAMPING] =
		g_param_spec_boolean("timestamping", "timestamping",
				     "updating timestamps of incoming events",
				     FALSE,
				     G_PARAM_READABLE);
	port_info_props[PORT_INFO_PROP_TIMESTAMP_REAL] =
		g_param_spec_boolean("timestamp_real", "timestamp_real",
				     "whether timestamp is in real-time mode",
				     FALSE,
				     G_PARAM_READWRITE);
	port_info_props[PORT_INFO_PROP_TIMESTAMP_QUEUE_ID] =
		g_param_spec_int("timestamp_queue", "timestamp_queue",
				 "the queue ID to get timestamps",
				 INT_MIN, INT_MAX, 0,
				 G_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class, PORT_INFO_PROP_COUNT,
					  port_info_props);
}

static void
alsaseq_port_info_init(ALSASeqPortInfo *self)
{
	self->priv = alsaseq_port_info_get_instance_private(self);
	/* TODO: error handling? */
	snd_seq_port_info_malloc(&self->priv->handle);
}
