#include <alsa/asoundlib.h>
#include "seq.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqSeqPrivate {
	snd_seq_t *handle;
};
G_DEFINE_TYPE_WITH_PRIVATE (ALSASeqSeq, alsaseq_seq, G_TYPE_OBJECT)

static void alsaseq_seq_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsaseq_seq_parent_class)->dispose(gobject);
}

/*
gobject_new() -> g_clear_object()
*/

static void alsaseq_seq_finalize (GObject *gobject)
{
	ALSASeqSeq *self = ALSASEQ_SEQ(gobject);

	if (self->priv->handle != NULL)
		snd_seq_close(self->priv->handle);

	G_OBJECT_CLASS(alsaseq_seq_parent_class)->finalize (gobject);
}

static void alsaseq_seq_class_init(ALSASeqSeqClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->dispose = alsaseq_seq_dispose;
	gobject_class->finalize = alsaseq_seq_finalize;
}

static void
alsaseq_seq_init(ALSASeqSeq *self)
{
	self->priv = alsaseq_seq_get_instance_private(self);
}

ALSASeqSeq *alsaseq_seq_new(gchar *node, GError **exception)
{
	ALSASeqSeq *self;
	snd_seq_t *handle;
	int err;

	/* Always open duplex ports. */
	err = snd_seq_open(&handle, node, SND_SEQ_OPEN_DUPLEX, 0);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	self = g_object_new(ALSASEQ_TYPE_SEQ, NULL);
	self->priv->handle = handle;

	return self;
}

const gchar *alsaseq_seq_get_name(ALSASeqSeq *self)
{
	return snd_seq_name(self->priv->handle);
}

gint alsaseq_seq_get_client_id(ALSASeqSeq *self)
{
	return snd_seq_client_id(self->priv->handle);
}

guint alsaseq_seq_get_output_buffer_size(ALSASeqSeq *self)
{
	return snd_seq_get_output_buffer_size(self->priv->handle);
}

gboolean alsaseq_seq_set_output_buffer_size(ALSASeqSeq *self, guint size,
					    GError **exception)
{
	int err;

	err = snd_seq_set_output_buffer_size(self->priv->handle, size);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
	}

	return err == 0;
}

guint alsaseq_seq_get_input_buffer_size(ALSASeqSeq *self)
{
	return snd_seq_get_input_buffer_size(self->priv->handle);
}

/*
 * Aligned to sizeof(snd_seq_event_t) automatically.
 */
gboolean alsaseq_seq_set_input_buffer_size(ALSASeqSeq *self, guint size,
					   GError **exception)
{
	int err;

	err = snd_seq_set_input_buffer_size(self->priv->handle, size);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
	}

	return err == 0;
}
