#include <alsa/asoundlib.h>
#include "client_info.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqClientInfoPrivate {
	snd_seq_client_info_t *handle;
};

G_DEFINE_TYPE_WITH_PRIVATE (ALSASeqClientInfo, alsaseq_client_info,
			    G_TYPE_OBJECT)

enum alsaseq_client_info_prop {
	CLIENT_INFO_PROP_HANDLE = 1,
	CLIENT_INFO_PROP_COUNT,
};

static GParamSpec *client_info_props[CLIENT_INFO_PROP_COUNT] = { NULL, };

static void alseseq_client_info_get_property(GObject *obj, guint id,
					     GValue *val, GParamSpec *spec)
{
	ALSASeqClientInfo *self = ALSASEQ_CLIENT_INFO(obj);
	void *temp;

	switch (id) {
	case CLIENT_INFO_PROP_HANDLE:
		g_value_set_pointer(val, self->priv->handle);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		return;
	}

}

static void alseseq_client_info_set_property(GObject *obj, guint id,
					     const GValue *val,
					     GParamSpec *spec)
{
	ALSASeqClientInfo *self = ALSASEQ_CLIENT_INFO(obj);
	switch (id) {
	case CLIENT_INFO_PROP_HANDLE:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		return;
	}
}

static void alsaseq_client_info_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsaseq_client_info_parent_class)->dispose(gobject);
}

static void alsaseq_client_info_finalize (GObject *gobject)
{
	ALSASeqClientInfo *self = ALSASEQ_CLIENT_INFO(gobject);

	snd_seq_client_info_free(self->priv->handle);

	G_OBJECT_CLASS(alsaseq_client_info_parent_class)->finalize (gobject);
}

static void alsaseq_client_info_class_init(ALSASeqClientInfoClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = alseseq_client_info_get_property;
	gobject_class->set_property = alseseq_client_info_set_property;
	gobject_class->dispose = alsaseq_client_info_dispose;
	gobject_class->finalize = alsaseq_client_info_finalize;

	client_info_props[CLIENT_INFO_PROP_HANDLE] =
		g_param_spec_pointer("handle", "handle",
				     "a pointer to snd_seq_client_info_t",
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(gobject_class, CLIENT_INFO_PROP_COUNT,
					  client_info_props);
}

static void
alsaseq_client_info_init(ALSASeqClientInfo *self)
{
	self->priv = alsaseq_client_info_get_instance_private(self);
	/* TODO: error handling? */
	snd_seq_client_info_malloc(&self->priv->handle);
}

const gchar *alsaseq_client_info_get_name(const ALSASeqClientInfo *info)
{
	return snd_seq_client_info_get_name(info->priv->handle);
}

gboolean
alsaseq_client_info_get_broadcast_filter(const ALSASeqClientInfo *info)
{
	return snd_seq_client_info_get_broadcast_filter(info->priv->handle);
}

gboolean
alsaseq_client_info_get_error_bounce(const ALSASeqClientInfo *info)
{
	return snd_seq_client_info_get_error_bounce(info->priv->handle);
}

const unsigned char *
alsaseq_client_info_get_event_filter(const ALSASeqClientInfo *info)
{
	return snd_seq_client_info_get_event_filter(info->priv->handle);
}

gint alsaseq_client_info_get_num_ports(const ALSASeqClientInfo *info)
{
	return snd_seq_client_info_get_num_ports(info->priv->handle);
}

gint alsaseq_client_info_get_event_lost(const ALSASeqClientInfo *info)
{
	return snd_seq_client_info_get_event_lost(info->priv->handle);
}


void alsaseq_client_info_set_name(const ALSASeqClientInfo *info,
				  const char *name)
{
	snd_seq_client_info_set_name(info->priv->handle, name);
}

void alsaseq_client_info_set_broadcast_filter(const ALSASeqClientInfo *info,
					      gboolean enable)
{
	snd_seq_client_info_set_broadcast_filter(info->priv->handle, enable);
}

void alsaseq_client_info_set_error_bounce(const ALSASeqClientInfo *info,
					  gboolean enable)
{
	snd_seq_client_info_set_error_bounce(info->priv->handle, enable);
}

void alsaseq_client_info_set_event_filter(const ALSASeqClientInfo *info,
					  unsigned char *filter)
{
	snd_seq_client_info_set_event_filter(info->priv->handle, filter);
}

void alsaseq_client_info_event_filter_clear(const ALSASeqClientInfo *info)
{
	snd_seq_client_info_event_filter_clear(info->priv->handle);
}

void alsaseq_client_info_event_filter_add(const ALSASeqClientInfo *info,
					  gint event_type)
{
	snd_seq_client_info_event_filter_add(info->priv->handle, event_type);
}

void alsaseq_client_info_event_filter_del(const ALSASeqClientInfo *info,
					  gint event_type)
{
	snd_seq_client_info_event_filter_del(info->priv->handle, event_type);
}

void alsaseq_client_info_event_filter_check(const ALSASeqClientInfo *info,
					    gint event_type)
{
	snd_seq_client_info_event_filter_check(info->priv->handle, event_type);
}

