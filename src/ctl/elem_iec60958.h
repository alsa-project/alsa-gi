#ifndef __ALSA_GOBJECT_ALSACTL_ELEM_IEC60958__H__
#define __ALSA_GOBJECT_ALSACTL_ELEM_IEC60958__H__

#include <glib.h>
#include <glib-object.h>

#include "elem.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEM_IEC60958	(alsactl_elem_iec60958_get_type())

#define ALSACTL_ELEM_IEC60958(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),				\
				    ALSACTL_TYPE_ELEM_IEC60958,		\
				    ALSACtlElemIec60958))
#define ALSACTL_IS_ELEM_IEC60958(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),				\
				    ALSACTL_TYPE_ELEM_IEC60958))

#define ALSACTL_ELEM_IEC60958_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),				\
				 ALSACTL_TYPE_ELEM_IEC60958,		\
				 ALSACtlElemIec60958Class))
#define ALSACTL_IS_ELEM_IEC60958_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),				\
				 ALSACTL_TYPE_ELEM_IEC60958))
#define ALSACTL_ELEM_IEC60958_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),				\
				   ALSACTL_TYPE_ELEM_IEC60958,		\
				   ALSACtlElemIec60958Class))

typedef struct _ALSACtlElemIec60958		ALSACtlElemIec60958;
typedef struct _ALSACtlElemIec60958Class	ALSACtlElemIec60958Class;

struct _ALSACtlElemIec60958 {
	ALSACtlElem parent_instance;
};

struct _ALSACtlElemIec60958Class {
	ALSACtlElemClass parent_class;
};

GType alsactl_elem_iec60958_get_type(void) G_GNUC_CONST;

#endif
