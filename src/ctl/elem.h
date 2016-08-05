#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEM__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEM__H__

#include <glib.h>
#include <glib-object.h>

#include "client.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEM	(alsactl_elem_get_type())

#define ALSACTL_ELEM(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEM,		\
				    ALSACtlElem))
#define ALSACTL_IS_ELEM(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEM))

#define ALSACTL_ELEM_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEM,		\
				 ALSACtlElemClass))
#define ALSACTL_IS_ELEM_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEM))
#define ALSACTL_ELEM_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEM,		\
				   ALSACtlElemClass))

typedef struct _ALSACtlElem		ALSACtlElem;
typedef struct _ALSACtlElemClass	ALSACtlElemClass;
typedef struct _ALSACtlElemPrivate	ALSACtlElemPrivate;

struct _ALSACtlElem {
	GObject parent_instance;

	ALSACtlElemPrivate *priv;
};

struct _ALSACtlElemClass {
	GObjectClass parent_class;

	void (*update)(ALSACtlElem *self, GError **exception);
};

GType alsactl_elem_get_type(void) G_GNUC_CONST;

void alsactl_elem_update(ALSACtlElem *self, GError **exception);

void alsactl_elem_lock(ALSACtlElem *self, GError **exception);
void alsactl_elem_unlock(ALSACtlElem *self, GError **exception);

void alsactl_elem_value_ioctl(ALSACtlElem *self, int cmd,
			      struct snd_ctl_elem_value *elem_val,
			      GError **exception);
void alsactl_elem_info_ioctl(ALSACtlElem *self, struct snd_ctl_elem_info *info,
			     GError **exception);
#endif
