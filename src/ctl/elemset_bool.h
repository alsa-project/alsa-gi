#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_BOOL__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEMSET_BOOL__H__

#include <glib.h>
#include <glib-object.h>

#include "elemset.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEMSET_BOOL	(alsactl_elemset_bool_get_type())

#define ALSACTL_ELEMSET_BOOL(obj)				\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEMSET_BOOL,	\
				    ALSACtlElemsetBool))
#define ALSACTL_IS_ELEMSET_BOOL(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEMSET_BOOL))

#define ALSACTL_ELEMSET_BOOL_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEMSET_BOOL,	\
				 ALSACtlElemsetBoolClass))
#define ALSACTL_IS_ELEMSET_BOOL_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEMSET_BOOL))
#define ALSACTL_ELEMSET_BOOL_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEMSET_BOOL,	\
				   ALSACtlElemsetBoolClass))

typedef struct _ALSACtlElemsetBool	ALSACtlElemsetBool;
typedef struct _ALSACtlElemsetBoolClass	ALSACtlElemsetBoolClass;

struct _ALSACtlElemsetBool {
	ALSACtlElemset parent_instance;
};

struct _ALSACtlElemsetBoolClass {
	ALSACtlElemsetClass parent_class;
};

GType alsactl_elemset_bool_get_type(void) G_GNUC_CONST;

#endif
