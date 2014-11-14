#ifndef HINAWA_SND_H
#define HINAWA_SND_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSACARD_TYPE_SND_UNIT	(alsacard_snd_unit_get_type())

#define ALSACARD_SND_UNIT(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACARD_TYPE_SND_UNIT,	\
				    ALSACardSndUnit))
#define ALSACARD_IS_SND_UNIT(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACARD_TYPE_SND_UNIT))

#define ALSACARD_SND_UNIT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACARD_TYPE_SND_UNIT,		\
				 ALSACardSndUnitClass))
#define ALSACARD_IS_SND_UNIT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACARD_TYPE_SND_UNIT))
#define ALSACARD_SND_UNIT_GET_CLASS(obj) 				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACARD_TYPE_SND_UNIT,	\
				   ALSACardSndUnitClass))

typedef struct _ALSACardSndUnit		ALSACardSndUnit;
typedef struct _ALSACardSndUnitClass	ALSACardSndUnitClass;
typedef struct _ALSACardSndUnitPrivate	ALSACardSndUnitPrivate;

struct _ALSACardSndUnit
{
	GObject parent_instance;

	ALSACardSndUnitPrivate *priv;
};

struct _ALSACardSndUnitClass
{
    GObjectClass parent_class;
};

GType alsacard_snd_unit_get_type(void) G_GNUC_CONST;

typedef void (*ALSACardSndUnitCB)(ALSACardSndUnit* unit, void *private_data, gint val);

ALSACardSndUnit *alsacard_snd_unit_new(gchar *str);

#endif
