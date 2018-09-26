#ifndef __ALSA_GOBJECT_ALSACARD_CARD__H
#define __ALSA_GOBJECT_ALSACARD_CARD__H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSACARD_TYPE_UNIT  (alsacard_unit_get_type())

#define ALSACARD_UNIT(obj)                          \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),              \
                                ALSACARD_TYPE_UNIT, \
                                ALSACardUnit))
#define ALSACARD_IS_UNIT(obj)                       \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),              \
                                ALSACARD_TYPE_UNIT))

#define ALSACARD_UNIT_CLASS(klass)                  \
    (G_TYPE_CHECK_CLASS_CAST((klass),               \
                             ALSACARD_TYPE_UNIT,    \
                             ALSACardUnitClass))
#define ALSACARD_IS_UNIT_CLASS(klass)               \
    (G_TYPE_CHECK_CLASS_TYPE((klass),               \
                             ALSACARD_TYPE_UNIT))
#define ALSACARD_UNIT_GET_CLASS(obj)                \
    (G_TYPE_INSTANCE_GET_CLASS((obj),               \
                               ALSACARD_TYPE_UNIT,  \
                               ALSACardUnitClass))

typedef struct _ALSACardUnit        ALSACardUnit;
typedef struct _ALSACardUnitClass   ALSACardUnitClass;

struct _ALSACardUnit
{
    GObject parent_instance;
};

struct _ALSACardUnitClass
{
    GObjectClass parent_class;
};

GType alsacard_unit_get_type(void) G_GNUC_CONST;

void alsacard_unit_open(ALSACardUnit *self, gchar *path, GError **exception);

#endif
