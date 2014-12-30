#ifndef __ALSATIMER_CLIENT_H__
#define __ALSATIMER_CLIENT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define ALSATIMER_TYPE_CLIENT	(alsatimer_client_get_type())

#define ALSATIMER_CLIENT(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSATIMER_TYPE_CLIENT,	\
				    ALSATimerClient))
#define ALSATIMER_IS_CLIENT(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSATIMER_TYPE_CLIENT))

#define ALSATIMER_CLIENT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSATIMER_TYPE_CLIENT,		\
				 ALSATimerClientClass))
#define ALSATIMER_IS_CLIENT_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSATIMER_TYPE_Seq))
#define ALSATIMER_CLIENT_GET_CLASS(obj) 			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSATIMER_TYPE_CLIENT,	\
				   ALSATimerClientClass))

typedef struct _ALSATimerClient		ALSATimerClient;
typedef struct _ALSATimerClientClass	ALSATimerClientClass;
typedef struct _ALSATimerClientPrivate	ALSATimerClientPrivate;

struct _ALSATimerClient
{
	GObject parent_instance;

	ALSATimerClientPrivate *priv;
};

struct _ALSATimerClientClass
{
    GObjectClass parent_class;
};

GType alsatimer_client_get_type(void) G_GNUC_CONST;

ALSATimerClient *alsatimer_client_new(gchar *str, GError **exception);

void alsatimer_client_get_status(ALSATimerClient *self, GArray *status,
				 GError **exception);

void alsatimer_client_start(ALSATimerClient *client, GError **exception);
void alsatimer_client_stop(ALSATimerClient *client, GError **exception);
void alsatimer_client_resume(ALSATimerClient *client, GError **exception);
#endif
