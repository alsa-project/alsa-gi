#ifndef __ALSASEQ_CLIENT_H__
#define __ALSASEQ_CLIENT_H__

#include <glib-object.h>
#include <gio/gio.h>
#include "client_info.h"
#include "system_info.h"
#include "port_info.h"

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
				 ALSASEQ_TYPE_Seq))
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

ALSASeqClient *alsaseq_client_new(gchar *str, GError **exception);
//ALSASeqClient *alsaseq_client_new_lconf(gchar *node, gpointer *lconf);

const guchar *alsaseq_client_get_name(ALSASeqClient *self);
int alsaseq_client_get_id(ALSASeqClient *self);

guint alsaseq_client_get_output_buffer_size(ALSASeqClient *self);
gboolean alsaseq_client_set_output_buffer_size(ALSASeqClient *self, guint size,
					    GError **exception);
guint alsaseq_client_get_input_buffer_size(ALSASeqClient *self);
gboolean alsaseq_client_set_input_buffer_size(ALSASeqClient *self, guint size,
					    GError **exception);

ALSASeqClientInfo *alsaseq_client_get_info(ALSASeqClient *client,
					   GError **exception);
void alsaseq_client_set_info(ALSASeqClient *client, ALSASeqClientInfo *info,
			     GError **exception);

ALSASeqSystemInfo *alsaseq_client_get_system_info(ALSASeqClient *client,
						  GError **exception);

ALSASeqPortInfo *alsaseq_client_create_port(ALSASeqClient *client,
					    GError **exception);
void alsaseq_client_delete_port(ALSASeqClient *client, guint id,
				GError **exception);
#endif
