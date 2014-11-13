#ifndef HINAWA_SND_H
#define HINAWA_SND_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSASeq_TYPE_SND_UNIT	(alsaseq_snd_unit_get_type())

#define ALSASeq_SND_UNIT(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSASeq_TYPE_SND_UNIT,	\
				    ALSASeqSndUnit))
#define ALSASeq_IS_SND_UNIT(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSASeq_TYPE_SND_UNIT))

#define ALSASeq_SND_UNIT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSASeq_TYPE_SND_UNIT,		\
				 ALSASeqSndUnitClass))
#define ALSASeq_IS_SND_UNIT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSASeq_TYPE_SND_UNIT))
#define ALSASeq_SND_UNIT_GET_CLASS(obj) 				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSASeq_TYPE_SND_UNIT,	\
				   ALSASeqSndUnitClass))

typedef struct _ALSASeqSndUnit		ALSASeqSndUnit;
typedef struct _ALSASeqSndUnitClass	ALSASeqSndUnitClass;
typedef struct _ALSASeqSndUnitPrivate	ALSASeqSndUnitPrivate;

struct _ALSASeqSndUnit
{
	GObject parent_instance;

	ALSASeqSndUnitPrivate *priv;
};

struct _ALSASeqSndUnitClass
{
    GObjectClass parent_class;
};

GType alsaseq_snd_unit_get_type(void) G_GNUC_CONST;

typedef void (*ALSASeqSndUnitCB)(ALSASeqSndUnit* unit, void *private_data, gint val);

ALSASeqSndUnit *alsaseq_snd_unit_new(gchar *str);

#endif
