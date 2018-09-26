#ifndef __ALSA_GOBJECT_ALSACTL_ELEM_BOOL__H__
#define __ALSA_GOBJECT_ALSACTL_ELEM_BOOL__H__

#include <glib.h>
#include <glib-object.h>

#include "elem.h"

G_BEGIN_DECLS

#define ALSACTL_TYPE_ELEM_BOOL  (alsactl_elem_bool_get_type())

#define ALSACTL_ELEM_BOOL(obj)                          \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),                  \
                                ALSACTL_TYPE_ELEM_BOOL, \
                                ALSACtlElemBool))
#define ALSACTL_IS_ELEM_BOOL(obj)                       \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),                  \
                                ALSACTL_TYPE_ELEM_BOOL))

#define ALSACTL_ELEM_BOOL_CLASS(klass)                  \
    (G_TYPE_CHECK_CLASS_CAST((klass),                   \
                             ALSACTL_TYPE_ELEM_BOOL,    \
                             ALSACtlElemBoolClass))
#define ALSACTL_IS_ELEM_BOOL_CLASS(klass)               \
    (G_TYPE_CHECK_CLASS_TYPE((klass),                   \
                             ALSACTL_TYPE_ELEM_BOOL))
#define ALSACTL_ELEM_BOOL_GET_CLASS(obj)                \
    (G_TYPE_INSTANCE_GET_CLASS((obj),                   \
                               ALSACTL_TYPE_ELEM_BOOL,  \
                               ALSACtlElemBoolClass))

typedef struct _ALSACtlElemBool         ALSACtlElemBool;
typedef struct _ALSACtlElemBoolClass    ALSACtlElemBoolClass;

struct _ALSACtlElemBool {
    ALSACtlElem parent_instance;
};

struct _ALSACtlElemBoolClass {
    ALSACtlElemClass parent_class;
};

GType alsactl_elem_bool_get_type(void) G_GNUC_CONST;

void alsactl_elem_bool_update(ALSACtlElemBool *self, GError **exception);

void alsactl_elem_bool_read(ALSACtlElemBool *self, GArray *values,
                            GError **exception);
void alsactl_elem_bool_write(ALSACtlElemBool *self, GArray *values,
                             GError **exception);

#endif
