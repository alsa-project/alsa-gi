#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "card.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* For error handling. */
G_DEFINE_QUARK("ALSACardUnit", alsacard_unit)
#define raise(exception, errno)                             \
    g_set_error(exception, alsacard_unit_quark(), errno,    \
            "%d: %s", __LINE__, strerror(errno))

G_DEFINE_TYPE(ALSACardUnit, alsacard_unit, G_TYPE_OBJECT)

static void alsacard_unit_class_init(ALSACardUnitClass *klass)
{
    return;
}

static void
alsacard_unit_init(ALSACardUnit *self)
{
    return;
}

void alsacard_unit_open(ALSACardUnit *self, gchar *path, GError **exception)
{
    return;
}
