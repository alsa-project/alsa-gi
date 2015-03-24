#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_IEC60958__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_IEC60958__H__

#include <glib.h>
#include <glib-object.h>

#include "elemset.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEMSET_IEC60958	(alsactl_elemset_iec60958_get_type())

#define ALSACTL_ELEMSET_IEC60958(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),				\
				    ALSACTL_TYPE_ELEMSET_IEC60958,	\
				    ALSACtlElemsetIec60958))
#define ALSACTL_IS_ELEMSET_IEC60958(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),				\
				    ALSACTL_TYPE_ELEMSET_IEC60958))

#define ALSACTL_ELEMSET_IEC60958_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),				\
				 ALSACTL_TYPE_ELEMSET_IEC60958,		\
				 ALSACtlElemsetIec60958Class))
#define ALSACTL_IS_ELEMSET_IEC60958_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),				\
				 ALSACTL_TYPE_ELEMSET_IEC60958))
#define ALSACTL_ELEMSET_IEC60958_GET_CLASS(obj)				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),				\
				   ALSACTL_TYPE_ELEMSET_IEC60958,	\
				   ALSACtlElemsetIec60958Class))

typedef struct _ALSACtlElemsetIec60958		ALSACtlElemsetIec60958;
typedef struct _ALSACtlElemsetIec60958Class	ALSACtlElemsetIec60958Class;

struct _ALSACtlElemsetIec60958 {
	ALSACtlElemset parent_instance;
};

struct _ALSACtlElemsetIec60958Class {
	ALSACtlElemsetClass parent_class;
};

GType alsactl_elemset_iec60958_get_type(void) G_GNUC_CONST;

#endif
