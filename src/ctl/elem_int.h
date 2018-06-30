#ifndef __ALSA_GOBJECT_ALSACTL_ELEM_INT__H__
#define __ALSA_GOBJECT_ALSACTL_ELEM_INT__H__

#include <glib.h>
#include <glib-object.h>

#include "elem.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEM_INT	(alsactl_elem_int_get_type())

#define ALSACTL_ELEM_INT(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEM_INT,	\
				    ALSACtlElemInt))
#define ALSACTL_IS_ELEM_INT(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEM_INT))

#define ALSACTL_ELEM_INT_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEM_INT,		\
				 ALSACtlElemIntClass))
#define ALSACTL_IS_ELEM_INT_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEM_INT))
#define ALSACTL_ELEM_INT_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEM_INT,	\
				   ALSACtlElemIntClass))

typedef struct _ALSACtlElemInt		ALSACtlElemInt;
typedef struct _ALSACtlElemIntClass	ALSACtlElemIntClass;
typedef struct _ALSACtlElemIntPrivate	ALSACtlElemIntPrivate;

struct _ALSACtlElemInt {
	ALSACtlElem parent_instance;

	ALSACtlElemPrivate *priv;
};

struct _ALSACtlElemIntClass {
	ALSACtlElemClass parent_class;
};

GType alsactl_elem_int_get_type(void) G_GNUC_CONST;

void alsactl_elem_int_get_max(ALSACtlElemInt *self, unsigned int *max);
void alsactl_elem_int_get_min(ALSACtlElemInt *self, unsigned int *min);
void alsactl_elem_int_get_step(ALSACtlElemInt *self, unsigned int *step);

void alsactl_elem_int_read(ALSACtlElemInt *self, GArray *values,
			    GError **exception);
void alsactl_elem_int_write(ALSACtlElemInt *self, GArray *values,
			     GError **exception);
#endif
