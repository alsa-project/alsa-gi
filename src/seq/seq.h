#ifndef HINAWA_SND_H
#define HINAWA_SND_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSASEQ_TYPE_SEQ	(alsaseq_seq_get_type())

#define ALSASEQ_SEQ(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSASEQ_TYPE_SEQ,		\
				    ALSASeqSeq))
#define ALSASEQ_IS_SEQ(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSASEQ_TYPE_SEQ))

#define ALSASEQ_SEQ_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSASEQ_TYPE_SND_UNIT,		\
				 ALSASeqSeqClass))
#define ALSASEQ_IS_SEQ_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSASEQ_TYPE_Seq))
#define ALSASEQ_SND_UNIT_GET_CLASS(obj) 			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSASEQ_TYPE_SEQ,		\
				   ALSASeqSeqClass))

typedef struct _ALSASeqSeq		ALSASeqSeq;
typedef struct _ALSASeqSeqClass	ALSASeqSeqClass;
typedef struct _ALSASeqSeqPrivate	ALSASeqSeqPrivate;

struct _ALSASeqSeq
{
	GObject parent_instance;

	ALSASeqSeqPrivate *priv;
};

struct _ALSASeqSeqClass
{
    GObjectClass parent_class;
};

GType alsaseq_seq_get_type(void) G_GNUC_CONST;

ALSASeqSeq *alsaseq_seq_new(gchar *str);
//ALSASeqSeq *alsaseq_seq_new_lconf(gchar *node, gpointer *lconf);

const gchar *alsaseq_seq_get_name(ALSASeqSeq *self);
int alsaseq_seq_get_client_id(ALSASeqSeq *self);

guint alsaseq_seq_get_output_buffer_size(ALSASeqSeq *self);
gboolean alsaseq_seq_set_output_buffer_size(ALSASeqSeq *self, guint size);
guint alsaseq_seq_get_input_buffer_size(ALSASeqSeq *self);
gboolean alsaseq_seq_set_input_buffer_size(ALSASeqSeq *self, guint size);


#endif
