#ifndef __ALSA_TOOLS_GIR_ALSACTL_CLIENT__H_
#define __ALSA_TOOLS_GIR_ALSACTL_CLIENT__H_

#include <glib.h>
#include <glib-object.h>

#include <sound/asound.h>

#include "elem.h"

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

void alsactl_client_get_elem_list(ALSACtlClient *self, GArray *list,
				     GError **exception);

typedef struct _ALSACtlElem	ALSACtlElem;
ALSACtlElem *alsactl_client_get_elem(ALSACtlClient *self, guint numid,
				     GError **exception);
void alsactl_client_add_int_elems(ALSACtlClient *self, gint iface,
				  guint number, const gchar *name,
				  guint channels, guint64 min, guint64 max,
				  guint step, const GArray *dimen,
				  GArray *elems, GError **exception);
void alsactl_client_add_bool_elems(ALSACtlClient *self, gint iface,
				   guint number, const gchar *name,
				   guint channels, const GArray *dimen,
				   GArray *elems, GError **exception);
void alsactl_client_add_enum_elems(ALSACtlClient *self, gint iface,
				   guint number,  const gchar *name,
				   guint channels, GArray *items,
				   const GArray *dimen,
				   GArray *elems, GError **exception);
void alsactl_client_add_byte_elems(ALSACtlClient *self, gint iface,
				   guint number, const gchar *name,
				   guint channels, const GArray *dimen,
				   GArray *elems, GError **exception);
void alsactl_client_add_iec60958_elems(ALSACtlClient *self, gint iface,
				       guint number, const gchar *name,
				       GArray *elems, GError **exception);
void alsactl_client_remove_elem(ALSACtlClient *self, ALSACtlElem *elem);
#endif
