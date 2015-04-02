#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "card.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSACardSndUnitPrivate {
	gchar *unit;
};
G_DEFINE_TYPE_WITH_PRIVATE(ALSACardSndUnit, alsacard_snd_unit, G_TYPE_OBJECT)

static void alsacard_snd_unit_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsacard_snd_unit_parent_class)->dispose(obj);
}

/*
gobject_new() -> g_clear_object()
*/

static void alsacard_snd_unit_finalize(GObject *obj)
{
	ALSACardSndUnit *self = ALSACARD_SND_UNIT(obj);

	if (self->priv->unit)
		free(self->priv->unit);

	G_OBJECT_CLASS(alsacard_snd_unit_parent_class)->finalize(obj);

	printf("destroy\n");
}

static void alsacard_snd_unit_class_init(ALSACardSndUnitClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = alsacard_snd_unit_dispose;
	gobject_class->finalize = alsacard_snd_unit_finalize;

	printf("class initialized\n");
}

static void
alsacard_snd_unit_init(ALSACardSndUnit *self)
{
	self->priv = alsacard_snd_unit_get_instance_private(self);
	printf("init\n");
}

ALSACardSndUnit *alsacard_snd_unit_new(gchar *str)
{
    return g_object_new(ALSACARD_TYPE_SND_UNIT, NULL);;
}
