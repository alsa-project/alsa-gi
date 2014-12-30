#ifndef __ALSASEQ_CLIENT_H__
#define __ALSASEQ_CLIENT_H__

#include <glib-object.h>
#include <alsa/asoundlib.h>

G_BEGIN_DECLS

#define ALSASEQ_TYPE_CLIENT	(alsaseq_client_get_type())

#define ALSASEQ_CLIENT(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSASEQ_TYPE_CLIENT,	\
				    ALSASeqClient))
#define ALSASEQ_IS_CLIENT(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSASEQ_TYPE_CLIENT))

#define ALSASEQ_CLIENT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSASEQ_TYPE_CLIENT,		\
				 ALSASeqClientClass))
#define ALSASEQ_IS_CLIENT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSASEQ_TYPE_CLIENT))
#define ALSASEQ_CLIENT_GET_CLASS(obj) 				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSASEQ_TYPE_CLIENT,		\
				   ALSASeqClientClass))

typedef struct _ALSASeqClient		ALSASeqClient;
typedef struct _ALSASeqClientClass	ALSASeqClientClass;
typedef struct _ALSASeqClientPrivate	ALSASeqClientPrivate;

struct _ALSASeqClient
{
	GObject parent_instance;

	ALSASeqClientPrivate *priv;

	snd_seq_t *handle;	/* Read-only */
};

struct _ALSASeqClientClass
{
    GObjectClass parent_class;
};

GType alsaseq_client_get_type(void) G_GNUC_CONST;

ALSASeqClient *alsaseq_client_new(gchar *seq, GError **exception);

void alsaseq_client_get_pool_status(ALSASeqClient *self, GArray *status,
				    GError **exception);
void alsaseq_client_listen(ALSASeqClient *self, GError **exception);
void alsaseq_client_unlisten(ALSASeqClient *self, GError **exception);
#endif
