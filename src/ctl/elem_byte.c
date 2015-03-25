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
#include "elem_byte.h"

G_DEFINE_TYPE(ALSACtlElemByte, alsactl_elem_byte, ALSACTL_TYPE_ELEM)

static void ctl_elem_byte_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_byte_parent_class)->dispose(obj);
}

static void ctl_elem_byte_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elem_byte_parent_class)->finalize(gobject);
}

static void alsactl_elem_byte_class_init(ALSACtlElemByteClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elem_byte_dispose;
	gobject_class->finalize = ctl_elem_byte_finalize;
}

static void alsactl_elem_byte_init(ALSACtlElemByte *self)
{
	return;
}
