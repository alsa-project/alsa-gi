#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_BYTE__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_BYTE__H__

#include <glib.h>
#include <glib-object.h>

#include "elemset.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEMSET_BYTE	(alsactl_elemset_byte_get_type())

#define ALSACTL_ELEMSET_BYTE(obj)				\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEMSET_BYTE,	\
				    ALSACtlElemsetByte))
#define ALSACTL_IS_ELEMSET_BYTE(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEMSET_BYTE))

#define ALSACTL_ELEMSET_BYTE_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEMSET_BYTE,	\
				 ALSACtlElemsetByteClass))
#define ALSACTL_IS_ELEMSET_BYTE_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEMSET_BYTE))
#define ALSACTL_ELEMSET_BYTE_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEMSET_BYTE,	\
				   ALSACtlElemsetByteClass))

typedef struct _ALSACtlElemsetByte	ALSACtlElemsetByte;
typedef struct _ALSACtlElemsetByteClass	ALSACtlElemsetByteClass;

struct _ALSACtlElemsetByte {
	ALSACtlElemset parent_instance;
};

struct _ALSACtlElemsetByteClass {
	ALSACtlElemsetClass parent_class;
};

GType alsactl_elemset_byte_get_type(void) G_GNUC_CONST;

#endif
