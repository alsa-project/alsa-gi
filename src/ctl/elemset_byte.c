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
#include "elemset_byte.h"

G_DEFINE_TYPE(ALSACtlElemsetByte, alsactl_elemset_byte, ALSACTL_TYPE_ELEMSET)

static void ctl_elemset_byte_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elemset_byte_parent_class)->dispose(obj);
}

static void ctl_elemset_byte_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elemset_byte_parent_class)->finalize(gobject);
}

static void alsactl_elemset_byte_class_init(ALSACtlElemsetByteClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elemset_byte_dispose;
	gobject_class->finalize = ctl_elemset_byte_finalize;
}

static void alsactl_elemset_byte_init(ALSACtlElemsetByte *self)
{
	return;
}
