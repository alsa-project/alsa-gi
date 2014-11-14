#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "alsaseq.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqSndUnitPrivate {
	gchar *unit;
};
G_DEFINE_TYPE_WITH_PRIVATE (ALSASeqSndUnit, alsaseq_snd_unit, G_TYPE_OBJECT)

static void alsaseq_snd_unit_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsaseq_snd_unit_parent_class)->dispose(gobject);
}

/*
gobject_new() -> g_clear_object()
*/

static void alsaseq_snd_unit_finalize (GObject *gobject)
{
	ALSASeqSndUnit *self = ALSASeq_SND_UNIT(gobject);

	if (self->priv->unit)
		free(self->priv->unit);

	G_OBJECT_CLASS(alsaseq_snd_unit_parent_class)->finalize (gobject);

	printf("destroy\n");
}

static void alsaseq_snd_unit_class_init(ALSASeqSndUnitClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->dispose = alsaseq_snd_unit_dispose;
	gobject_class->finalize = alsaseq_snd_unit_finalize;

	printf("class initialized\n");
}

static void
alsaseq_snd_unit_init(ALSASeqSndUnit *self)
{
	self->priv = alsaseq_snd_unit_get_instance_private(self);
	printf("init\n");
}

ALSASeqSndUnit *alsaseq_snd_unit_new(gchar *str)
{
    return g_object_new(ALSASeq_TYPE_SND_UNIT, NULL);;
}
