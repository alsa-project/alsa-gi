#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEM_ENUM__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEM_ENUM__H__

#include <glib.h>
#include <glib-object.h>

#include "elem.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEM_ENUM	(alsactl_elem_enum_get_type())

#define ALSACTL_ELEM_ENUM(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEM_ENUM,	\
				    ALSACtlElemEnum))
#define ALSACTL_IS_ELEM_ENUM(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEM_ENUM))

#define ALSACTL_ELEM_ENUM_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEM_ENUM,	\
				 ALSACtlElemEnumClass))
#define ALSACTL_IS_ELEM_ENUM_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEM_ENUM))
#define ALSACTL_ELEM_ENUM_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEM_ENUM,	\
				   ALSACtlElemEnumClass))

typedef struct _ALSACtlElemEnum		ALSACtlElemEnum;
typedef struct _ALSACtlElemEnumClass	ALSACtlElemEnumClass;

struct _ALSACtlElemEnum {
	ALSACtlElem parent_instance;
};

struct _ALSACtlElemEnumClass {
	ALSACtlElemClass parent_class;
};

GType alsactl_elem_enum_get_type(void) G_GNUC_CONST;

#endif
