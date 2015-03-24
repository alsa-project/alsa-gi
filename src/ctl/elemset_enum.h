#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_ENUM__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_ENUM__H__

#include <glib.h>
#include <glib-object.h>

#include "elemset.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEMSET_ENUM	(alsactl_elemset_enum_get_type())

#define ALSACTL_ELEMSET_ENUM(obj)				\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEMSET_ENUM,	\
				    ALSACtlElemsetEnum))
#define ALSACTL_IS_ELEMSET_ENUM(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEMSET_ENUM))

#define ALSACTL_ELEMSET_ENUM_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEMSET_ENUM,	\
				 ALSACtlElemsetEnumClass))
#define ALSACTL_IS_ELEMSET_ENUM_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEMSET_ENUM))
#define ALSACTL_ELEMSET_ENUM_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEMSET_ENUM,	\
				   ALSACtlElemsetEnumClass))

typedef struct _ALSACtlElemsetEnum		ALSACtlElemsetEnum;
typedef struct _ALSACtlElemsetEnumClass	ALSACtlElemsetEnumClass;

struct _ALSACtlElemsetEnum {
	ALSACtlElemset parent_instance;
};

struct _ALSACtlElemsetEnumClass {
	ALSACtlElemsetClass parent_class;
};

GType alsactl_elemset_enum_get_type(void) G_GNUC_CONST;

#endif
