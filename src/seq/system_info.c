#include <alsa/asoundlib.h>
#include "system_info.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqSystemInfoPrivate {
	snd_seq_system_info_t *handle;
};

G_DEFINE_TYPE_WITH_PRIVATE (ALSASeqSystemInfo, alsaseq_system_info,
			    G_TYPE_OBJECT)

enum alsaseq_system_info_prop {
	SYSTEM_INFO_PROP_HANDLE = 1,
	SYSTEM_INFO_PROP_COUNT,
};

static GParamSpec *system_info_props[SYSTEM_INFO_PROP_COUNT] = { NULL, };

static void alseseq_system_info_get_property(GObject *obj, guint id,
					     GValue *val, GParamSpec *spec)
{
	ALSASeqSystemInfo *self = ALSASEQ_SYSTEM_INFO(obj);
	void *temp;

	switch (id) {
	case SYSTEM_INFO_PROP_HANDLE:
		g_value_set_pointer(val, self->priv->handle);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		return;
	}

}

static void alseseq_system_info_set_property(GObject *obj, guint id,
					     const GValue *val,
					     GParamSpec *spec)
{
	ALSASeqSystemInfo *self = ALSASEQ_SYSTEM_INFO(obj);
	switch (id) {
	case SYSTEM_INFO_PROP_HANDLE:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		return;
	}
}

static void alsaseq_system_info_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsaseq_system_info_parent_class)->dispose(gobject);
}

static void alsaseq_system_info_finalize (GObject *gobject)
{
	ALSASeqSystemInfo *self = ALSASEQ_SYSTEM_INFO(gobject);

	snd_seq_system_info_free(self->priv->handle);

	G_OBJECT_CLASS(alsaseq_system_info_parent_class)->finalize (gobject);
}

static void alsaseq_system_info_class_init(ALSASeqSystemInfoClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = alseseq_system_info_get_property;
	gobject_class->set_property = alseseq_system_info_set_property;
	gobject_class->dispose = alsaseq_system_info_dispose;
	gobject_class->finalize = alsaseq_system_info_finalize;

	system_info_props[SYSTEM_INFO_PROP_HANDLE] =
		g_param_spec_pointer("handle", "handle",
				     "a pointer to snd_seq_system_info_t",
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(gobject_class, SYSTEM_INFO_PROP_COUNT,
					  system_info_props);
}

static void
alsaseq_system_info_init(ALSASeqSystemInfo *self)
{
	self->priv = alsaseq_system_info_get_instance_private(self);
	/* TODO: error handling? */
	snd_seq_system_info_malloc(&self->priv->handle);
}

gint alsaseq_system_info_get_max_queues(const ALSASeqSystemInfo *info)
{
	return snd_seq_system_info_get_queues(info->priv->handle);
}

gint alsaseq_system_info_get_max_clients(const ALSASeqSystemInfo *info)
{
	return snd_seq_system_info_get_clients(info->priv->handle);
}

gint alsaseq_system_info_get_max_ports(const ALSASeqSystemInfo *info)
{
	return snd_seq_system_info_get_ports(info->priv->handle);
}

gint alsaseq_system_info_get_max_channels(const ALSASeqSystemInfo *info)
{
	return snd_seq_system_info_get_channels(info->priv->handle);
}
gint alsaseq_system_info_get_cur_queues(const ALSASeqSystemInfo *info)
{
	return snd_seq_system_info_get_cur_queues(info->priv->handle);
}

gint alsaseq_system_info_get_cur_clients(const ALSASeqSystemInfo *info)
{
	return snd_seq_system_info_get_cur_clients(info->priv->handle);
}

