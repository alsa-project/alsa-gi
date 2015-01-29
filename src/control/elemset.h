#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEMSET__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEMSET__H__

#include <glib.h>
#include <glib-object.h>

#include "client.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEMSET	(alsactl_elemset_get_type())

#define ALSACTL_ELEMSET(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEMSET,	\
				    ALSACtlElemset))
#define ALSACTL_IS_ELEMSET(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEMSET))

#define ALSACTL_ELEMSET_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEMSET,		\
				 ALSACtlElemsetClass))
#define ALSACTL_IS_ELEMSET_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEMSET))
#define ALSACTL_ELEMSET_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEMSET,	\
				   ALSACtlElemsetClass))

typedef struct _ALSACtlElemset		ALSACtlElemset;
typedef struct _ALSACtlElemsetClass	ALSACtlElemsetClass;
typedef struct _ALSACtlElemsetPrivate	ALSACtlElemsetPrivate;

typedef struct _ALSACtlClient	ALSACtlClient;
struct _ALSACtlElemset {
	GObject parent_instance;

	ALSACtlClient *client;

	ALSACtlElemsetPrivate *priv;
};

struct _ALSACtlElemsetClass {
	GObjectClass parent_class;
};

GType alsactl_elemset_get_type(void) G_GNUC_CONST;

void alsactl_elemset_update(ALSACtlElemset *self, GError **exception);

void alsactl_elemset_lock(ALSACtlElemset *self, GError **exception);
void alsactl_elemset_unlock(ALSACtlElemset *self, GError **exception);

#endif
