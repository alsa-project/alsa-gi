#ifndef __ALSA_TIMER_QUERY_H__
#define __ALSA_TIMER_QUERY_H__

#include <glib-object.h>
#include <gio/gio.h>

#include "client.h"

G_BEGIN_DECLS

#define ALSA_TIMER_TYPE_QUERY	(alsatimer_query_get_type())

#define ALSA_TIMER_QUERY(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSA_TIMER_TYPE_QUERY,	\
				    ALSATimerQuery))
#define ALSA_TIMER_IS_QUERY(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSA_TIMER_TYPE_QUERY))

#define ALSA_TIMER_QUERY_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSA_TIMER_TYPE_QUERY,		\
				 ALSATimerQueryClass))
#define ALSA_TIMER_IS_QUERY_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSA_TIMER_TYPE_Seq))
#define ALSA_TIMER_QUERY_GET_CLASS(obj) 			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSA_TIMER_TYPE_QUERY,	\
				   ALSATimerQueryClass))

typedef struct _ALSATimerQuery		ALSATimerQuery;
typedef struct _ALSATimerQueryClass	ALSATimerQueryClass;
typedef struct _ALSATimerQueryPrivate	ALSATimerQueryPrivate;

struct _ALSATimerQuery
{
	GObject parent_instance;

	ALSATimerQueryPrivate *priv;
};

struct _ALSATimerQueryClass
{
    GObjectClass parent_class;
};

GType alsatimer_query_get_type(void) G_GNUC_CONST;

ALSATimerQuery *alsatimer_query_new(GError **exception);

ALSATimerQuery *alsatimer_query_adjoin(ALSATimerQuery *self,
				       GError **exception);

ALSATimerClient *alsatimer_query_get_client(ALSATimerQuery *self,
					    GError **exception);

#endif
