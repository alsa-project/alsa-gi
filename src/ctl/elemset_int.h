#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_INT__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_INT__H__

#include <glib.h>
#include <glib-object.h>

#include "elemset.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEMSET_INT	(alsactl_elemset_int_get_type())

#define ALSACTL_ELEMSET_INT(obj)				\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEMSET_INT,	\
				    ALSACtlElemsetInt))
#define ALSACTL_IS_ELEMSET_INT(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEMSET_INT))

#define ALSACTL_ELEMSET_INT_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEMSET_INT,	\
				 ALSACtlElemsetIntClass))
#define ALSACTL_IS_ELEMSET_INT_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEMSET_INT))
#define ALSACTL_ELEMSET_INT_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEMSET_INT,	\
				   ALSACtlElemsetIntClass))

typedef struct _ALSACtlElemsetInt		ALSACtlElemsetInt;
typedef struct _ALSACtlElemsetIntClass	ALSACtlElemsetIntClass;

struct _ALSACtlElemsetInt {
	ALSACtlElemset parent_instance;
};

struct _ALSACtlElemsetIntClass {
	ALSACtlElemsetClass parent_class;
};

GType alsactl_elemset_int_get_type(void) G_GNUC_CONST;

#endif
