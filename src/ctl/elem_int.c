#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stddef.h>

#include <sound/asound.h>
#include "elem_int.h"

G_DEFINE_TYPE(ALSACtlElemInt, alsactl_elem_int, ALSACTL_TYPE_ELEM)

static void ctl_elem_int_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_int_parent_class)->dispose(obj);
}

static void ctl_elem_int_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elem_int_parent_class)->finalize(gobject);
}

static void alsactl_elem_int_class_init(ALSACtlElemIntClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elem_int_dispose;
	gobject_class->finalize = ctl_elem_int_finalize;
}

static void alsactl_elem_int_init(ALSACtlElemInt *self)
{
	return;
}
