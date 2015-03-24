#ifndef __ALSA_TOOLS_GIR_ALSACTL_CLIENT__H_
#define __ALSA_TOOLS_GIR_ALSACTL_CLIENT__H_

#include <glib.h>
#include <glib-object.h>

#include <sound/asound.h>

#include "elemset.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_CLIENT	(alsactl_client_get_type())

#define ALSACTL_CLIENT(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_CLIENT,	\
				    ALSACtlClient))
#define ALSACTL_IS_CLIENT(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_CLIENT))

#define ALSACTL_CLIENT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_CLIENT,		\
				 ALSACtlClientClass))
#define ALSACTL_IS_CLIENT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_CLIENT))
#define ALSACTL_CLIENT_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_CLIENT,	\
				   ALSACtlClientClass))

typedef struct _ALSACtlClient		ALSACtlClient;
typedef struct _ALSACtlClientClass	ALSACtlClientClass;
typedef struct _ALSACtlClientPrivate	ALSACtlClientPrivate;

struct _ALSACtlClient {
	GObject parent_instance;

	ALSACtlClientPrivate *priv;

	int fd;
};

struct _ALSACtlClientClass {
	GObjectClass parent_class;
};

GType alsactl_client_get_type(void) G_GNUC_CONST;

void alsactl_client_open(ALSACtlClient *self, const gchar *path,
			 GError **exception);

void alsactl_client_listen(ALSACtlClient *self, GError **exception);
void alsactl_client_unlisten(ALSACtlClient *self);

void alsactl_client_get_elemset_list(ALSACtlClient *self, GArray *list,
				     GError **exception);

typedef struct _ALSACtlElemset	ALSACtlElemset;
ALSACtlElemset *alsactl_client_get_elemset(ALSACtlClient *self, guint numid,
					   GError **exception);
ALSACtlElemset *alsactl_client_add_elemset_int(ALSACtlClient *self, gint iface,
					       const gchar *name, guint count,
					       guint64 min, guint64 max,
					       guint step, GError **exception);
ALSACtlElemset *alsactl_client_add_elemset_bool(ALSACtlClient *self, gint iface,
						const gchar *name, guint count,
						GError **exception);
ALSACtlElemset *alsactl_client_add_elemset_enum(ALSACtlClient *self, gint iface,
						const gchar *name, guint count,
						GArray *labels,
						GError **exception);
ALSACtlElemset *alsactl_client_add_elemset_iec60958(ALSACtlClient *self,
						    gint iface,
						    const gchar *name,
						    GError **exception);

void alsactl_client_remove_elemset(ALSACtlClient *self, ALSACtlElemset *elem,
				   GError **exception);
#endif
