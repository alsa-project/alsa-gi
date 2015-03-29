#ifndef __ALSA_TOOLS_GIR_ALSACTL_ELEM_BYTE__H__
#define __ALSA_TOOLS_GIR_ALSACTL_ELEM_BYTE__H__

#include <glib.h>
#include <glib-object.h>

#include "elem.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEM_BYTE	(alsactl_elem_byte_get_type())

#define ALSACTL_ELEM_BYTE(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSACTL_TYPE_ELEM_BYTE,	\
				    ALSACtlElemByte))
#define ALSACTL_IS_ELEM_BYTE(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSACTL_TYPE_ELEM_BYTE))

#define ALSACTL_ELEM_BYTE_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSACTL_TYPE_ELEM_BYTE,	\
				 ALSACtlElemByteClass))
#define ALSACTL_IS_ELEM_BYTE_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSACTL_TYPE_ELEM_BYTE))
#define ALSACTL_ELEM_BYTE_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSACTL_TYPE_ELEM_BYTE,	\
				   ALSACtlElemByteClass))

typedef struct _ALSACtlElemByte	ALSACtlElemByte;
typedef struct _ALSACtlElemByteClass	ALSACtlElemByteClass;

struct _ALSACtlElemByte {
	ALSACtlElem parent_instance;
};

struct _ALSACtlElemByteClass {
	ALSACtlElemClass parent_class;
};

GType alsactl_elem_byte_get_type(void) G_GNUC_CONST;

void alsactl_elem_byte_update(ALSACtlElemByte *self, GError **exception);

void alsactl_elem_byte_read(ALSACtlElemByte *self, GArray *values,
			    GError **exception);
void alsactl_elem_byte_write(ALSACtlElemByte *self, GArray *values,
			     GError **exception);
#endif
