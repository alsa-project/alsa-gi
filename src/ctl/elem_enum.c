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
#include "elem_enum.h"

G_DEFINE_TYPE(ALSACtlElemEnum, alsactl_elem_enum, ALSACTL_TYPE_ELEM)

static void ctl_elem_enum_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_enum_parent_class)->dispose(obj);
}

static void ctl_elem_enum_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elem_enum_parent_class)->finalize(gobject);
}

static void alsactl_elem_enum_class_init(ALSACtlElemEnumClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elem_enum_dispose;
	gobject_class->finalize = ctl_elem_enum_finalize;
}

static void alsactl_elem_enum_init(ALSACtlElemEnum *self)
{
	return;
}
