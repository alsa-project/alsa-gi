#ifndef __ALSASEQ_CLIENT_H__
#define __ALSASEQ_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

#include "port.h"

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
};

struct _ALSASeqClientClass
{
	GObjectClass parent_class;
};

GType alsaseq_client_get_type(void) G_GNUC_CONST;

void alsaseq_client_open(ALSASeqClient *self, gchar *path, const gchar *name,
			 GError **exception);
void alsaseq_client_update(ALSASeqClient *self, GError **exception);

void alsaseq_client_listen(ALSASeqClient *self, GError **exception);
void alsaseq_client_unlisten(ALSASeqClient *self);

typedef struct _ALSASeqPort	ALSASeqPort;
ALSASeqPort *alsaseq_client_open_port(ALSASeqClient *self, const gchar *name,
				      GError **exception);
void alsaseq_client_close_port(ALSASeqClient *self, ALSASeqPort *port);
void alsaseq_client_get_ports(ALSASeqClient *self, GArray *ports,
			      GError **exception);
#endif
